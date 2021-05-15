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

using namespace plugin;

class UniversalConverterProject {
public:
    UniversalConverterProject() {
        auto v = FM::GetAppVersion();
        if (v.id() == ID_ED_13_1000) {
            GenerateBigIdx();
            patch::SetUInt(0x5504D5 + 3, 0x2013000B); // binary database version
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
    }
} universalConverterProject;
