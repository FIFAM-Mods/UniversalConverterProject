#include "Compression.h"
#include <Windows.h>
#include "license_check/license_check.h"

HMODULE dllHandle;

unsigned int translate_addr(unsigned int addr) {
	return (unsigned int)dllHandle + addr;
}

unsigned int get_decompressed_size(void *compressHeader) {
	return ((unsigned int(__cdecl*)(void*))translate_addr(0x3047D0))(compressHeader);
}

void decompress(void *inData, void *outData) {
	((void(__cdecl*)(void*, void*))translate_addr(0x304660))(inData, outData);
}

unsigned int compress(void *inData, unsigned int inSize, void *outData, bool bHasData) {
    unsigned char *data_1CA3 = new unsigned char[0x1CA3 * 4];
    unsigned char *data_4000 = new unsigned char[0x4000 * 4];
    *(unsigned char **)translate_addr(0xBF31E8) = data_1CA3;
    *(unsigned char **)translate_addr(0xBF31EC) = data_4000;
	unsigned int result = ((unsigned int(__cdecl*)(void*, unsigned int, void*, bool))translate_addr(0x304E20))(inData, inSize, outData, bHasData);
    delete[] data_1CA3;
    delete[] data_4000;
    return result;
}

bool compression_open() {
    std::wstring gfxCoreModuleDll = Magic<'G','f','x','C','o','r','e','.','d','l','l'>(2908048495);
	LoadLibraryW(gfxCoreModuleDll.c_str());
	dllHandle = GetModuleHandleW(gfxCoreModuleDll.c_str());
	if (dllHandle == 0)
		return false;
	return true;
}

void compression_close() {
	FreeLibrary(dllHandle);
	dllHandle = nullptr;
}

void enable_compression_extended_file_size() {
    DWORD oldp;
    VirtualProtect((LPVOID)translate_addr(0x304DA9), 2, PAGE_READWRITE, &oldp);
    *(unsigned short *)translate_addr(0x304DA9) = 0x9090;
    VirtualProtect((LPVOID)translate_addr(0x304DA9), 2, oldp, &oldp);
}
