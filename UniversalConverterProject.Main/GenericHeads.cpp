#include "GenericHeads.h"

using namespace plugin;

unsigned int beardTypes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
unsigned int skinColors[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
unsigned int hairColors[] = { 0, 1, 2, 3, 4, 5, 6, 7, 9 };
unsigned int hairTypes[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162 };
unsigned int headTypes[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,500,501,502,503,504,505,506,507,508,509,510,511,512,513,514,515,516,517,518,519,520,1000,1001,1002,1003,1004,1005,1006,1007,1008,1009,1010,1011,1012,1013,1014,1015,1016,1017,1018,1500,1501,1502,1503,1504,1505,1506,1507,1508,1509,1510,1511,1512,1513,1514,1515,1516,1517,1518,1519,1520,1521,1522,1523,1524,1525,1526,2000,2001,2002,2003,2004,2005,2006,2007,2008,2009,2010,2012,2500,2501,2502,2503,2504,2505,2506,3000,3001,3002,3003,3004,3005,3500,3501,3502,3503,3504,3505,4000,4001,4002,4003,4500,4501,4502,5000,5001,5002,5003,521,522,523,524,525,526,527,528,529,530,531,532,533,534,535,536,537,538,539,540,541,542,543,544,545,546,547,548,549,550,551,552,553,554,555,556,557,558,559,560,561,562,1019,1020,1021,1022,1023,1024,1025,1026,1027,1527,1528,2011,2013,2014,2015,2016,2017,2019,2020,2021,2022,2023,2024,2025,2026,2027,2028,2029,2030,2507,2508,2509,2510,2511,2512,2513,2514,2515,2516,2517,2518,4525 };
unsigned int eyeColors[] = { 1,2,9,5,4,7,0,3,8,6 };

void PatchGenericHeads(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

    // generic heads

    // sideburns
        patch::SetUInt(0x24A1854 + 4 * 1, 0); // sideburns 1

        // beard types
        patch::SetUInt(0x24A1880, std::size(beardTypes));
        patch::SetPointer(0x40BE90 + 0x4 + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[eax*4]
        patch::SetPointer(0x40C650 + 0xDA + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[edx*4]
        patch::SetPointer(0x413CA0 + 0x23D + 4, beardTypes); // movzx   edx, byte ptr ds:gFifaBeardTypes[ecx*4]
        patch::SetPointer(0x4534F0 + 0x2A + 3, beardTypes); // mov     edx, ds:gFifaBeardTypes[edx*4]
        patch::SetPointer(0xCB5AE0 + 0x168 + 3, beardTypes); // mov     eax, ds:gFifaBeardTypes[ebx*4]
        patch::SetPointer(0x30994DC, beardTypes); // .data:030994DC	dd offset gFifaBeardTypes
        patch::SetPointer(0xEA264D + 1, &beardTypes[0]);
        patch::SetPointer(0xEA2648 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA265E + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA2675 + 1, &beardTypes[0]);
        patch::SetPointer(0xEA2670 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA268A + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA269C + 1, &beardTypes[0]);
        patch::SetPointer(0xEA25CD + 1, &beardTypes[0]);
        patch::SetPointer(0xEA25C8 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA25DE + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA25F7 + 1, &beardTypes[0]);
        patch::SetPointer(0xEA19EC + 1, &beardTypes[0]);
        patch::SetPointer(0xEA19E7 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA19F9 + 1, &beardTypes[std::size(beardTypes)]);
        patch::SetPointer(0xEA1A08 + 1, &beardTypes[0]);

        // skin colors
        patch::SetUInt(0x24A1884, std::size(skinColors));
        patch::SetPointer(0x40BEB0 + 0x7 + 3, skinColors); // mov     eax, ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x40C650 + 0x82 + 3, skinColors); // mov     ecx, ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x413BB0 + 0xAB + 3, skinColors); // mov     al, byte ptr ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x413CA0 + 0x259 + 4, skinColors); // movzx   ecx, byte ptr ds:gFifaSkinColors[eax*4]
        patch::SetPointer(0x4534F0 + 0x59 + 3, skinColors); // mov     edx, ds:gFifaSkinColors[edx*4]
        patch::SetPointer(0xCB5AE0 + 0x78 + 3, skinColors); // mov     eax, ds:gFifaSkinColors[ebx*4]
        patch::SetPointer(0x30994CC, skinColors); // .data:030994CC	dd offset gFifaSkinColors
        patch::SetPointer(0xEA1C63 + 1, &skinColors[0]);
        patch::SetPointer(0xEA1C5E + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0xEA1C74 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0xEA1C7F + 1, &skinColors[0]);
        patch::SetPointer(0xEA1EE0 + 1, &skinColors[0]);
        patch::SetPointer(0xEA1EDB + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0xEA1EF1 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0xEA1F07 + 1, &skinColors[0]);
        patch::SetPointer(0xEA1F02 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0xEA1F18 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0xEA1F23 + 1, &skinColors[0]);
        patch::SetPointer(0xEA1A6C + 1, &skinColors[0]);
        patch::SetPointer(0xEA1A67 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0xEA1A79 + 1, &skinColors[std::size(skinColors)]);
        patch::SetPointer(0xEA1A88 + 1, &skinColors[0]);

        // hair colors
        patch::SetUInt(0x24A1890, std::size(hairColors));
        patch::SetPointer(0x40BE70 + 0x7 + 3, hairColors); // mov     eax, ds:gFifaHairColors[eax*4]
        patch::SetPointer(0x40C650 + 0x18D + 3, hairColors); // mov     ecx, ds:gFifaHairColors[eax*4]
        patch::SetPointer(0x413BB0 + 0xBB + 4, hairColors); // movzx   ecx, byte ptr ds:gFifaHairColors[ecx*4]
        patch::SetPointer(0x413CA0 + 0x275 + 4, hairColors); // movzx   eax, byte ptr ds:gFifaHairColors[edx*4]
        patch::SetPointer(0x4534F0 + 0x1C + 3, hairColors); // mov     edx, ds:gFifaHairColors[edx*4]
        patch::SetPointer(0xCB5AE0 + 0x2A8 + 3, hairColors); // mov     eax, ds:gFifaHairColors[ebx*4]
        patch::SetPointer(0x30994E4, hairColors); // .data:030994E4	dd offset gFifaHairColors
        patch::SetPointer(0x5A0269 + 1, &hairColors[0]);
        patch::SetPointer(0x5A0264 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A027A + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0285 + 1, &hairColors[0]);
        patch::SetPointer(0x5A036F + 1, &hairColors[0]);
        patch::SetPointer(0x5A036A + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0380 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A0397 + 1, &hairColors[0]);
        patch::SetPointer(0x5A0392 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A03AC + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0x5A03B8 + 1, &hairColors[0]);
        patch::SetPointer(0xEA196C + 1, &hairColors[0]);
        patch::SetPointer(0xEA1967 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0xEA1979 + 1, &hairColors[std::size(hairColors)]);
        patch::SetPointer(0xEA1988 + 1, &hairColors[0]);

        // hair models
        patch::SetUInt(0x24A187C, std::size(hairTypes));
        patch::SetPointer(0x40BE60 + 0x4 + 3, hairTypes); // mov     eax, ds:gFifamGenericHairIdToFIFA[eax*4]
        patch::SetPointer(0x40C650 + 0x56 + 3, hairTypes); // mov     eax, ds:gFifamGenericHairIdToFIFA[edx*4]
        patch::SetPointer(0x413BB0 + 0x3F + 4, hairTypes); // movzx   edx, byte ptr ds:gFifamGenericHairIdToFIFA[ecx*4]
        patch::SetPointer(0x413CA0 + 0x1D0 + 4, hairTypes); // movzx   eax, byte ptr ds:gFifamGenericHairIdToFIFA[edx*4]
        patch::SetPointer(0x4534F0 + 0x8 + 3, hairTypes); // mov     edx, ds:gFifamGenericHairIdToFIFA[ecx*4]
        patch::SetPointer(0xCB5AE0 + 0x258 + 3, hairTypes); // mov     eax, ds:gFifamGenericHairIdToFIFA[ebx*4]
        patch::SetPointer(0x30994C8, hairTypes); // .data:030994C8	dd offset gFifamGenericHairIdToFIFA
        patch::SetPointer(0xEA2210 + 1, &hairTypes[0]);
        patch::SetPointer(0xEA220B + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA2221 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA2238 + 1, &hairTypes[0]);
        patch::SetPointer(0xEA2233 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA224D + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA2259 + 1, &hairTypes[0]);
        patch::SetPointer(0xEA20D1 + 1, &hairTypes[0]);
        patch::SetPointer(0xEA20CC + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA20E2 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA20ED + 1, &hairTypes[0]);
        patch::SetPointer(0xEA192C + 1, &hairTypes[0]);
        patch::SetPointer(0xEA1927 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA1939 + 1, &hairTypes[std::size(hairTypes)]);
        patch::SetPointer(0xEA1948 + 1, &hairTypes[0]);

        // head models
        patch::SetUInt(0x24A1878, std::size(headTypes));
        patch::SetPointer(0x40BE50 + 0x3 + 3, headTypes); // mov     eax, ds:gFifamGenericFaceIdToFIFA[eax*4]
        patch::SetPointer(0x40C650 + 0x25 + 3, headTypes); // mov     edx, ds:gFifamGenericFaceIdToFIFA[ecx*4]
        patch::SetPointer(0x413BB0 + 0x2D + 3, headTypes); // mov     eax, ds:gFifamGenericFaceIdToFIFA[edx*4]
        patch::SetPointer(0x413CA0 + 0x1B8 + 3, headTypes); // mov     ecx, ds:gFifamGenericFaceIdToFIFA[eax*4]
        patch::SetPointer(0x4534F0 + 0x37 + 3, headTypes); // mov     edx, ds:gFifamGenericFaceIdToFIFA[edx*4]
        patch::SetPointer(0xCB5AE0 + 0x28 + 3, headTypes); // mov     eax, ds:gFifamGenericFaceIdToFIFA[ebx*4]
        patch::SetPointer(0x30994C4, headTypes); // .data:off_30994C4	dd offset gFifamGenericFaceIdToFIFA
        patch::SetPointer(0xEA1E31 + 1, &headTypes[0]);
        patch::SetPointer(0xEA1E2C + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1E42 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1E4D + 1, &headTypes[0]);
        patch::SetPointer(0xEA1E6C + 1, &headTypes[0]);
        patch::SetPointer(0xEA1E67 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1E7D + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1E88 + 1, &headTypes[0]);
        patch::SetPointer(0xEA1BF1 + 1, &headTypes[0]);
        patch::SetPointer(0xEA1BEC + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1C02 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1C0D + 1, &headTypes[0]);
        patch::SetPointer(0xEA18EC + 1, &headTypes[0]);
        patch::SetPointer(0xEA18E7 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA18F9 + 1, &headTypes[std::size(headTypes)]);
        patch::SetPointer(0xEA1908 + 1, &headTypes[0]);

        patch::SetUInt(0x24A1898, std::size(eyeColors));
        patch::SetPointer(0x40BED0 + 0x7 + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[eax*4]
        patch::SetPointer(0x40C650 + 0x15E + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0x413BB0 + 0x6F + 4, eyeColors); // movzx   edx, byte ptr ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0x413CA0 + 0x208 + 4, eyeColors); // movzx   ecx, byte ptr ds:gFifaEyeColors[eax*4]
        patch::SetPointer(0x4534F0 + 0x48 + 3, eyeColors); // mov     edx, ds:gFifaEyeColors[edx*4]
        patch::SetPointer(0xCB5AE0 + 0xC8 + 3, eyeColors); // mov     eax, ds:gFifaEyeColors[ebx*4]
        patch::SetPointer(0x30994D8, eyeColors); // .data:030994D8	dd offset gFifaEyeColors
        patch::SetPointer(0xEA2019 + 1, &eyeColors[0]);
        patch::SetPointer(0xEA2014 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA202A + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA2040 + 1, &eyeColors[0]);
        patch::SetPointer(0xEA203B + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA2051 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA205C + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1D73 + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1D6E + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1D84 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1D8F + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1DAF + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1DAA + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1DC4 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1DD0 + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1AEC + 1, &eyeColors[0]);
        patch::SetPointer(0xEA1AE7 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1AF9 + 1, &eyeColors[std::size(eyeColors)]);
        patch::SetPointer(0xEA1B08 + 1, &eyeColors[0]);
    }
}