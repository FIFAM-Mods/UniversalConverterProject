#include "plugin-std.h"
#include "ClubIDs.h"
#include "FixedArchivesReadingForEditor.h"
#include "Talent6Stars.h"
#include "EditorChiefExecExport.h"
#include "LowestLeagues.h"
#include "StaffNames.h"
#include "Country207.h"
#include "FemaleNames.h"
#include "Competitions.h"
#include "Formations.h"
#include "Editor.h"
#include "ParameterFiles.h"
#include "Translation.h"
#include "RendererHook.h"
#include "GenerateBigIdx.h"
#include "ExtendedPlayerEditor.h"
#include "UcpSettings.h"
#include "GenericKits.h"
#include "Kits.h"
#include "DatabaseOptions.h"
#include "CustomShaders.h"
#include "ReserveNames.h"

using namespace plugin;

bool IsWomensDatabase = false;

class UniversalConverterProject {
public:
    UniversalConverterProject() {
        auto v = FM::GetAppVersion();
        if (v.id() == ID_ED_13_1000) {
            Settings::GetInstance().Load();
            IsWomensDatabase = patch::GetUShort(0x66E7A8) == L'W';
            GenerateBigIdx();
            patch::SetUInt(0x5504D5 + 3, 0x2013000E); // binary database version
            // set version (0x2013)
            patch::SetUChar(0x4C7ACA + 1, 0xE); // CClub::Write
            patch::SetUChar(0x4E4F66 + 1, 0xE); // CCountry::SaveFileData
            patch::SetUChar(0x53AE37 + 1, 0xE); // Without.sav
            patch::SetUChar(0x541565 + 1, 0xE); // Rules.sav
            // validate version (0x2013)
            patch::SetUChar(0x545493 + 8, 0xE); // 
            patch::SetUChar(0x4C527C + 1, 0xE); // CClub::Read
            patch::SetUChar(0x4C7B8C + 1, 0xE); // loading .clb file
            patch::SetUChar(0x4E9A1A + 1, 0xE); // CCountry::LoadFileData
            patch::SetUChar(0x53B5B9 + 1, 0xE); // Without.sav
            patch::SetUChar(0x541847 + 1, 0xE); // Rules.sav
        }
        PatchRendererHook(v);
        PatchTranslation(v);
        PatchClubIDs(v);
        PatchArchivesReadingForEditor(v);
        //PatchTalentStars(v);
        PatchEditorChiefExecExport(v);
        PatchLowestLeagues(v);
        PatchStaffNames(v);
        PatchCountry207(v);
        //PatchFemaleNames(v);
        PatchCompetitions(v);
        PatchFormations(v);
        PatchEditor(v);
        PatchParameterFiles(v);
        PatchExtendedPlayer(v);
        //PatchGenericKits(v);
        PatchKits(v);
        PatchDatabaseOptions(v);
        PatchReserveNames(v);
    }

    ~UniversalConverterProject() {
        //if (Settings::GetInstance().EnableCustomShaders)
        //    ClearShadersDataStorage();
    }
} universalConverterProject;
