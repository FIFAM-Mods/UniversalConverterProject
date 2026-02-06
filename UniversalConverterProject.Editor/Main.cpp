#include "plugin.h"
#include "ClubIDs.h"
#include "ArtArchives.h"
#include "Talent6Stars.h"
#include "EditorChiefExecExport.h"
#include "LowestLeagues.h"
#include "StaffNames.h"
#include "Country207.h"
#include "FemaleNames.h"
#include "Competitions.h"
#include "Formations.h"
#include "Editor.h"
#include "TextFiles.h"
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
#include "Assessment.h"
#include "Resolutool.h"
#include "PlayerAppearance.h"
#include "shared.h"

using namespace plugin;

class UniversalConverterProject {
public:
    static FM::Version v;

    UniversalConverterProject() {
        auto v = FM::GetAppVersion();
        if (v.id() == ID_ED_13_1000) {
            if (!CheckPluginName(Magic<'U','n','i','v','e','r','s','a','l','C','o','n','v','e','r','t','e','r','P','r','o','j','e','c','t','.','E','d','i','t','o','r','.','a','s','i'>()))
                return;
            if (exists(FM::GameDirPath(L"plugins\\CustomTranslation.asi"))) {
                ::Error("This plugin is not compatible with CustomTranslation plugin. Please delete CustomTranslation plugin.");
            }
            Settings::GetInstance().Load();
            GenerateBigIdx();
            patch::SetUInt(0x5504D5 + 3, 0x20130011); // TODO: binary database version
            // set version (0x2013)
            patch::SetUChar(0x4C7ACA + 1, 0xF); // CClub::Write
            patch::SetUChar(0x4E4F66 + 1, 0xF); // CCountry::SaveFileData
            patch::SetUChar(0x53AE37 + 1, 0xF); // Without.sav
            patch::SetUChar(0x541565 + 1, 0xF); // Rules.sav
            // validate version (0x2013)
            patch::SetUChar(0x545493 + 8, 0xF); // 
            patch::SetUChar(0x4C527C + 1, 0xF); // CClub::Read
            patch::SetUChar(0x4C7B8C + 1, 0xF); // loading .clb file
            patch::SetUChar(0x4E9A1A + 1, 0xF); // CCountry::LoadFileData
            patch::SetUChar(0x53B5B9 + 1, 0xF); // Without.sav
            patch::SetUChar(0x541847 + 1, 0xF); // Rules.sav
            PatchRendererHook(v);
            PatchTranslation(v);
            PatchClubIDs(v);
            PatchArtArchives(v);
            //PatchTalentStars(v);
            PatchEditorChiefExecExport(v);
            PatchLowestLeagues(v);
            PatchStaffNames(v);
            PatchCountry207(v);
            //PatchFemaleNames(v);
            PatchCompetitions(v);
            PatchFormations(v);
            PatchEditor(v);
            PatchTextFiles(v);
            PatchExtendedPlayer(v);
            //PatchGenericKits(v);
            PatchKits(v);
            PatchDatabaseOptions(v);
            PatchReserveNames(v);
            PatchAssessment(v);
            PatchPlayerAppearance(v);
        }
        else if (v.id() == ID_RESOLUTOOL_13_1000) {
            PatchResolutool(v);
        }
    }

    ~UniversalConverterProject() {
        if (v.id() == ID_ED_13_1000) {
            //if (Settings::GetInstance().EnableCustomShaders)
            //    ClearShadersDataStorage();
            ClearArtArchives();
        }
    }
};

FM::Version UniversalConverterProject::v;
UniversalConverterProject universalConverterProject;
