#include "ReserveNames.h"
#include "FifamTypes.h"
#include "Editor.h"

using namespace plugin;

const WideChar *METHOD GetReserveTeamName_Editor(void *team, DUMMY_ARG, UInt langId) {
    UInt uid = *raw_ptr<UInt>(team, 0xC);
    const WideChar *teamResName = GetText(Format("RESERVE_TEAM_%08X", uid).c_str());
    if (teamResName)
        return teamResName;
    WideChar const *teamName = raw_ptr<WideChar const>(team, 0x10 + langId * 60);
    UChar countryId = *raw_ptr<UChar>(team, 0xA);
    WideChar *out = (WideChar *)(120 * *(UInt *)0x71A070 + 0x719E90);
    *(UInt *)0x71A070 = (*(UInt *)0x71A070 + 1) % 4;
    const WideChar *countryResFormat = GetText(Format("RESERVE_COUNTRY_%u", countryId).c_str());
    if (!countryResFormat) {
        countryResFormat = GetText("RESERVE_COUNTRY_0");
        if (!countryResFormat)
            countryResFormat = L"%s Res";
    }
    swprintf(out, countryResFormat, teamName);
    return out;
}

void PatchReserveNames(FM::Version v) {
    if (v.id() == ID_ED_13_1000)
        patch::RedirectJump(0x4C4110, GetReserveTeamName_Editor);
}
