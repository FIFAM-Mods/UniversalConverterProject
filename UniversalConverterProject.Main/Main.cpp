#include "plugin.h"
#include "LicenseCheck.h"
#include "shared.h"
#include "UcpSettings.h"
#include "WorldCup.h"
#include "LeagueSelection.h"
#include "NationalTeamManagers.h"
#include "GfxCoreHook.h"
#include "PlayerMatchEvents.h"
#include "FifaWorldPlayerGala.h"
#include "PlayerDevelopment.h"
#include "StaffNames.h"
#include "ForeignersLimit.h"
#include "TeamControl.h"
#include "EasyEdit.h"
#include "Competitions.h"
#include "EABFFixes.h"
#include "SpareEmployeeFix.h"
#include "EuCountries.h"
#include "Minikits.h"
#include "Balls.h"
#include "ExcludeNames.h"
#include "3dMatchStandings.h"
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
#include "PlayerAppearance.h"
#include "CustomStadiums.h"
#include "GenerateBigIdx.h"
#include "PortraitDialog.h"
#include "PlayerTalks.h"
#include "TheClubScreenExtended.h"
#include "ExtendedPlayer.h"
#include "ExtendedTeam.h"
#include "PlayerNameEdit.h"
#include "InterfaceTheme.h"
#include "DatabaseOptions.h"
#include "CustomShaders.h"
#include "YouthGenCountries.h"
#include "ClubSponsors.h"
#include "Achievements.h"
#include "ReserveNames.h"
#include "ExtendedCountry.h"
#include "StatsFifaRanking.h"
#include "MobilePhone.h"
#include "UEFALeaguePhase.h"
#include "Assessment.h"
#include "MainMenu.h"
#include "ScreenResolution.h"
#include "AbilityColors.h"
#include "Languages.h"
#include "NewspaperEuroStats.h"
#include "EuropeanGoldenShoe.h"
#include "LeagueSplit.h"
#include "PlayerLevelPatches.h"
#include "WidgetContinental.h"
#include "Finances.h"
#include "Shared.h"
#include <ShlObj.h>

AUTHOR_INFO("Universal Converter Project Main ASI plugin, made by Dmitri");

using namespace plugin;

class UniversalConverterProject {
public:
    static FM::Version v;

    template<Bool WindowedModeClosedWindow>
    static void METHOD OnExitGameSaveGameOptions(void *game, DUMMY_ARG, UInt resolution) {
        CallMethod<0x44F0E0>(game, resolution);
        if (WindowedModeClosedWindow)
            Call<0x4506E0>(); // write GameOptions.dat
        WindowedModeOnExitGame();
        Settings::GetInstance().Save();
        SaveTestFile();
        Path introMusicPath = R"(data\audio\music\0 Intro Music (TBD).asf)";
        std::error_code ec;
        if (exists(introMusicPath, ec))
            remove(introMusicPath, ec);
    }

    static Char const *GetGameVersion() {
        static Char gameVersionStr[256] = {};
        static Bool initialized = false;
        if (!initialized) {
            StringA appName = WtoA(GetFullAppName());
            strcpy(gameVersionStr, appName.c_str());
        }
        return gameVersionStr;
    }

    UniversalConverterProject() {
        v = FM::GetAppVersion();
        if (v.id() == ID_FM_13_1030_RLD) {
            if (!CheckPluginName(Magic<'U','n','i','v','e','r','s','a','l','C','o','n','v','e','r','t','e','r','P','r','o','j','e','c','t','.','M','a','i','n','.','a','s','i'>()))
                return;
            if (GetPatchTestVersion()) {
                ::Message(L"This is a test patch version (%s).\nPlease do not redistribute it.", GetPatchTestVersion());
            }
            if (exists(FM::GameDirPath(L"plugins\\CustomTranslation.asi"))) {
                ::Error("This plugin is not compatible with CustomTranslation plugin. Please delete CustomTranslation plugin.");
            }
            Settings::GetInstance().Load();
            SafeLog::Clear();
            SafeLog::Write(Utils::Format(L"Log started on %s", Utils::CurrentTime()));
            GenerateBigIdx();
            patch::SetUChar(0x451B92, 0xEB); // remove EA logo
            patch::SetPointer(0x30655F4, L"jpg"); // loadscreens tpi patch
            //patch::SetUInt(0x108F675 + 1, 0x2019);
            patch::SetUInt(0x1338FE4 + 1, 0x10000000); // 64 MB > 256 MB master.dat size limit
            const UInt SaveGameVersion = 48; // 48 - FM 2026 1.0 savefile
            patch::SetUInt(0x1082C02 + 3, SaveGameVersion); // new savegame version
            patch::SetUChar(0x1080E29 + 2, UChar(SaveGameVersion)); // remake the code if version >= 128 is needed

            if (!exists("locale.ini"))
                Warning("File \"locale.ini\" is not found. Make sure this file is present in the game folder.");

            PatchTranslation(v);

            static std::wstring gameVersionStr = GetPatchNameWithVersion(false);
            patch::SetPointer(0x4D2880 + 1, (void *)gameVersionStr.c_str());

            patch::RedirectCall(0x4523DD, OnExitGameSaveGameOptions<false>);
            patch::RedirectCall(0x4515E0, OnExitGameSaveGameOptions<true>);

            patch::RedirectJump(0x452F20, GetGameVersion);

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
            PatchInterfaceTheme(v);
            PatchDatabaseOptions(v);
            PatchWorldCup(v);
            PatchLeagueSelection(v);
            PatchNationalTeamManagers(v);
            PatchGfxCoreHook(v);
            PatchPlayerMatchEvents(v);
            PatchFifaWorldPlayerGala(v);
            //PatchPlayerDevelopment(v);
            PatchStaffNames(v);
            PatchForeignersLimit(v);
            PatchTeamControl(v);
            //InstallEasyEdit(v); -- deprecated since version 2022.1. do not enable
            PatchCompetitions(v);
            PatchLeagueSplit(v);
            PatchSpareEmployeeFix(v);
            PatchEABFFixes(v);
            PatchEuCountries(v);
            PatchMinikits(v);
            PatchBalls(v);
            PatchExcludeNames(v);
            Patch3dMatchStandings(v);
            PatchKits(v);
            PatchMiscFormerCupOpponents(v);
            PatchPlayerAccessories(v);
            PatchFormations(v);
            PatchWindowedMode(v);
            Patch3dAdBoardsAndBanners(v);
            PatchPitch(v);
            PatchScouting(v);
            PatchMedia(v);
            PatchPlayerAppearance(v);
            PatchCustomStadiums(v);
            PatchPortraitDialog(v);
            //PatchPlayerTalks(v);
            PatchTheClubScreenExtended(v);
            PatchExtendedPlayer(v);
            PatchExtendedTeam(v);
            PatchPlayerNameEdit(v);
            PatchYouthGenCountries(v);
            PatchClubSponsors(v);
            //PatchAchievements(v);
            PatchReserveNames(v);
            PatchExtendedCountry(v);
            PatchStatsFifaRanking(v);
            PatchMobilePhone(v);
            PatchUEFALeaguePhase(v);
            PatchAssessment(v);
            PatchMainMenu(v);
            PatchScreenResolution(v);
            PatchAbilityColors(v);
            PatchLanguages(v);
            PatchNewspaperEuroStats(v);
            PatchEuropeanGoldenShoe(v);
            PatchPlayerLevelPatches(v);
            PatchWidgetContinental(v);
            PatchFinances(v);

#ifdef BETA
            DoBetaPatches(v);
#endif
        }
    }

    ~UniversalConverterProject() {
        if (v.id() == ID_FM_13_1030_RLD) {
            UnpatchEABFFixes(v);
            if (Settings::GetInstance().EnableCustomShaders)
                ClearShadersDataStorage();
        }
    }
};

FM::Version UniversalConverterProject::v;
UniversalConverterProject universalConverterProject;
