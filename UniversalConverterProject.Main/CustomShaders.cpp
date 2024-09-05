#include "CustomShaders.h"
#include "GfxCoreHook.h"
#include "UcpSettings.h"
#include "Utils.h"
#include "FifamTypes.h"
#include "d3dx9.h"
#include <fstream>

using namespace std;

struct Shader {
    const char *mpName;
    D3DVERTEXELEMENT9 *mpVertexElements;
    void *mpVertexShaderData;
    void *mpPixelShaderData;
    void *mpVertexShaderFunction[5];
    void *mpPixelShaderFunction[5];
    unsigned int nVertexShaderRefCount;
    unsigned int nPixelShaderRefCount;
    unsigned int nVertexShaderRefCountSaved;
    unsigned int nPixelShaderRefCountSaved;
    IDirect3DVertexShader9 *mpD3DVertexShader[5];
    IDirect3DVertexDeclaration9 *mpVertexDecl;
    IDirect3DPixelShader9 *mpD3DPixelShader[5];
    unsigned int numTechniques;
    char const **mpTechniqueNames;
    unsigned int mnTechniqueIndex;
    unsigned char bLoadTechnique[5];
    char _pad85[3];
    Shader *mpPrev;
    int field_8C;
};

void writeshadercode(std::ofstream &f, std::string const &codeStr, bool pixel, bool effectLookALike) {
    std::string line;
    std::string comment;
    std::vector<std::string> lines;
    for (auto c : codeStr) {
        if (c == '\n') {
            if (!line.empty()) {
                if (Utils::StartsWith(line, "// approximately"))
                    comment = line;
                else {
                    if (Utils::StartsWith(line, "#line"))
                        line = "// " + line.substr(1);
                    lines.push_back(line);
                }
            }
            line.clear();
        }
        else
            line.push_back(c);
    }
    if (!line.empty())
        lines.push_back(line);
    if (effectLookALike) {
        f << (pixel ? "    PixelShader =" : "    VertexShader =") << std::endl;
        f << "    asm";
    }
    else
        f << (pixel ? "#PixelShader asm" : "#VertexShader asm");
    if (!comment.empty())
        f << " " << comment;
    f << std::endl;
    if (effectLookALike)
        f << "    {" << std::endl;
    for (auto const &l : lines) {
        if (effectLookALike)
            f << "    " << l << std::endl;
        else
            f << l << std::endl;
    }
    if (effectLookALike)
        f << "    };" << std::endl;
    else
        f << "#End" << std::endl;
}

void DumpShader(Shader *s, bool effectLookALike) {
    CreateDirectoryW(FM::GameDirPath(L"shaders_original").c_str(), NULL);
    std::ofstream f(std::string("shaders_original\\") + *(char const **)s + (effectLookALike ? ".fx" : ".sh"));
    for (unsigned int i = 0; i < s->numTechniques; i++) {
        if (effectLookALike)
            f << "technique \"" << s->mpTechniqueNames[i] << "\" {" << std::endl;
        else
            f << "#Technique \"" << s->mpTechniqueNames[i] << "\"" << std::endl;
        for (unsigned int c = 0; c < 2; c++) {
            void *code = c == 0 ? s->mpVertexShaderFunction[i] : s->mpPixelShaderFunction[i];
            if (code) {
                ID3DXBuffer *buf = nullptr;
                D3DXDisassembleShader((const DWORD *)code, FALSE, NULL, &buf);
                if (buf->GetBufferSize() > 1)
                    writeshadercode(f, (char const *)buf->GetBufferPointer(), c == 1, effectLookALike);
                buf->Release();
            }
        }
        if (effectLookALike)
            f << "}" << std::endl;
        else
            f << "#End" << std::endl;
    }
}

void DumpShaders() {
    for (Shader *s = *(Shader **)GfxCoreAddress(0xBEF6C4); s; s = s->mpPrev)
        DumpShader(s, false);
}

vector<unsigned char *> &GetShadersDataStorage() {
    static vector<unsigned char *> data;
    return data;
}

void ClearShadersDataStorage() {
    for (auto i : GetShadersDataStorage())
        delete[] i;
    GetShadersDataStorage().clear();
}

void ShaderLoadCustom(Shader *shader) {
    wstring shaderFilePath = FM::GameDirPath(L"shaders\\" + AtoW(shader->mpName) + L".sh");
    if (plugin::FileExistsW(shaderFilePath)) {
        auto lines = FileToLinesA(shaderFilePath);
        string techniqueName;
        string shaderEntryPoint;
        string shaderModel;
        bool asmShader = false;
        string shaderCode;
        enum class Mode { None, Technique, VertexShader, PixelShader } mode = Mode::None;
        auto ClearSection = [&] {
            mode = Mode::None;
            shaderCode.clear();
            shaderEntryPoint.clear();
            shaderModel.clear();
            asmShader = false;
        };
        for (size_t lineId = 0; lineId < lines.size(); lineId++) {
            auto line = lines[lineId];
            auto lineLow = ToLower(line);
            if (Utils::StartsWith(lineLow, "#technique")) {
                ClearSection();
                techniqueName.clear();
                if (line.size() > 11) {
                    auto parts = Split(line.substr(11), ' ', true, true, true);
                    if (parts.size() > 0)
                        techniqueName = parts[0];
                }
                mode = Mode::Technique;
            }
            else if (Utils::StartsWith(lineLow, "#vertexshader") || Utils::StartsWith(lineLow, "#pixelshader")) {
                ClearSection();
                unsigned int titleSize = 0;
                if (Utils::StartsWith(lineLow, "#vertexshader")) {
                    titleSize = 14;
                    mode = Mode::VertexShader;
                }
                else {
                    titleSize = 13;
                    mode = Mode::PixelShader;
                }
                if (line.size() > titleSize) {
                    auto parts = Split(line.substr(titleSize), ' ', true, true, true);
                    for (size_t i = 0; i < parts.size(); i++) {
                        if (parts[i] == "asm")
                            asmShader = true;
                        else if (parts[i] == "vs_1_1" || parts[i] == "vs_2_0" || parts[i] == "vs_2_x" || parts[i] == "vs_3_0" ||
                            parts[i] == "vs_2_sw" || parts[i] == "vs_3_sw" || parts[i] == "ps_1_1" || parts[i] == "ps_1_2" ||
                            parts[i] == "ps_1_3" || parts[i] == "ps_1_4" || parts[i] == "ps_2_x" || parts[i] == "ps_3_0" ||
                            parts[i] == "ps_2_sw" || parts[i] == "ps_3_sw")
                        {
                            shaderModel = parts[i];
                        }
                        else if (shaderEntryPoint.empty())
                            shaderEntryPoint = parts[i];
                    }
                }
            }
            else if (Utils::StartsWith(lineLow, "#end")) {
                if (mode == Mode::VertexShader || mode == Mode::PixelShader) {
                    // find technique
                    int techniqueIndex = -1;
                    for (size_t i = 0; i < shader->numTechniques; i++) {
                        if (!_stricmp(shader->mpTechniqueNames[i], techniqueName.c_str())) {
                            ID3DXBuffer *buf = nullptr;
                            ID3DXBuffer *err = nullptr;
                            HRESULT result;
                            if (asmShader) {
                                //::Warning(string("assemble ") + (mode == Mode::VertexShader ? "VS " : "PS ") + "\nin technique " + techniqueName + "\nin file " + shader->mpName);
                                result = D3DXAssembleShader(shaderCode.c_str(), shaderCode.size(), NULL, NULL, 0, &buf, &err);
                            }
                            else {
                                string funcName = shaderEntryPoint;
                                if (funcName.empty())
                                    funcName = "main";
                                string profile = shaderModel;
                                if (profile.empty())
                                    profile = (mode == Mode::VertexShader) ? "vs_2_0" : "ps_2_0";
                                //::Warning(string("compile ") + (mode == Mode::VertexShader ? "VS " : "PS ") + profile + " " + funcName + "\nin technique " + techniqueName + "\nin file " + shader->mpName);
                                result = D3DXCompileShader(shaderCode.c_str(), shaderCode.size(), NULL, NULL, funcName.c_str(), profile.c_str(), 0, &buf, &err, NULL);
                            }
                            if (FAILED(result)) {
                                if (!err) {
                                    ::Error(string("Failed to compile ") + ((mode == Mode::VertexShader) ? "vertex" : "pixel") + " shader\n"
                                        + "in technique " + techniqueName + "\nin file " + shader->mpName);
                                }
                                else {
                                    ::Error(string("Failed to compile ") + ((mode == Mode::VertexShader) ? "vertex" : "pixel") + " shader:\n"
                                        + (char const *)err->GetBufferPointer() + "in technique " + techniqueName + "\nin file " + shader->mpName);
                                }
                            }
                            else {
                                unsigned char *data = new unsigned char[buf->GetBufferSize()];
                                memcpy(data, buf->GetBufferPointer(), buf->GetBufferSize());
                                if (mode == Mode::VertexShader)
                                    shader->mpVertexShaderFunction[i] = data;
                                else
                                    shader->mpPixelShaderFunction[i] = data;
                                GetShadersDataStorage().push_back(data);
                                if (err && Settings::GetInstance().ShadersShowWarnings) {
                                    ::Error(string("There were warnings during ") + ((mode == Mode::VertexShader) ? "vertex" : "pixel") + " shader " + (asmShader ? "assembling" : "compilation") + ":\n"
                                        + (char const *)err->GetBufferPointer() + "in technique " + techniqueName + "\nin file " + shader->mpName);
                                }
                            }
                            if (err)
                                err->Release();
                            if (buf)
                                buf->Release();
                            break;
                        }
                    }
                }
                ClearSection();
            }
            else if (mode == Mode::VertexShader || mode == Mode::PixelShader)
                shaderCode += line + "\n";
        }
    }
}

void ReloadShaders() {
    //::Warning("Reloading shaders");
    for (Shader *s = *(Shader **)GfxCoreAddress(0xBEF6C4); s; s = s->mpPrev) {
        CallMethodDynGlobal(GfxCoreAddress(0x2D73E0), s, 1); // Shader::ReleaseVertex
        CallMethodDynGlobal(GfxCoreAddress(0x2D7460), s, 1); // Shader::ReleasePixel
    }
    // reset all shaders code
    for (Shader *s = *(Shader **)GfxCoreAddress(0xBEF6C4); s; s = s->mpPrev) {
        unsigned int *shaderData = (unsigned int *)s->mpVertexShaderData;
        for (unsigned int i = 0; i < s->numTechniques; i++) {
            s->mpVertexShaderFunction[i] = shaderData;
            if (i == (s->numTechniques - 1))
                break;
            while (*shaderData != 0xFFFF)
                shaderData++;
            shaderData++;
        }
        shaderData = (unsigned int *)s->mpPixelShaderData;
        for (unsigned int i = 0; i < s->numTechniques; i++) {
            s->mpPixelShaderFunction[i] = shaderData;
            if (i == (s->numTechniques - 1))
                break;
            while (*shaderData != 0xFFFF)
                shaderData++;
            shaderData++;
        }
    }
    ClearShadersDataStorage();
    for (Shader *s = *(Shader **)GfxCoreAddress(0xBEF6C4); s; s = s->mpPrev)
        ShaderLoadCustom(s);
    CallDynGlobal(GfxCoreAddress(0x2E58D0)); // ReCreateVertexShaders
    CallDynGlobal(GfxCoreAddress(0x2E58F0)); // ReCreatePixelShaders
}

void OnFocusGained() {
    ReloadShaders();
    CallDynGlobal(GfxCoreAddress(0x16630)); // FocusGained
}

int gLastTechniqueIndex = -1;
int gCurrentTechniqueIndex = -1;

void *METHOD OnConstructETTObject(void *t, DUMMY_ARG, void *data) {
    gLastTechniqueIndex = -1;
    gCurrentTechniqueIndex = -1;
    void *result = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x2ECF70), t, data);
    gLastTechniqueIndex = -1;
    gCurrentTechniqueIndex = -1;
    return result;
}

void *METHOD OnConstructETTTechnique(void *t, DUMMY_ARG, void *data, void *vertexDeclaration) {
    gCurrentTechniqueIndex = *raw_ptr<int>(*(void **)data, 4);
    void *result = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x2EC0B0), t, data, vertexDeclaration);
    gLastTechniqueIndex = gCurrentTechniqueIndex;
    return result;
}

void *METHOD OnConstructEmptyPixelShader(void *t, DUMMY_ARG, void *data) {
    static const unsigned char DummyPixelShader[] = {
        0x01, 0x01, 0xFF, 0xFF, 0xFE, 0xFF, 0x20, 0x00, 0x44, 0x42, 0x55, 0x47, 0x28, 0x00, 0x00, 0x00,
        0x46, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xFF, 0xFF, 0x88, 0x00, 0x00, 0x00, 0x0A, 0x09, 0x70, 0x73,
        0x2E, 0x31, 0x2E, 0x31, 0x0A, 0x09, 0x6D, 0x6F, 0x76, 0x20, 0x72, 0x30, 0x2C, 0x20, 0x76, 0x30,
        0x0A, 0x00, 0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66, 0x74, 0x20, 0x28, 0x52, 0x29, 0x20,
        0x44, 0x33, 0x44, 0x58, 0x39, 0x20, 0x53, 0x68, 0x61, 0x64, 0x65, 0x72, 0x20, 0x41, 0x73, 0x73,
        0x65, 0x6D, 0x62, 0x6C, 0x65, 0x72, 0x20, 0x39, 0x2E, 0x31, 0x32, 0x2E, 0x35, 0x38, 0x39, 0x2E,
        0x30, 0x30, 0x30, 0x30, 0x00, 0xAB, 0xAB, 0xAB, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80,
        0x00, 0x00, 0xE4, 0x90, 0xFF, 0xFF, 0x00, 0x00
    };
    if (gLastTechniqueIndex != gCurrentTechniqueIndex) {
        Shader *s = CallAndReturnDynGlobal<Shader *>(GfxCoreAddress(0x2D75D0), GfxCoreAddress(0xBF0778)); // FindShader(gCurrentShaderName)
        if (s) {
            for (unsigned int i = 0; i < s->numTechniques; i++) {
                if (s->mpPixelShaderFunction[i]) {
                    for (unsigned char *psData : GetShadersDataStorage()) {
                        if (psData == s->mpPixelShaderFunction[i]) {
                            //::Warning("Replaced empty pixel shader in technique %d in effect %s", gCurrentTechniqueIndex, (char *)0xD43278);
                            return CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x2E57A0), t, DummyPixelShader);
                        }
                    }
                }
            }
        }
    }
    return CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x2E57A0), t, data);
}

void InstallCustomShaders() {
    if (Settings::GetInstance().DumpShaders)
        DumpShaders();
    bool useCustomShaders = false;
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(FM::GameDirPath(L"shaders\\*.sh").c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        useCustomShaders = true;
        FindClose(hFind);
    }
    if (useCustomShaders) {
        for (Shader *s = *(Shader **)GfxCoreAddress(0xBEF6C4); s; s = s->mpPrev)
            ShaderLoadCustom(s);
        patch::RedirectCall(GfxCoreAddress(0x2EBF7C), OnConstructEmptyPixelShader);
        patch::RedirectCall(GfxCoreAddress(0x2E4BEF), OnConstructETTObject);
        patch::RedirectCall(GfxCoreAddress(0x2E4C42), OnConstructETTObject);
        patch::RedirectCall(GfxCoreAddress(0x2ED0F3), OnConstructETTTechnique);
        if (Settings::GetInstance().ShadersReload)
            patch::RedirectCall(GfxCoreAddress(0x16C0C), OnFocusGained);
    }
}
