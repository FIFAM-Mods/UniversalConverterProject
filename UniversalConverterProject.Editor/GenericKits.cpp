#include "GenericKits.h"

const unsigned int NUM_GEN_SHIRTS = 60;
const unsigned int NUM_GEN_SHORTS = 26;
const unsigned int NUM_GEN_SOCKS = 26;

unsigned int KitColors[] = {
    0x000040,0x000080,0x0000C0,0x0000FF,0x4040FF,0x4040C0,0x404080,0x8080FF,
    0x4000C0,0x8040C0,0x4000FF,0x8000FF,0x8040FF,0xC080FF,0x8080C0,0xC0C0FF,
    0x400080,0x8000C0,0xC000FF,0xFF00FF,0xC040FF,0xFF40FF,0xC040C0,0xFF80FF,
    0x400040,0x800080,0xC000C0,0x804080,0xC080C0,0xFFC0FF,0xFF80C0,0xFF40C0,
    0x800040,0xC00080,0xFF0080,0xFF00C0,0xFF4080,0xC04080,0xC04040,0x804040,
    0xFF0040,0xC00040,0xFF4040,0xFF8040,0xC08040,0xFF8080,0xC08080,0xFFC0C0,
    0x400000,0x800000,0xC00000,0xFF0000,0xFF4000,0xC04000,0x804000,0xFFC080,
    0xFF8000,0xC08000,0xFFC000,0xFFC040,0xC0C040,0xC0C080,0x808040,0xFFFFC0,
    0x404000,0x808000,0xC0C000,0xFFFF00,0xFFFF40,0xFFFF80,0xC0FF80,0xC0FF40,
    0xC0FF00,0x80C000,0x80C040,0x80FF00,0x80FF40,0x40FF00,0x80C080,0x80FF80,
    0x40FF40,0x40C040,0x40C000,0x00FF40,0x00FF00,0x00C040,0x00FF80,0x40FF80,
    0x004000,0x408000,0x008000,0x00C000,0x008040,0x408040,0x00C080,0x40C080,
    0x40FFC0,0x80FFC0,0xC0FFC0,0x00FFC0,0x00C0C0,0x40C0C0,0x80C0C0,0x008080,
    0x00FFFF,0x40FFFF,0x80FFFF,0xC0FFFF,0x80C0FF,0x40C0FF,0x00C0FF,0x4080C0,
    0x004080,0x0040C0,0x0080C0,0x0040FF,0x0080FF,0x4080FF,0x408080,0x004040,
    0x000000,0x202020,0x404040,0x808080,0xC0C0C0,0xE0E0E0,0xFFFFFF
};

const unsigned int NUM_KIT_COLORS = std::size(KitColors);

void PatchGenericKits(FM::Version v) {
    if (v.id() == VERSION_ED_13) {
        patch::SetUChar(0x435FA4 + 1, NUM_GEN_SHIRTS);
        patch::SetUChar(0x435FB5 + 1, NUM_GEN_SHIRTS);
        patch::SetUChar(0x435FC6 + 1, NUM_GEN_SHORTS);
        patch::SetUChar(0x435FD7 + 1, NUM_GEN_SHORTS);
        patch::SetUChar(0x435FE8 + 1, NUM_GEN_SOCKS);
        patch::SetUChar(0x435FF9 + 1, NUM_GEN_SOCKS);
        patch::SetUChar(0x511960 + 1, NUM_GEN_SHIRTS);
        patch::SetUChar(0x511970 + 1, NUM_GEN_SHORTS);
        patch::SetUChar(0x511982 + 1, NUM_GEN_SOCKS);
        //patch::SetUInt(0x436EC0 + 1, 32);
        patch::SetUChar(0x437292 + 2, NUM_KIT_COLORS);
        patch::SetUChar(0x436FD2 + 2, NUM_KIT_COLORS);
        patch::SetUChar(0x436F34 + 2, NUM_KIT_COLORS);
        patch::SetUChar(0x436FCA + 2, 16);
        patch::SetPointer(0x436EA4 + 3, KitColors);
        patch::SetPointer(0x436F39 + 3, KitColors);
        patch::SetPointer(0x436F40 + 4, (unsigned char *)KitColors + 2);
        patch::SetPointer(0x43718C + 3, KitColors);
        patch::SetUInt(0x6C95E8 + 4, NUM_KIT_COLORS);
    }
}
