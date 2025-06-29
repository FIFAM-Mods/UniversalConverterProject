#include "NewspaperEuroStats.h"
#include "FifamTypes.h"
#include "FifamCompRegion.h"
#include "FifamContinent.h"
#include "GameInterfaces.h"

using namespace plugin;

const UInt NewspaperEuroStatsStructSize = 0x64C;

struct NewspaperEuroStatsExtension {
    CXgTextBox *TbTitle;
};

UShort METHOD C08NewspaperEuroStats_IsFinalRound_22(CDBRound *round) {
    return (round->GetRoundType() == ROUND_FINAL) ? 22 : 0x7FFF;
}

UChar METHOD C08NewspaperEuroStats_IsManagerRegion(CDBRound *round) {
    UInt region = FifamCompRegion::Europe;
    GetFirstManagerRegion(region);
    return (round->GetRegion() == region) ? FifamCompRegion::Europe : 0;
}

UInt C08NewspaperEuroStats_region = 0;

template<Bool Init>
void METHOD C08NewspaperEuroStats_Setup(CXgFMPanel *screen) {
    C08NewspaperEuroStats_region = FifamCompRegion::Europe;
    GetFirstManagerRegion(C08NewspaperEuroStats_region);
    if (Init) {
        NewspaperEuroStatsExtension *ext = raw_ptr<NewspaperEuroStatsExtension>(screen, NewspaperEuroStatsStructSize);
        ext->TbTitle = screen->GetTextBox("TbTitle");
        switch (C08NewspaperEuroStats_region) {
        case FifamCompRegion::Europe:
            ext->TbTitle->SetText(GetTranslation("IDS_MISC_ALLTIME_EU_CUP_STATS"));
            break;
        case FifamCompRegion::SouthAmerica:
            ext->TbTitle->SetText(GetTranslation("IDS_MISC_ALLTIME_SA_CUP_STATS"));
            break;
        case FifamCompRegion::NorthAmerica:
            ext->TbTitle->SetText(GetTranslation("IDS_MISC_ALLTIME_NA_CUP_STATS"));
            break;
        case FifamCompRegion::Africa:
            ext->TbTitle->SetText(GetTranslation("IDS_MISC_ALLTIME_AFRICA_CUP_STATS"));
            break;
        case FifamCompRegion::Asia:
            ext->TbTitle->SetText(GetTranslation("IDS_MISC_ALLTIME_ASIA_CUP_STATS"));
            break;
        case FifamCompRegion::Oceania:
            ext->TbTitle->SetText(GetTranslation("IDS_MISC_ALLTIME_OCEANIA_CUP_STATS"));
            break;
        }
    }
    CallMethod<0xA405D0>(screen);
}

Bool C08NewspaperEuroStats_IsEuropeanCountryId(UChar countryId) {
    CDBCountry *country = GetCountry(countryId);
    if (country) {
        switch (C08NewspaperEuroStats_region) {
        case FifamCompRegion::Europe:
            return country->GetContinent() == FifamContinent::Europe;
        case FifamCompRegion::SouthAmerica:
            return country->GetContinent() == FifamContinent::SouthAmerica;
        case FifamCompRegion::NorthAmerica:
            return country->GetContinent() == FifamContinent::NorthAmerica;
        case FifamCompRegion::Africa:
            return country->GetContinent() == FifamContinent::Africa;
        case FifamCompRegion::Asia:
            return country->GetContinent() == FifamContinent::Asia;
        case FifamCompRegion::Oceania:
            return country->GetContinent() == FifamContinent::Oceania;
        }
    }
    return false;
}

UInt METHOD C08NewspaperEuroStats_GetCountryAssessmentPosition(CAssessmentTable *table, DUMMY_ARG, UChar countryId) {
    if (C08NewspaperEuroStats_region == FifamCompRegion::Europe)
        return table->GetCountryPosition(countryId);
    return 99;
}

void PatchNewspaperEuroStats(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x10481CD, C08NewspaperEuroStats_IsFinalRound_22); // CDBCompetition::GetIndex
        patch::RedirectCall(0x10481DA, C08NewspaperEuroStats_IsManagerRegion); // CDBCompetition::GetRegion
        patch::RedirectCall(0xA41787, C08NewspaperEuroStats_Setup<true>);
        patch::RedirectCall(0xA417DA, C08NewspaperEuroStats_Setup<false>);
        patch::RedirectCall(0xA40645, C08NewspaperEuroStats_IsEuropeanCountryId);
        patch::RedirectCall(0xA4074E, C08NewspaperEuroStats_GetCountryAssessmentPosition);
        patch::RedirectCall(0xA4099D, C08NewspaperEuroStats_GetCountryAssessmentPosition);

        patch::SetUInt(0x49F184 + 1, NewspaperEuroStatsStructSize + sizeof(NewspaperEuroStatsExtension));
        patch::SetUInt(0x49F18B + 1, NewspaperEuroStatsStructSize + sizeof(NewspaperEuroStatsExtension));
    }
}
