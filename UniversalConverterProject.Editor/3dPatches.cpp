#include "3dPatches.h"
#include "RendererHook.h"
#include "Patch.h"
#include "Utils.h"
#include "FifamTypes.h"
#include "Kits.h"
#include "AssetLoader.h"

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

UInt gCurrentPlayerLoadPlayerId = 0;
Int gCurrentPlayerLoadFaceType = -1;

void METHOD OnLoadFifaPlayer(void *player, DUMMY_ARG, void *playerInfo, void *resource, bool loadBody, void *resMan) {
    gCurrentPlayerLoadPlayerId = *raw_ptr<UInt>(playerInfo, 4);
    gCurrentPlayerLoadFaceType = *raw_ptr<Int>(playerInfo, 0x20);
    //void *textures = raw_ptr<void>(player, 0x20);
    //CallMethodDynGlobal(RendererAddress(0xAC0C3), textures); // clear loaded textures
    //*raw_ptr<unsigned int>(textures, 0x0) = 0;
    //*raw_ptr<unsigned int>(textures, 0x8) = 0;
    //*raw_ptr<unsigned int>(textures, 0xC) = 0;
    //*raw_ptr<unsigned int>(player, 0x70) = 0;
    //*raw_ptr<unsigned int>(player, 0x74) = 0;
    //CallMethodDynGlobal(RendererAddress(0xA09DC), textures, 16); // init loaded textures
    CallMethodDynGlobal(RendererAddress(0x9E6CA), player, playerInfo, resource, loadBody, resMan);
    UpdateKitCollar(player);
    gCurrentPlayerLoadPlayerId = 0;
    gCurrentPlayerLoadFaceType = -1;
}

int METHOD OnGetPlayerEffTexture(void *t, DUMMY_ARG, char const *name, bool cached, bool cube) {
    if (gCurrentPlayerLoadPlayerId != 0) {
        auto customEffTexPath = AssetFileName("eff_" + Utils::Format("%d", gCurrentPlayerLoadPlayerId) + ".dds");
        if (!customEffTexPath.empty())
            return CallMethodAndReturnDynGlobal<int>(RendererAddress(0xAB9FE), t, customEffTexPath.c_str(), false, cube);
    }
    Bool female = gCurrentPlayerLoadFaceType >= 3 && gCurrentPlayerLoadFaceType <= 7;
    return CallMethodAndReturnDynGlobal<int>(RendererAddress(0xAB9FE), t, female ? "eff_female.dds" : "eff.dds", cached, cube);
}

void *METHOD Test1(void *t, DUMMY_ARG, int a) {
    Error("Test");
    return CallMethodAndReturnDynGlobal<void *>(RendererAddress(0x83D23), t, a);
}

template<UInt Func>
Char const* METHOD OnGetHeadResourceName(void* t, DUMMY_ARG, Char *filename, Int a3, Bool* bExists) {
    UInt headId = *raw_ptr<UInt>(t, 4);
    void* pT = t;
    if (headId != 0 && !AssetExists(Utils::Format("m228__%d.o", headId))) {
        static UChar buf[60];
        memcpy(buf, t, 60);
        pT = buf;
        *raw_ptr<UInt>(pT, 4) = 0;
    }
    return CallMethodAndReturnDynGlobal<Char const*>(RendererAddress(Func), pT, filename, a3, bExists);
}

int OnRenderModel() {
    void *device = *(void **)(RendererAddress(0x38E3D8));
    CallVirtualMethod<57>(device, device, 22, 3);
    return CallAndReturnDynGlobal<int>(RendererAddress(0xFFE0));
}

UInt HeadCamera_XOffset_retn = 0;
Float HeadCamera_XOffset_x = -1.6f;

void __declspec(naked) HeadCamera_XOffset() {
    __asm {
        fld HeadCamera_XOffset_x
        fstp dword ptr[ebp - 0x20]
        jmp HeadCamera_XOffset_retn
    }
}

void OnFormatFaceFileName(char *buf, char const *format, int sideburns, int beardColor, int beard, int skinType, int skinColor) {
    sprintf(buf, format, sideburns, beardColor, skinType >= 3 ? 0 : beard, skinType, skinColor);
}

void Install3dPatches() {

    FindAssets(ASSETS_DIR, "");

    //patch::RedirectCall(RendererAddress(0x1E750), OnRenderModel);

    //Error("%X", RendererAddress(0));
    patch::RedirectJump(RendererAddress(0x108100), FileIO_GetFilePath);

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

    // Head camera
    patch::SetDouble(RendererAddress(0x2CC110), 91.0);
    //HeadCamera_XOffset_retn = RendererAddress(0x83E50);
    //patch::RedirectJump(RendererAddress(0x83E4B), HeadCamera_XOffset);

    // starhead
    patch::RedirectCall(RendererAddress(0x91A18), OnGetHeadResourceName<0x920B1>);
    patch::RedirectCall(RendererAddress(0x9E75C), OnGetHeadResourceName<0x920B1>);
    patch::RedirectCall(RendererAddress(0x91949), OnGetHeadResourceName<0x91B7D>);
    patch::RedirectCall(RendererAddress(0x9EC00), OnGetHeadResourceName<0x91B7D>);
    patch::RedirectCall(RendererAddress(0x919B0), OnGetHeadResourceName<0x91E11>);
    patch::RedirectCall(RendererAddress(0x9F63E), OnGetHeadResourceName<0x91E11>);
    patch::RedirectCall(RendererAddress(0x919E4), OnGetHeadResourceName<0x91EAA>);
    patch::RedirectCall(RendererAddress(0x9F68E), OnGetHeadResourceName<0x91EAA>);

    // disable beard for female skin types
    patch::RedirectCall(RendererAddress(0x91C07), OnFormatFaceFileName);

   // const unsigned int kitW = 1024;
   // const unsigned int kitH = 2048;
   // 
   // patch::SetUInt(RendererAddress(0x8F34B + 6), kitW);
   // patch::SetUInt(RendererAddress(0x8F355 + 6), kitH);
   // patch::SetUInt(RendererAddress(0x8F337 + 1), kitW * 2);
   // patch::SetUInt(RendererAddress(0x8F40C + 6), kitW);
   // patch::SetUInt(RendererAddress(0x8F418 + 6), kitH);
   // patch::SetUInt(RendererAddress(0x8F3E9 + 1), kitW * kitH * 4 + 0x100);
   // patch::SetUInt(RendererAddress(0x8F48D + 1), kitW);
   // patch::SetUInt(RendererAddress(0x8F488 + 1), kitH);
   // patch::SetUInt(RendererAddress(0x8F35F + 1), kitW);
    //patch::SetUChar(RendererAddress(0xAB39C), 0xC3);

    //patch::SetUChar(RendererAddress(0x901AF + 1), 3);
    //patch::SetUChar(RendererAddress(0x901C9 + 1), 3);
    //patch::SetUChar(RendererAddress(0x901E2 + 1), 3);

    //patch::RedirectCall(RendererAddress(0x7BCDC), Test1);
}
