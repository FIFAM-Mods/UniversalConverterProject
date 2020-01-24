#include "plugin-std.h"
#include "license_check/license_check.h"
#include "shared.h"
#include "Settings.h"
#include "InternationalCupsFix.h"
#include "WorldCup.h"
#include "LeagueSelection.h"
#include "MasterDatLimit.h"
#include "Talent6Stars.h"
#include "TournamentFinalists.h"
#include "NationalTeamManagers.h"
#include "DatabaseName.h"
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
#include "3dAdboards.h"
#include "Pitch.h"
#include "Beta.h"
#include "Scouting.h"
#include "Media.h"
#include "Translation.h"

AUTHOR_INFO("Universal Converter Project Main ASI plugin, made by Dmitry/DK22");

using namespace plugin;

class UniversalConverterProject {
public:
    static FM::Version v;

    UniversalConverterProject() {
        if (!CheckLicense(Magic<'U','n','i','v','e','r','s','a','l','C','o','n','v','e','r','t','e','r','P','r','o','j','e','c','t','.','M','a','i','n','.','a','s','i'>(1583797143)))
            return;
        v = FM::GetAppVersion();
        if (v.id() == ID_FM_13_1030_RLD) {
            patch::SetUChar(0x451B92, 0xEB); // remove EA logo

            static std::wstring sJpg = Magic<'j','p','g'>(1947963115);
            patch::SetPointer(0x30655F4, (void *)sJpg.c_str()); // loadscreens tpi patch

            if (!exists(Magic<'l','o','c','a','l','e','.','i','n','i'>(1515555421))) {
                Warning(Magic<'F','i','l','e',' ','"','l','o','c','a','l','e','.','i','n','i','"',' ','i','s',' ','n','o','t',' ','f','o','u','n','d','.',' ','M','a','k','e',' ','s','u','r','e',' ','t','h','i','s',' ','f','i','l','e',' ','i','s',' ','p','r','e','s','e','n','t',' ','i','n',' ','t','h','e',' ','g','a','m','e',' ','f','o','l','d','e','r','.'>(1104203179));
            }

            PatchTranslation(v);

            static std::wstring gameVersionStr = GetPatchNameWithVersion(true);
            patch::SetPointer(0x4D2880 + 1, (void *)gameVersionStr.c_str());
        }
        std::wstring testFileName = Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','u','c','p','-','l','a','u','n','c','h','e','d'>(3552446780);
        std::wstring readMode = Magic<'r','b'>(3009798726);
        std::wstring writeMode = Magic<'w','b'>(2430180274);
        FILE *testFile = _wfopen(testFileName.c_str(), readMode.c_str());
        if (!testFile) {
            testFile = _wfopen(testFileName.c_str(), writeMode.c_str());
            if (testFile)
                fclose(testFile);
        }
        else {
            fclose(testFile);
            if (v.id() == ID_FM_13_1030_RLD) {
                patch::RedirectJump(0x4B43BD, (void *)0x4B4420);
                patch::RedirectJump(0x4B4405, (void *)0x4B4420);
            }
        }
        PatchDatabaseName(v);
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
        InstallEasyEdit(v);
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
        Patch3dAdBoards(v);
        PatchPitch(v);
        PatchScouting(v);
        //PatchMedia(v);
    #ifdef BETA
        DoBetaPatches(v);
    #endif
    }

    ~UniversalConverterProject() {
        CloseWindowedMode(v);
    }
};

FM::Version UniversalConverterProject::v;
UniversalConverterProject universalConverterProject;

wchar_t const *GetMainDatabaseName() {
    static std::wstring maindbname = Magic<'U','n','i','v','e','r','s','a','l','C','o','n','v','e','r','t','e','r','P','r','o','j','e','c','t','D','a','t','a','b','a','s','e','.','u','c','p','d','b'>(3800056213);
    return maindbname.c_str();
}

wchar_t const *GetEditorDatabaseName() {
    static std::wstring editordbname = Magic<'d','a','t','a','b','a','s','e','\\','U','C','P','E','d','i','t','o','r','D','a','t','a','b','a','s','e','.','u','c','p','d','b'>(3967910619);
    return editordbname.c_str();
}

wchar_t const *GetWorldCupDatabaseName() {
    static std::wstring wcdbname = Magic<'U','n','i','v','e','r','s','a','l','C','o','n','v','e','r','t','e','r','P','r','o','j','e','c','t','D','a','t','a','b','a','s','e','_','W','C','.','u','c','p','d','b'>(3738894356);
    return wcdbname.c_str();
}

char const *GetMainMenuScreenName() {
    std::wstring wscreenname = Magic<'S','c','r','e','e','n','s','/','1','0','U','n','i','v','e','r','s','a','l','C','o','n','v','e','r','t','e','r','P','r','o','j','e','c','t','S','t','a','r','t','M','a','i','n','M','e','n','u','.','x','m','l'>(991862529);
    static std::string screenname = WtoA(wscreenname);
    return screenname.c_str();
}

char const *GetDatabaseScreenName() {
    std::wstring wscreenname = Magic<'S','c','r','e','e','n','s','/','1','0','U','n','i','v','e','r','s','a','l','C','o','n','v','e','r','t','e','r','P','r','o','j','e','c','t','S','t','a','r','t','D','a','t','a','b','a','s','e','.','x','m','l'>(4188733198);
    static std::string screenname = WtoA(wscreenname);
    return screenname.c_str();
}

String GetAppName() {
    if (GameLanguage() == L"ger")
        return Magic<'F','u','s','s','b','a','l','l',' ','M','a','n','a','g','e','r'>(868965328);
    if (GameLanguage() == L"fre")
        return Magic<'L','F','P',' ','M','a','n','a','g','e','r'>(2462642185);
    return Magic<'F','I','F','A',' ','M','a','n','a','g','e','r'>(3424558644);
}

String GetPatchName() {
    if (GameLanguage() == L"ger")
        return Magic<'S','e','a','s','o','n',' ','2','0','2','0'>(141786079);
    return Magic<'S','e','a','s','o','n',' ','2','0','2','0'>(141786079);
}

String GetPatchVersion() {
    return Magic<'1','.','0','.','2'>(486041454);
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
    return Magic<'F','M'>(1823017569);
}
