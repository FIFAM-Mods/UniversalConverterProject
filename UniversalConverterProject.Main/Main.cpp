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
#include "ExcludeNames.h"
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
#include "CustomStadiums.h"
#include "GenerateBigIdx.h"
#include "PortraitDialog.h"
#include "PlayerTalks.h"
#include "TheClubScreenExtended.h"
#include "ExtendedPlayer.h"
#include "ExtendedTeam.h"
#include "PlayerNameEdit.h"
#include "WomensDatabase.h"
#include "InterfaceTheme.h"
#include "DatabaseOptions.h"
#include "CustomShaders.h"
#include "YouthGenCountries.h"
#include "ClubSponsors.h"
#include "Achievements.h"
#include "ReserveNames.h"
#include <ShlObj.h>

AUTHOR_INFO("Universal Converter Project Main ASI plugin, made by Dmitri");

using namespace plugin;

void GenerateXML() {
    struct PairBox {
        int x, y;
        string name;
        string text;
    };
    vector<PairBox> pairBoxes;
    ofstream f;
    f.open("myxml.xml");
    {
        int baseW[] = { 21, 1145 };
        int baseH[] = { 215, 297, 380, 462, 545, 627, 710, 792 };
        pairBoxes.emplace_back(baseW[0], baseH[0], "TbLast32_1", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[1], baseH[0], "TbLast32_2", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[0], baseH[4], "TbLast32_3", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[1], baseH[4], "TbLast32_4", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[0], baseH[2], "TbLast32_5", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[1], baseH[2], "TbLast32_6", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[0], baseH[6], "TbLast32_7", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[1], baseH[6], "TbLast32_8", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[0], baseH[1], "TbLast32_9", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[1], baseH[1], "TbLast32_10", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[0], baseH[5], "TbLast32_11", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[1], baseH[5], "TbLast32_12", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[0], baseH[3], "TbLast32_13", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[1], baseH[3], "TbLast32_14", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[0], baseH[7], "TbLast32_15", "ID_TOURNAMENT_LAST32");
        pairBoxes.emplace_back(baseW[1], baseH[7], "TbLast32_16", "ID_TOURNAMENT_LAST32");
    }
    {
        int baseW[] = { 161, 1005 };
        int baseH[] = { 256, 421, 586, 751 };
        pairBoxes.emplace_back(baseW[0], baseH[0], "TbLast16_1", "ID_TOURNAMENT_EIGHTFINAL");
        pairBoxes.emplace_back(baseW[1], baseH[0], "TbLast16_2", "ID_TOURNAMENT_EIGHTFINAL");
        pairBoxes.emplace_back(baseW[0], baseH[2], "TbLast16_3", "ID_TOURNAMENT_EIGHTFINAL");
        pairBoxes.emplace_back(baseW[1], baseH[2], "TbLast16_4", "ID_TOURNAMENT_EIGHTFINAL");
        pairBoxes.emplace_back(baseW[0], baseH[1], "TbLast16_5", "ID_TOURNAMENT_EIGHTFINAL");
        pairBoxes.emplace_back(baseW[1], baseH[1], "TbLast16_6", "ID_TOURNAMENT_EIGHTFINAL");
        pairBoxes.emplace_back(baseW[0], baseH[3], "TbLast16_7", "ID_TOURNAMENT_EIGHTFINAL");
        pairBoxes.emplace_back(baseW[1], baseH[3], "TbLast16_8", "ID_TOURNAMENT_EIGHTFINAL");
    }
    {
        int baseW[] = { 301, 865 };
        int baseH[] = { 338, 668 };
        pairBoxes.emplace_back(baseW[0], baseH[0], "TbQuarterFinal_1", "IDCSTR_MATCHINFO_35");
        pairBoxes.emplace_back(baseW[1], baseH[0], "TbQuarterFinal_2", "IDCSTR_MATCHINFO_35");
        pairBoxes.emplace_back(baseW[0], baseH[1], "TbQuarterFinal_3", "IDCSTR_MATCHINFO_35");
        pairBoxes.emplace_back(baseW[1], baseH[1], "TbQuarterFinal_4", "IDCSTR_MATCHINFO_35");
    }
    pairBoxes.emplace_back(442, 503, "TbSemiFinal1", "IDCSTR_MATCHINFO_36");
    pairBoxes.emplace_back(724, 503, "TbSemiFinal2", "IDCSTR_MATCHINFO_36");
    pairBoxes.emplace_back(583, 503, "TbFinal", "IDCSTR_MATCHINFO_37");
    pairBoxes.emplace_back(583, 599, "TbThirdPlace", "IDCSTR_MATCHINFO_112");

    for (auto const &p : pairBoxes) {
        f << "\t\t<Obj Rtti=\"TextBox\" Uid=\"" << p.name << "\" >" << std::endl;
        f << "\t\t\t<Meta >" << std::endl;
        f << "\t\t\t\t<Entry Key=\"Design\" Value=\"FM12 ListBox_Static_20px\" />" << std::endl;
        f << "\t\t\t</Meta>" << std::endl;
        f << "\t\t\t<Appearance Rect=\"" << p.x << "," << p.y << "," << 116 << "," << 18 << "\" />" << std::endl;
        f << "\t\t\t<ColText String=\"$" << p.text << "\" Font=\"SubHeadline\" ColorActive=\"0xfff1f1f1\" ColorInactive=\"0xfff1f1f1\" ColorHighlight=\"0xfff1f1f1\" ColorDisabled=\"0xfff1f1f1\" TextModifier=\"UpperCase\" />" << std::endl;
        f << "\t\t</Obj>" << std::endl; 
    }
    int flagId = 1;
    for (auto const &p : pairBoxes) {
        for (int i = 0; i < 2; i++) {
            f << "\t\t<Obj Rtti=\"Image\" Uid=\"TbFlag" << flagId++ << "\" >" << std::endl;
            f << "\t\t\t<Meta >" << std::endl;
            f << "\t\t\t\t<Entry Key=\"Design\" Value=\"RoundedRect\" />" << std::endl;
            f << "\t\t\t\t<Entry Key=\"Design.Control\" Value=\"Tile\" />" << std::endl;
            f << "\t\t\t</Meta>" << std::endl;
            f << "\t\t\t<Appearance Rect=\"" << p.x + ((i == 0) ? 9 : 67) << "," << p.y + 11 << "," << 38 << "," << 38 << "\" />" << std::endl;
            f << "\t\t\t<Behaviour RClick=\"true\" />" << std::endl;
            f << "\t\t\t<Texture Rect=\"0,0,8,8\" Resrc=\"dark\\art_fm\\screens\\Misc\\Empty.tga\" Color=\"0xffffffff\" ScaleMethod=\"ScaleXY\" />" << std::endl;
            f << "\t\t</Obj>" << std::endl;
        }
    }
    int tbId = 1;
    for (auto const &p : pairBoxes) {
        f << "\t\t<Obj Rtti=\"TextBox\" Uid=\"Tb" << tbId++ << "\" >" << std::endl;
        f << "\t\t\t<Meta >" << std::endl;
        f << "\t\t\t\t<Entry Key=\"Design\" Value=\"FM12 ListBox_Static_20px\" />" << std::endl;
        f << "\t\t\t</Meta>" << std::endl;
        f << "\t\t\t<Appearance Rect=\"" << p.x << "," << p.y + 44 << "," << 56 << "," << 18 << "\" />" << std::endl;
        f << "\t\t\t<ColText String=\"GER\" Font=\"SubHeadline\" ColorActive=\"0xfff1f1f1\" ColorInactive=\"0xfff1f1f1\" ColorHighlight=\"0xfff1f1f1\" ColorDisabled=\"0xff717171\" />" << std::endl;
        f << "\t\t</Obj>" << std::endl;
        f << "\t\t<Obj Rtti=\"TextBox\" Uid=\"Tb" << tbId++ << "\" >" << std::endl;
        f << "\t\t\t<Meta >" << std::endl;
        f << "\t\t\t\t<Entry Key=\"Design\" Value=\"FM12 ListBox_Static_20px\" />" << std::endl;
        f << "\t\t\t</Meta>" << std::endl;
        f << "\t\t\t<Appearance Rect=\"" << p.x - 1 << "," << p.y + 44 << "," << 116 << "," << 18 << "\" />" << std::endl;
        f << "\t\t\t<ColText String=\"$ID_TM_VS\" Font=\"SubHeadline\" ColorActive=\"0xfff1f1f1\" ColorInactive=\"0xfff1f1f1\" ColorHighlight=\"0xfff1f1f1\" ColorDisabled=\"0xff717171\" />" << std::endl;
        f << "\t\t</Obj>" << std::endl;
        f << "\t\t<Obj Rtti=\"TextBox\" Uid=\"Tb" << tbId++ << "\" >" << std::endl;
        f << "\t\t\t<Meta >" << std::endl;
        f << "\t\t\t\t<Entry Key=\"Design\" Value=\"FM12 ListBox_Static_20px\" />" << std::endl;
        f << "\t\t\t</Meta>" << std::endl;
        f << "\t\t\t<Appearance Rect=\"" << p.x + 58 << "," << p.y + 44 << "," << 56 << "," << 18 << "\" />" << std::endl;
        f << "\t\t\t<ColText String=\"GER\" Font=\"SubHeadline\" ColorActive=\"0xfff1f1f1\" ColorInactive=\"0xfff1f1f1\" ColorHighlight=\"0xfff1f1f1\" ColorDisabled=\"0xff717171\" />" << std::endl;
        f << "\t\t</Obj>" << std::endl;
        f << "\t\t<Obj Rtti=\"TextBox\" Uid=\"Tb" << tbId++ << "\" >" << std::endl;
        f << "\t\t\t<Meta >" << std::endl;
        f << "\t\t\t\t<Entry Key=\"Design\" Value=\"FM12 ListBox_Static_20px\" />" << std::endl;
        f << "\t\t\t</Meta>" << std::endl;
        f << "\t\t\t<Appearance Rect=\"" << p.x << "," << p.y + 59 << "," << 116 << "," << 18 << "\" />" << std::endl;
        f << "\t\t\t<ColText String=\"(0) 2 - 1 (1)\" Font=\"SubHeadline\" ColorActive=\"0xfff1f1f1\" ColorInactive=\"0xfff1f1f1\" ColorHighlight=\"0xfff1f1f1\" ColorDisabled=\"0xff717171\" />" << std::endl;
        f << "\t\t</Obj>" << std::endl;
    }
}

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
            const UInt SaveGameVersion = 44;
            patch::SetUInt(0x1082C02 + 3, SaveGameVersion); // new savegame version
            patch::SetUChar(0x1080E29 + 2, UChar(SaveGameVersion)); // remake the code if version >= 128 is needed

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
        PatchExcludeNames(v);
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
        PatchCustomStadiums(v);
        PatchPortraitDialog(v);
        //PatchPlayerTalks(v);
        PatchTheClubScreenExtended(v);
        PatchExtendedPlayer(v);
        PatchExtendedTeam(v);
        PatchPlayerNameEdit(v);
        PatchWomensDatabase(v);
        PatchInterfaceTheme(v);
        PatchDatabaseOptions(v);
        PatchYouthGenCountries(v);
        PatchClubSponsors(v);
        //PatchAchievements(v);
        PatchReserveNames(v);

    #ifdef BETA
        DoBetaPatches(v);
    #endif
    }

    ~UniversalConverterProject() {
        UnpatchEABFFixes();
        //if (Settings::GetInstance().EnableCustomShaders)
        //    ClearShadersDataStorage();
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
    return L"2024";
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
