#include "plugin-std.h"
#include "license_check/license_check.h"
#include "shared.h"
#include "UcpSettings.h"
#include "InternationalCupsFix.h"
#include "WorldCup.h"
#include "LeagueSelection.h"
#include "MasterDatLimit.h"
#include "Talent6Stars.h"
#include "TournamentFinalists.h"
#include "NationalTeamManagers.h"
#include "GfxCoreHook.h"
#include "PlayerMatchEvents.h"
#include "FifaWorldPlayerGala.h"
#include "ScoutingSystems.h"
#include "LowestLeagues.h"
#include "PlayerDevelopment.h"
#include "StaffNames.h"
#include "ForeignersLimit.h"
#include "TeamControl.h"
#include "EasyEdit.h"
#include "Competitions.h"
#include "EABFFixes.h"
#include "SpareEmployeeFix.h"
#include "LoansLimit.h"
#include "EuCountries.h"
#include "Minikits.h"
#include "Balls.h"
#include "FemaleNames.h"
#include "3dMatchStandings.h"
#include "IncreaseSalaryOwnedClub.h"
#include "Kits.h"
#include "MiscFormerCupOpponents.h"
#include "PlayerAccessories.h"
#include "Formations.h"
#include "Utils.h"
#include "WindowedMode.h"
#include "3dAdboardsAndBanners.h"
#include "Pitch.h"
#include "Beta.h"
#include "Scouting.h"
#include "Media.h"
#include "Translation.h"
#include "GenericHeads.h"
#include "GameStartTweaks.h"
#include "CustomStadiums.h"
#include "GenerateBigIdx.h"
#include "PortraitDialog.h"
#include "PlayerTalks.h"
#include "TheClubScreenExtended.h"
#include "ExtendedPlayer.h"
#include "PlayerNameEdit.h"
#include "WomensDatabase.h"
#include "InterfaceTheme.h"
#include <ShlObj.h>

AUTHOR_INFO("Universal Converter Project Main ASI plugin, made by Dmitry/DK22");

using namespace plugin;

class UniversalConverterProject {
public:
    static FM::Version v;

    static void OnCloseGame() {
        Call<0xF61490>();
        Settings::GetInstance().Save();
        SaveTestFile();
    }

    UniversalConverterProject() {
        v = FM::GetAppVersion();
        if (v.id() == ID_FM_13_1030_RLD) {
            if (!CheckLicense(Magic<'U','n','i','v','e','r','s','a','l','C','o','n','v','e','r','t','e','r','P','r','o','j','e','c','t','.','M','a','i','n','.','a','s','i'>(1583797143)))
                return;
            Settings::GetInstance().Load();
#ifdef DELETE_LOG
            if (exists("ucp_safe.log"))
                remove("ucp_safe.log");
#endif
            GenerateBigIdx();
            patch::SetUChar(0x451B92, 0xEB); // remove EA logo
            patch::SetPointer(0x30655F4, L"jpg"); // loadscreens tpi patch
            //patch::SetUInt(0x108F675 + 1, 0x2019);
            const UInt SaveGameVersion = 42; // 43
            const UInt SaveGameVersionMin = 42;
            patch::SetUInt(0x1082C02 + 3, SaveGameVersion); // new savegame version
            patch::SetUChar(0x1080E29 + 2, UChar(SaveGameVersionMin)); // remake the code if version >= 128 is needed

            if (!exists("locale.ini"))
                Warning("File \"locale.ini\" is not found. Make sure this file is present in the game folder.");

            PatchTranslation(v);

            static std::wstring gameVersionStr = GetPatchNameWithVersion(false);
            patch::SetPointer(0x4D2880 + 1, (void *)gameVersionStr.c_str());

            patch::RedirectCall(0x45BFB5, OnCloseGame);

            path documentsPath = GetDocumentsPath();
            if (!documentsPath.empty()) {
                path testFileName = GetDocumentsPath() / "Config" / "ucp-launched";
                FILE *testFile = _wfopen(testFileName.c_str(), L"rb");
                if (testFile) {
                    fclose(testFile);
                    patch::RedirectJump(0x4B43BD, (void *)0x4B4420);
                    patch::RedirectJump(0x4B4405, (void *)0x4B4420);
                    IsFirstLaunch() = false;
                }
            }
        }
        PatchInternationalCups(v);
        PatchWorldCup(v);
        PatchLeagueSelection(v);
        PatchMasterDatLimit(v);
        PatchTalentStars(v);
        PatchTournamentFinalists(v);
        PatchNationalTeamManagers(v);
        PatchGfxCoreHook(v);
        PatchPlayerMatchEvents(v);
        PatchFifaWorldPlayerGala(v);
        PatchScoutingSystems(v);
        PatchLowestLeagues(v);
        //PatchPlayerDevelopment(v);
        PatchStaffNames(v);
        PatchForeignersLimit(v);
        PatchTeamControl(v);
        //InstallEasyEdit(v); -- deprecated since version 2022.1. do not enable
        PatchCompetitions(v);
        PatchSpareEmployeeFix(v);
        PatchEABFFixes(v);
        PatchLoansLimit(v);
        PatchEuCountries(v);
        PatchMinikits(v);
        PatchBalls(v);
        PatchFemaleNames(v);
        Patch3dMatchStandings(v);
        PatchIncreaseSalaryOwnedClub(v);
        PatchKits(v);
        PatchMiscFormerCupOpponents(v);
        PatchPlayerAccessories(v);
        PatchFormations(v);
        PatchWindowedMode(v);
        Patch3dAdBoardsAndBanners(v);
        PatchPitch(v);
        PatchScouting(v);
        PatchMedia(v);
        PatchGenericHeads(v);
        PatchGameStartTweaks(v);
        PatchCustomStadiums(v);
        PatchPortraitDialog(v);
        //PatchPlayerTalks(v);
        PatchTheClubScreenExtended(v);
        PatchExtendedPlayer(v);
        PatchPlayerNameEdit(v);
        PatchWomensDatabase(v);
        PatchInterfaceTheme(v);
    #ifdef BETA
        DoBetaPatches(v);
    #endif
    }
};

FM::Version UniversalConverterProject::v;
UniversalConverterProject universalConverterProject;

String GetAppName() {
    if (GameLanguage() == L"ger")
        return L"Fussball Manager";
    if (GameLanguage() == L"fre")
        return L"LFP Manager";
    return L"FIFA Manager";
}

String GetPatchName() {
    return L"2023";
}

String GetPatchVersion() {
    return L"1.0";
}

String GetFullAppName(Bool upperCase) {
    String name = GetAppName() + L' ' + GetPatchName() + L' ' + GetPatchVersion();
    if (upperCase)
        return Utils::ToUpper(name);
    return name;
}

String GetPatchNameWithVersion(Bool upperCase) {
    String name = GetPatchName() + L' ' + GetPatchVersion();
    if (upperCase)
        return Utils::ToUpper(name);
    return name;
}

String GetFMDocumentsFolderName() {
    return L"FM";
}

path GetDocumentsPath() {
    wchar_t documentsDir[MAX_PATH];
    bool foundDocuments = SHGetSpecialFolderPathW(NULL, documentsDir, CSIDL_MYDOCUMENTS, FALSE);
    if (foundDocuments)
        return path(documentsDir) / GetFMDocumentsFolderName();
    return path();
}

void SaveTestFile() {
    path documentsPath = GetDocumentsPath();
    if (!documentsPath.empty()) {
        path testFileName = documentsPath / "Config" / "ucp-launched";
        if (!exists(testFileName)) {
            FILE *testFile = _wfopen(testFileName.c_str(), L"wb");
            if (testFile)
                fclose(testFile);
        }
    }
}

Bool &IsFirstLaunch() {
    static Bool isFirstLaunch = true;
    return isFirstLaunch;
}
