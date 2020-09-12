#include "3dPatches.h"
#include "RendererHook.h"
#include "Patch.h"
#include "Utils.h"
#include "FifamTypes.h"

void *METHOD OnCreateFileIO(void *fileIO) {
    CallMethodDynGlobal(RendererAddress(0x108230), fileIO);
    std::wstring zdataPath = FM::GetGameDir() + L"data\\assets\\";
    wchar_t const *paths[] = { zdataPath.c_str() };
    CallMethodDynGlobal(RendererAddress(0x108540), fileIO, paths, 1);
    return fileIO;
}

bool METHOD OnSetFifaTexture(void *t, DUMMY_ARG, int samplerIndex) {
    void *device = *(void **)RendererAddress(0x39AAA4);
    __asm {
        push    1
        push    1
        push    samplerIndex
        mov     eax, device
        mov     eax, [eax]
        push    device
        call    dword ptr [eax+0x114]
    }
    __asm {
        push    1
        push    2
        push    samplerIndex
        mov     eax, device
        mov     eax, [eax]
        push    device
        call    dword ptr[eax + 0x114]
    }
    return CallMethodAndReturnDynGlobal<int>(RendererAddress(0xAB74D), t, samplerIndex);
}

unsigned int gCurrentPlayerLoadPlayerId = 0;

void METHOD OnLoadFifaPlayer(void *player, DUMMY_ARG, void *playerInfo, void *resource, bool loadBody, void *resMan) {
    gCurrentPlayerLoadPlayerId = *raw_ptr<unsigned int>(playerInfo, 4);
    //void *textures = raw_ptr<void>(player, 0x20);
    //CallMethodDynGlobal(RendererAddress(0xAC0C3), textures); // clear loaded textures
    //*raw_ptr<unsigned int>(textures, 0x0) = 0;
    //*raw_ptr<unsigned int>(textures, 0x8) = 0;
    //*raw_ptr<unsigned int>(textures, 0xC) = 0;
    //*raw_ptr<unsigned int>(player, 0x70) = 0;
    //*raw_ptr<unsigned int>(player, 0x74) = 0;
    //CallMethodDynGlobal(RendererAddress(0xA09DC), textures, 16); // init loaded textures
    CallMethodDynGlobal(RendererAddress(0x9E6CA), player, playerInfo, resource, loadBody, resMan);
    gCurrentPlayerLoadPlayerId = 0;
}

int METHOD OnGetPlayerEffTexture(void *t, DUMMY_ARG, char const *name, bool cached, bool cube) {
    if (gCurrentPlayerLoadPlayerId != 0) {
        auto customEffTexPath = "data\\assets\\eff_" + Utils::Format("%d", gCurrentPlayerLoadPlayerId) + ".dds";
        if (exists(customEffTexPath))
            return CallMethodAndReturnDynGlobal<int>(RendererAddress(0xAB9FE), t, customEffTexPath.c_str(), false, cube);
    }
    return CallMethodAndReturnDynGlobal<int>(RendererAddress(0xAB9FE), t, name, cached, cube);
}

void *METHOD Test1(void *t, DUMMY_ARG, int a) {
    Error("Test");
    return CallMethodAndReturnDynGlobal<void *>(RendererAddress(0x83D23), t, a);
}

void Install3dPatches() {
    //Error("%X", RendererAddress(0));
    patch::RedirectCall(RendererAddress(0x92579), OnCreateFileIO);

    patch::SetChar(RendererAddress(0xAB61B + 3), 't');
    patch::SetChar(RendererAddress(0xAB625 + 3), 'g');
    patch::SetChar(RendererAddress(0xAB62F + 3), 'a');

    patch::SetPointer(RendererAddress(0xAB5EF + 1), ".dds");
    patch::SetPointer(RendererAddress(0x91E47 + 1), "hair_cm_-%d_%d.dds");
    patch::SetPointer(RendererAddress(0x91E29 + 1), "cm_%d.dds");
    patch::SetPointer(RendererAddress(0x91EDA + 1), "hair_cmb_-%d.dds");
    patch::SetPointer(RendererAddress(0x91EC2 + 1), "cmbhair_%d.dds");

    //patch::Nop(RendererAddress(0x809B8), 11);

    patch::RedirectCall(RendererAddress(0xAB9E1), OnSetFifaTexture);
    patch::RedirectCall(RendererAddress(0xAB9F5), OnSetFifaTexture);

    patch::RedirectCall(RendererAddress(0x842DE), OnLoadFifaPlayer);
    patch::RedirectCall(RendererAddress(0x847B8), OnLoadFifaPlayer);
    patch::RedirectCall(RendererAddress(0x85335), OnLoadFifaPlayer);
    patch::RedirectCall(RendererAddress(0xAE389), OnLoadFifaPlayer);

    patch::RedirectCall(RendererAddress(0x9F6C2), OnGetPlayerEffTexture);

    patch::SetUChar(RendererAddress(0x5EE2), 0xEB);

    patch::SetDouble(RendererAddress(0x2CC110), 91.0);

    //const unsigned int kitW = 1024;
    //const unsigned int kitH = 2048;
    //
    //patch::SetUInt(RendererAddress(0x8F34B + 6), kitW);
    //patch::SetUInt(RendererAddress(0x8F355 + 6), kitH);
    //patch::SetUInt(RendererAddress(0x8F337 + 1), kitW * 2);
    //patch::SetUInt(RendererAddress(0x8F40C + 6), kitW);
    //patch::SetUInt(RendererAddress(0x8F418 + 6), kitH);
    //patch::SetUInt(RendererAddress(0x8F3E9 + 1), kitW * kitH * 4 + 0x100);
    //patch::SetUInt(RendererAddress(0x8F48D + 1), kitW);
    //patch::SetUInt(RendererAddress(0x8F488 + 1), kitH);
    //patch::SetUInt(RendererAddress(0x8F35F + 1), kitW);
    //patch::SetUChar(RendererAddress(0xAB39C), 0xC3);

    //patch::SetUChar(RendererAddress(0x901AF + 1), 3);
    //patch::SetUChar(RendererAddress(0x901C9 + 1), 3);
    //patch::SetUChar(RendererAddress(0x901E2 + 1), 3);

    //patch::RedirectCall(RendererAddress(0x7BCDC), Test1);
}
