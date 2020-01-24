#include "plugin-std.h"
#include "license_check/license_check.h"
#include "ClubIDs.h"
#include "FixedArchivesReadingForEditor.h"
#include "Talent6Stars.h"
#include "EditorChiefExecExport.h"
#include "DatabaseName.h"
#include "LowestLeagues.h"
#include "StaffNames.h"
#include "Country207.h"
#include "FemaleNames.h"
#include "Competitions.h"
#include "Formations.h"
#include "Editor.h"
#include "ParameterFiles.h"
#include "Translation.h"
#include "compression.h"

using namespace plugin;

class UniversalConverterProject {
public:
    UniversalConverterProject() {
        compression_open();
        enable_compression_extended_file_size();
        auto v = FM::GetAppVersion();
        PatchTranslation(v);
        PatchDatabaseName(v);
        PatchClubIDs(v);
        PatchArchivesReadingForEditor(v);
        PatchTalentStars(v);
        PatchEditorChiefExecExport(v);
        PatchLowestLeagues(v);
        PatchStaffNames(v);
        PatchCountry207(v);
        PatchFemaleNames(v);
        PatchCompetitions(v);
        PatchFormations(v);
        PatchEditor(v);
        PatchParameterFiles(v);
    }

    ~UniversalConverterProject() {
        compression_close();
    }
} universalConverterProject;
