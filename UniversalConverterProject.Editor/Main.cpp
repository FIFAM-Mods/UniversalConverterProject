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

using namespace plugin;

class UniversalConverterProject {
public:
    UniversalConverterProject() {
        auto v = FM::GetAppVersion();
        PatchDatabaseName(v);
        PatchClubIDs(v);
        PatchArchivesReadingForEditor(v);
        PatchTalentStars(v);
        PatchEditorChiefExecExport(v);
        PatchLowestLeagues(v);
        PatchStaffNames(v);
        PatchCountry207(v);
        PatchFemaleNames(v);
    }
} universalConverterProject;
