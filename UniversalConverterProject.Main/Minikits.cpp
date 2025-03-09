#include "Minikits.h"
#include "Utils.h"
#include "GameInterfaces.h"

using namespace plugin;

bool ImgExistsAnyOrTpi(std::wstring const &filename) {
    return CallAndReturn<bool, 0xD2CBB0>(filename.c_str(), 0);
}

unsigned int KitFileExists(std::wstring const &fileNameNoExtension) {
    if (ImgExistsAnyOrTpi(fileNameNoExtension + L".tga"))
        return 1;
    if (ImgExistsAnyOrTpi(fileNameNoExtension + L".png"))
        return 2;
    return 0;
}

unsigned char SetImagePath(void *widget, std::wstring const &path) {
    return CallAndReturn<unsigned char, 0xD32860>(widget, path.c_str(), 4, 4);
}

template<unsigned int teamPtrOffset, unsigned int teamIndexOffset, unsigned int imagesOffset, unsigned int originalFunc>
void METHOD Render2dKit(void *t) {
    CDBTeam *club = nullptr;
    if (teamPtrOffset) {
        club = *raw_ptr<CDBTeam *>(t, teamPtrOffset);
    }
    else if (teamIndexOffset) {
        unsigned int teamIndex = *raw_ptr<unsigned int>(t, teamIndexOffset);
        club = CallAndReturn<CDBTeam *, 0xEC8F70>(teamIndex);
    }
    if (!club)
        return;
    Bool customMinikit = false;
    CDBTeamKit *teamKit = club->GetKit();
    Bool homeGenericKit = (*raw_ptr<UInt>(teamKit, 4) & 0xC0000000) == 0;
    if (homeGenericKit) {
        if (club->IsManagedByAI())
            homeGenericKit = false;
        else {
            if (!teamKit->HasUserKitType(0))
                homeGenericKit = false;
        }
    }
    if (!homeGenericKit) {
        unsigned int teamId = *raw_ptr<unsigned int>(club, 0xF0);
        std::wstring teamIdStr = Utils::Format(L"%08X", teamId);
        std::wstring baseFileName = L"data\\minikits\\" + teamIdStr + L"_";
        unsigned int homeKitFmt = KitFileExists(baseFileName + L"h");
        if (homeKitFmt != 0) {
            SetImagePath(*raw_ptr<void *>(t, imagesOffset + 8), L""); // disable layer1_1
            SetImagePath(*raw_ptr<void *>(t, imagesOffset + 16), L""); // disable layer2_1
            SetImagePath(*raw_ptr<void *>(t, imagesOffset + 24), L""); // disable shadow1
            SetImagePath(*raw_ptr<void *>(t, imagesOffset + 12), L""); // disable layer1_2
            SetImagePath(*raw_ptr<void *>(t, imagesOffset + 20), L""); // disable layer2_2
            SetImagePath(*raw_ptr<void *>(t, imagesOffset + 28), L""); // disable shadow2
            SetImagePath(*raw_ptr<void *>(t, imagesOffset + 0), baseFileName + (homeKitFmt == 2 ? L"h.png" : L"h.tga"));
            unsigned int awayKitFmt = KitFileExists(baseFileName + L"a");
            if (awayKitFmt != 0)
                SetImagePath(*raw_ptr<void *>(t, imagesOffset + 4), baseFileName + (homeKitFmt == 2 ? L"a.png" : L"a.tga"));
            else
                SetImagePath(*raw_ptr<void *>(t, imagesOffset + 4), L""); // disable background2
            customMinikit = true;
        }
    }
    if (!customMinikit) {
        SetImagePath(*raw_ptr<void *>(t, imagesOffset + 0), L"art\\Lib\\LineUp\\Shirt\\220x220\\LineUpShirt.tga");
        SetImagePath(*raw_ptr<void *>(t, imagesOffset + 4), L"art\\Lib\\LineUp\\Shirt\\220x220\\LineUpShirt.tga");
        CallMethod<originalFunc>(t);
    }
}

void PatchMinikits(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x654991, Render2dKit<0, 0x21E0, 0x19A0, 0x652E40>);
        patch::RedirectCall(0x6D36C1, Render2dKit<0xBE0, 0, 0xBC0, 0x6D0630>);
    }
}
