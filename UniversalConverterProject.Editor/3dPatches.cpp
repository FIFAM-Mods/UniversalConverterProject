#include "3dPatches.h"
#include "RendererHook.h"
#include "Patch.h"

void *METHOD OnCreateFileIO(void *fileIO) {
    CallMethodDynGlobal(RendererAddress(0x108230), fileIO);
    std::wstring zdataPath = FM::GetGameDir() + L"data\\assets\\";
    wchar_t const *paths[] = { zdataPath.c_str() };
    CallMethodDynGlobal(RendererAddress(0x108540), fileIO, paths, 1);
    return fileIO;
}

void Install3dPatches() {
    patch::RedirectCall(RendererAddress(0x92579), OnCreateFileIO);

    patch::SetChar(RendererAddress(0xAB61B + 3), 't');
    patch::SetChar(RendererAddress(0xAB625 + 3), 'g');
    patch::SetChar(RendererAddress(0xAB62F + 3), 'a');

    patch::SetPointer(RendererAddress(0xAB5EF + 1), ".dds");
    patch::SetPointer(RendererAddress(0x91E47 + 1), "hair_cm_-%d_%d.dds");
    patch::SetPointer(RendererAddress(0x91E29 + 1), "cm_%d.dds");
    patch::SetPointer(RendererAddress(0x91EDA + 1), "hair_cmb_-%d.dds");
    patch::SetPointer(RendererAddress(0x91EC2 + 1), "cmbhair_%d.dds");
}
