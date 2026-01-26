#include "ForeignersLimit.h"
#include "GameInterfaces.h"
#include "FifamCompID.h"
#include "FifamNation.h"
#include "FifamContinent.h"
#include "FifamCompDbType.h"
#include "FifamClubTeamType.h"
#include "shared.h"

using namespace plugin;

void __declspec(naked) CheckSpainNominationDateMonth_13() {
    __asm {
        cmp dword ptr[esp + 0x2D8], 1
        je LABEL_TRUE
        cmp dword ptr[esp + 0x2D8], 8
        je LABEL_TRUE

        mov eax, 0xF2342D
        jmp eax
    LABEL_TRUE:
        mov eax, 0xF23114
        jmp eax
    }
}

CDBTeam *gTeamNominationTeam = nullptr;
int gTeamNominationYear = 0;
int gTeamNominationMonth = 0;
int gTeamNominationDay = 0;

void METHOD OnProcessSquadNominating(CDBTeam *team, DUMMY_ARG, int year, int month, int day) {
    gTeamNominationYear = year;
    gTeamNominationMonth = month;
    gTeamNominationDay = day;
    gTeamNominationTeam = team;
    CallMethod<0xF23030>(team, year, month, day);
    gTeamNominationTeam = nullptr;
}

bool RequiresAdditionalTeamNominationForContinentalComps(CDBCompetition **outComp) {
    if (outComp)
        *outComp = nullptr;
    if (gTeamNominationTeam) {
        CJDate nominationDate;
        nominationDate.Set(gTeamNominationYear, gTeamNominationMonth, gTeamNominationDay);
        UInt nomd = nominationDate.Value();
        auto IsRoundPlayedToday = [&](CDBRound *r) {
            UInt date = CallMethodAndReturn<UInt, 0x10423F0>(r, 0);
            if (nomd == date)
                return true;
            return false;
        };
        static CCompID compIDsAry[] = {
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 1),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 2),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 3),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 4),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 5),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 6),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 7),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 8),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 9),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 10),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::UefaCup, 1),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::UefaCup, 2),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::UefaCup, 3)
        };
        for (auto const &id : compIDsAry) {
            CDBRound *round = GetRound(id.countryId, id.type, id.index);
            if (round && IsRoundPlayedToday(round) && round->IsTeamPresent(gTeamNominationTeam->GetTeamID())) {
                SafeLog::Write(Utils::Format(L"found team %s in round %08X", gTeamNominationTeam->GetName(), id));
                if (outComp)
                    *outComp = round;
                return true;
            }
        }
    }
    return false;
}

Bool32 RequiresTeamNominationForContinentalComps() {
    CDBCompetition *comp = nullptr;
    if (RequiresAdditionalTeamNominationForContinentalComps(&comp))
        return true;
    return (gTeamNominationMonth == 1 || gTeamNominationMonth == 8) && gTeamNominationDay == 31;
}

void __declspec(naked) TeamNominationForContinentalCompsCheck() {
    __asm {
        call RequiresTeamNominationForContinentalComps
        test eax, eax
        jz SKIP_NOMINATION
//  DO_NOMINATION:
        mov eax, 0xF23146
        jmp eax
    SKIP_NOMINATION:
        mov eax, 0xF2308B
        jmp eax
    }
}

Int METHOD GetTeamQualifiedForContinentalCompetition(CDBTeam *team, DUMMY_ARG, int unk, CCompID *outCompId) {
    CDBCompetition *comp = nullptr;
    if (RequiresAdditionalTeamNominationForContinentalComps(&comp)) {
        *outCompId = comp->GetCompID();
        return 1;
    }
    return CallMethodAndReturn<Int, 0xEFE330>(team, unk, outCompId);
}

Bool IsUnitedKingdom(Int countryId) {
    return countryId == FifamNation::England || countryId == FifamNation::Scotland || countryId == FifamNation::Wales || countryId == FifamNation::Northern_Ireland;
}

Bool IsEurasianCustomsUnion(Int countryId) {
    return countryId == FifamNation::Russia || countryId == FifamNation::Kazakhstan || countryId == FifamNation::Belarus || countryId == FifamNation::Armenia || countryId == FifamNation::Kyrgyzstan;
}

Bool IsEuropeanFreeTradeAssociation(Int countryId) {
    return countryId == FifamNation::Iceland || countryId == FifamNation::Liechtenstein || countryId == FifamNation::Norway || countryId == FifamNation::Switzerland;
}

Bool IsOrganizationOfTurkicStates(Int countryId) {
    return countryId == FifamNation::Azerbaijan || countryId == FifamNation::Kazakhstan || countryId == FifamNation::Kyrgyzstan || countryId == FifamNation::Turkey || countryId == FifamNation::Uzbekistan || countryId == FifamNation::Turkmenistan;
}

Bool METHOD IsForeignNationForCompetition(CDBCompetition *comp, DUMMY_ARG, Int teamCountryId, Int playerNation) {
    if (comp) {
        CCompID *rootId = raw_ptr<CCompID>(comp, 0x1C);
        auto root = GetCompetition(*rootId);
        if (root && root->GetDbType() == FifamCompDbType::Root && root->IsContinental()) {
            if (!IsEuropeanCountry(teamCountryId))
                return false;
            return IsNonEuropeanCountry(playerNation);
        }
    }
    Bool isEuTeam = IsEuropeanUnion(teamCountryId);
    if (isEuTeam && IsEuropeanUnion(playerNation))
        return false;
    if (IsUnitedKingdom(teamCountryId) && IsUnitedKingdom(playerNation))
        return false;
    if (IsEuropeanFreeTradeAssociation(teamCountryId) && IsEuropeanFreeTradeAssociation(playerNation))
        return false;
    if (IsEurasianCustomsUnion(teamCountryId) && IsEurasianCustomsUnion(playerNation))
        return false;
    switch (teamCountryId) {
    case FifamNation::Czech_Republic:
        if (IsEuropeanFreeTradeAssociation(playerNation)
            || IsOrganizationOfTurkicStates(playerNation)
            || IsEurasianCustomsUnion(playerNation)
            || playerNation == FifamNation::Albania
            || playerNation == FifamNation::FYR_Macedonia
            || playerNation == FifamNation::Montenegro
            || playerNation == FifamNation::Serbia
            || playerNation == FifamNation::Turkey
            )
        {
            return false;
        }
        break;
    case FifamNation::Croatia:
    case FifamNation::Poland:
        if (IsEuropeanFreeTradeAssociation(playerNation))
            return false;
        break;
    case FifamNation::France:
        if (IsAfricanCaribbeanAndPacificGroupOfStates(playerNation)
            || IsEuropeanFreeTradeAssociation(playerNation)
            || IsOrganizationOfTurkicStates(playerNation)
            || IsEurasianCustomsUnion(playerNation)
            || playerNation == FifamNation::Albania
            || playerNation == FifamNation::Andorra
            || playerNation == FifamNation::Bosnia_Herzegovina
            || playerNation == FifamNation::FYR_Macedonia
            || playerNation == FifamNation::Georgia
            || playerNation == FifamNation::Moldova
            || playerNation == FifamNation::Montenegro
            || playerNation == FifamNation::Serbia
            || playerNation == FifamNation::Ukraine
            || playerNation == FifamNation::Algeria
            || playerNation == FifamNation::Morocco
            || playerNation == FifamNation::Tunisia
            || (GetCountryStore()->m_aCountries[playerNation].GetContinent() == FifamContinent::Oceania && playerNation != FifamNation::New_Zealand)
            )
        {
            return false;
        }
        break;
    case FifamNation::Germany:
        if (IsEuropeanFreeTradeAssociation(playerNation)
            || playerNation == FifamNation::Andorra
            || playerNation == FifamNation::Israel
            || playerNation == FifamNation::San_Marino
            || playerNation == FifamNation::Canada
            || playerNation == FifamNation::United_States
            || playerNation == FifamNation::Australia
            || playerNation == FifamNation::Japan
            || playerNation == FifamNation::Korea_Republic
            || playerNation == FifamNation::New_Zealand
            )
        {
            return false;
        }
        break;
    case FifamNation::Greece:
        if (IsEuropeanFreeTradeAssociation(playerNation)
            || playerNation == FifamNation::Albania
            || playerNation == FifamNation::Bosnia_Herzegovina
            || playerNation == FifamNation::Russia
            || playerNation == FifamNation::Serbia
            || playerNation == FifamNation::Turkey
            || playerNation == FifamNation::Algeria
            || playerNation == FifamNation::Morocco
            || playerNation == FifamNation::Tunisia
            )
        {
            return false;
        }
        break;
    case FifamNation::Hungary:
    case FifamNation::Romania:
        if (IsAfricanCaribbeanAndPacificGroupOfStates(playerNation)
            || IsEuropeanFreeTradeAssociation(playerNation))
        {
            return false;
        }
        break;
    case FifamNation::Spain:
        if (IsAfricanCaribbeanAndPacificGroupOfStates(playerNation)
            || IsEuropeanFreeTradeAssociation(playerNation)
            || IsOrganizationOfTurkicStates(playerNation)
            || playerNation == FifamNation::Albania
            || playerNation == FifamNation::Andorra
            || playerNation == FifamNation::Armenia
            || playerNation == FifamNation::Bosnia_Herzegovina
            || playerNation == FifamNation::FYR_Macedonia
            || playerNation == FifamNation::Georgia
            || playerNation == FifamNation::Moldova
            || playerNation == FifamNation::Montenegro
            || playerNation == FifamNation::Russia
            || playerNation == FifamNation::Serbia
            || playerNation == FifamNation::Ukraine
            || playerNation == FifamNation::Algeria
            || playerNation == FifamNation::Morocco
            || playerNation == FifamNation::Tunisia
            )
        {
            return false;
        }
        break;
    case FifamNation::Switzerland:
        if (IsEuropeanUnion(playerNation) || IsEuropeanFreeTradeAssociation(playerNation))
            return false;
        break;
    case FifamNation::Italy:
        if (IsEuropeanUnion(playerNation) || IsEuropeanFreeTradeAssociation(playerNation))
            return false;
        break;
    case FifamNation::Japan:
        if (IsAfricanCaribbeanAndPacificGroupOfStates(playerNation)
            || IsEuropeanFreeTradeAssociation(playerNation)
            || IsOrganizationOfTurkicStates(playerNation)
            || playerNation == FifamNation::Vietnam
            || playerNation == FifamNation::Thailand
            || playerNation == FifamNation::Myanmar
            || playerNation == FifamNation::Cambodia
            || playerNation == FifamNation::Singapore
            || playerNation == FifamNation::Indonesia
            || playerNation == FifamNation::Malaysia
            || playerNation == FifamNation::Qatar
            || playerNation == FifamNation::Morocco
            )
        {
            return false;
        }
        break;
    }
    if (isEuTeam)
        return true;
    return teamCountryId != playerNation;   
}

Bool METHOD OnIsPlayerNonEuropean(CDBPlayer *player) {
    CTeamIndex teamId = *raw_ptr<CTeamIndex>(player, 0xE8);
    if (teamId.countryId != 0) {
        auto team = GetTeam(teamId);
        if (team) {
            UInt teamLeagueID = *raw_ptr<UInt>(team, 0x974);
            if (teamLeagueID > 0) {
                CDBLeague *league = GetLeague(teamLeagueID);
                if (league && league->GetCompID().countryId >= 1 && league->GetCompID().countryId <= 207)
                    return IsPlayerForeignerForCompetition(player, league->GetCompID());
            }
        }
    }
    return false;
}

void METHOD TestX(void *t, DUMMY_ARG, UShort *out, UInt count) {
    CallMethod<0x1338A70>(t, out, count);
    for (UInt i = 0; i < count; i++) {
        if (out[i] != 0)
            ::Message("%d", out[i]);
    }
}

UInt METHOD OnGetIsCountryWithSimplerNaturalisationForSpain(CDBCountry *country) {
    if (country->GetCountryId() == FifamNation::Andorra
        || country->GetCountryId() == FifamNation::Portugal
        || country->GetCountryId() == FifamNation::Equatorial_Guinea
        || country->GetCountryId() == FifamNation::Philippines
        )
    {
        return CONTINENT_SOUTH_AMERICA;
    }
    return country->GetContinent();
}

void METHOD OnConstructLeagueDetails(void *t, DUMMY_ARG, Int a) {
    CallMethod<0xD527C0>(t, a);
    CallMethod<0xD1AC00>(raw_ptr<void>(t, 0x548));
}

void METHOD OnDestructLeagueDetails(void *t) {
    CallMethod<0xD182F0>(raw_ptr<void>(t, 0x548));
    CallMethod<0xD54220>(t);
}

void METHOD OnLeagueDetailsCreateUI(void *t) {
    CallMethod<0xD4F110>(t);
    void *lb = raw_ptr<void>(t, 0x548);
    CallMethod<0xD1EEE0>(lb, t, "LbCountries");
    Call<0xD19660>(lb, 4, 4, 63);
    Call<0xD196A0>(lb, 210, 204, 228);
}

CDBLeague *OnGetLeagueDetailsLeague(CCompID const *compId) {
    void *lb = raw_ptr<void>(compId, 0x548 - 0x48C);
    CDBLeague *league = GetLeague(*compId);
    if (false)
        CallVirtualMethod<11>(lb, false);
    else {
        CallVirtualMethod<11>(lb, true);
        CallMethod<0xD1AF40>(lb);
        //CallMethod<0xD186C0>(lb, 0);
        //CallMethod<0xD18510>(lb, 4, 0);
        for (UInt i = 1; i <= 207; i++) {
            if (!IsForeignNationForCompetition(league, 0, compId->countryId, i)) {
                CTeamIndex teamId = CTeamIndex::make(i, 0, 0xFFFF);
                CallMethod<0xD1E620>(lb, &teamId);
                CallMethod<0xD1F060>(lb, &teamId, 0xFF1A1A1A, 0);
                CallMethod<0xD18920>(lb, 0);
            }
        }
        //CallMethod<0xD184E0>(lb);
    }
    return league;
}

UInt METHOD GetMaxForeignersForGame(CDBLeagueBase *league) {
    UInt value = (*raw_ptr<UInt>(league, 0x2120) >> 16) & 0x1F;
    if (value == 0 || value > 18)
        return 18;
    else
        return value - 1;
}

UInt METHOD GetIsUnlimitedMaxForeignersForGame(CDBLeagueBase *league) {
    return GetMaxForeignersForGame(league) >= 18;
}

UInt METHOD GetIsNotUnlimitedMaxForeignersForGame(CDBLeagueBase *league) {
    return !GetIsUnlimitedMaxForeignersForGame(league);
}

UInt METHOD GetMaxPlayersForSeason(CDBLeagueBase *league) {
    UInt value = (*raw_ptr<UInt>(league, 0x2120) >> 21) & 0x1F;
    if (value == 0 || value > 28)
        return 99;
    else
        return value + 17;
}

UInt METHOD GetIsUnlimitedMaxPlayersForSeason(CDBLeagueBase *league) {
    return GetMaxPlayersForSeason(league) >= 99;
}

UInt METHOD GetIsNotUnlimitedMaxPlayersForSeason(CDBLeagueBase *league) {
    return !GetIsUnlimitedMaxPlayersForSeason(league);
}

UInt METHOD GetMaxForeignersForSeason(CDBLeagueBase *league) {
    UInt value = (*raw_ptr<UInt>(league, 0x2120) >> 26) & 0x1F;
    if (value == 0 || value > 30)
        return 99;
    else
        return value - 1;
}

UInt METHOD GetIsUnlimitedMaxForeignersForSeason(CDBLeagueBase *league) {
    return GetMaxForeignersForSeason(league) >= 99;
}

UInt METHOD GetIsNotUnlimitedMaxForeignersForSeason(CDBLeagueBase *league) {
    return !GetIsUnlimitedMaxForeignersForSeason(league);
}

UInt METHOD LeagueDetailsGetNoOfNonEUPlayers(CDBCompetition *comp) {
    UInt value = CallMethodAndReturn<UInt, 0xF81970>(comp);
    return (value == 11) ? 0 : value;
}

void OnLeagueDeatilsCopyNonEuUnlimited(WideChar const *dst, WideChar const *src) {
    Call<0x1493F2F>(dst, GetTranslation("IDS_DETAILS_LEAGUE_NO_MIN"));
}

UInt METHOD OnLeagueDetaulsGetNumYearsToBeNaturalized(CDBCountry *country) {
    UInt value = CallMethodAndReturn<UChar, 0xFD8870>(country);
    return value | (country->GetCountryId() << 16);
}

void *OnLeagueDetailsFormatNumYearsToBeNaturalized(void *t, UInt number, Int a, Int b) {
    UInt years = number & 0xFFFF;
    UInt countryId = (number >> 16) & 0xFFFF;
    if (countryId == FifamNation::Spain)
        CallMethod<0x14978B3>(t, FormatStatic(L"%d (%s)", years, GetTranslation("IDS_DETAILS_LEAGUE_YEARS_SPAIN")));
    else
        Call<0x14AD026>(t, years, a, b);
    return t;
}

UChar METHOD CDBCompetition_GetNoOfNonEUPlayers(CDBCompetition *comp) {
	if (comp->IsContinental())
		return 0;
	return *raw_ptr<UChar>(comp, 0x289);
}

// test

bool gLogCollector = false;

struct TeamDescForLineUp {
	CDBTeam *pTeam;
	CTeamIndex teamID;
};

WideChar const *TeamTypeStr[] = { L"First", L"Reserve", L"YouthA", L"", L"YouthB" };

void METHOD Collector_AddPlayer(void *vec, DUMMY_ARG, CDBPlayer **p) {
	CallMethod<0x13D8E80>(vec, p);
	if (gLogCollector)
		SafeLog::WriteToFile("lineup.log", Utils::Format(L"Added player: %s (team %s)",
		(*p)->GetName(), TeamTypeStr[CallMethodAndReturn<UInt, 0xFB5220>(*p)]));
}

enum CollectType { NationalTeam, Club, FullButInjured, Rotation };
WideChar const *CollectTypeStr[] = { L"NationalTeam", L"Club", L"FullButInjured", L"Rotation" };

template<UInt Orig, CollectType Type>
void METHOD Collect(void *t, DUMMY_ARG, FmVec<CDBPlayer *> *vec, TeamDescForLineUp *desc) {
	gLogCollector = desc->pTeam->GetName() == String(L"Brescia Calcio") || !desc->pTeam->IsManagedByAI();
	if (gLogCollector) {
		SafeLog::WriteToFile("lineup.log", L"____________________________________");
		SafeLog::WriteToFile("lineup.log", Utils::Format(L"Collecting %s %s", CollectTypeStr[Type], desc->pTeam->GetName()));
		SafeLog::WriteToFile("lineup.log", L"____________________________________");
	}
	CallMethod<Orig>(t, vec, desc);
	if (gLogCollector) {
		SafeLog::WriteToFile("lineup.log", L"New squad:");
		for (UInt i = 0; i < vec->size(); i++)
		    SafeLog::WriteToFile("lineup.log", Utils::Format(L"%u. %s", i + 1, vec->begin[i]->GetName()));
	}
	gLogCollector = false;
}

enum FilterType { Absent, Banned, EuropeanCup, Excluded, FirstTeam, Injured, LeaguePermission, LoanPlayers, LongtermInjury, Suspended, WorkingPermission, WrongTeam };
WideChar const *FilterTypeStr[] = { L"Absent", L"Banned", L"EuropeanCup", L"Excluded", L"FirstTeam", L"Injured", L"LeaguePermission", L"LoanPlayers", L"LongtermInjury", L"Suspended", L"WorkingPermission", L"WrongTeam" };

template<UInt Orig, FilterType Type>
Bool METHOD Filter(void *t, DUMMY_ARG, CDBPlayer **p) {
	Bool result = CallMethodAndReturn<Bool, Orig>(t, p);
	if (gLogCollector && result)
		SafeLog::WriteToFile("lineup.log", Utils::Format(L"%s: Removed player %s", FilterTypeStr[Type], (*p)->GetName()));
	return result;
}

struct MyFilterTeamForRotationCollector {
	void *vtable;
	CTeamIndex teamID;
};

Bool METHOD MyFilterTeamForRotationCollector_Filter(MyFilterTeamForRotationCollector *t, DUMMY_ARG, CDBPlayer **p) {
	Bool result = false;
	if (t->teamID.type == FifamClubTeamType::YouthA || t->teamID.type == FifamClubTeamType::YouthB)
		result = CallMethodAndReturn<Bool, 0x1427C10>(t, p); // LineUpAI::PlayerFilters::FilterWrongTeam::Filter()
	else
		result = CallMethodAndReturn<Bool, 0x1427C70>(t, p); // LineUpAI::PlayerFilters::FilterFirstTeam::Filter()
	//if (gLogCollector && result)
	//	SafeLog::WriteToFile("lineup.log", Utils::Format(L"%s: Removed player %s", L"MyFilterTeamForRotationCollector", (*p)->GetName()));
	return result;
}

Char const * METHOD MyFilterTeamForRotationCollector_GetName(MyFilterTeamForRotationCollector *t) {
	return "MyFilterTeamForRotationCollector";
}

void PatchForeignersLimit(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        //patch::RedirectCall(0xF3377D, TestX);

        patch::SetUChar(0xF8D672, 4); // default for Turkey
        patch::Nop(0xF8D540, 2);

        // Spain players registration
        patch::RedirectJump(0xF23106, CheckSpainNominationDateMonth_13);

        // CL/EL players registration
        patch::RedirectCall(0xF35F0A, OnProcessSquadNominating);
        patch::RedirectCall(0xF3D641, OnProcessSquadNominating);

        //patch::SetUShort(0xF2313E, 0xC085);
        patch::RedirectJump(0xF23074, TeamNominationForContinentalCompsCheck);
        //patch::Nop(0xF23125, 20);

        //patch::RedirectCall(0xF23161, GetTeamQualifiedForContinentalCompetition);
        //patch::RedirectCall(0xF23268, GetTeamQualifiedForContinentalCompetition);
        //patch::RedirectCall(0xF233AE, GetTeamQualifiedForContinentalCompetition);

        patch::Nop(0xF2324E, 6);

        // (Test limits)
        //patch::SetUChar(0xF8D53B + 1, 1);
        //patch::SetUChar(0xF8D53D + 2, FifamCompRegion::Russia);
        //patch::SetUChar(0xF8D62E + 4, FifamCompRegion::Russia);
        //patch::SetUChar(0xF8D672, 4); // default for Turkey
        //patch::SetUChar(0xF8D66A, 3); // limit for Russia

        //patch::RedirectCall(0x13DF7F9, (void *)0x13D2CD0); // fix the bug with line-up filler; THIS cause problem with Spain 3 non-eu limit

        patch::RedirectJump(0xF8D550, IsForeignNationForCompetition);
        patch::SetUChar(0xFBB342, 0xEB);
        patch::RedirectCall(0xFBB316, OnIsPlayerNonEuropean);

        patch::RedirectCall(0xFB44DC, OnGetIsCountryWithSimplerNaturalisationForSpain);

        // league details screen
        patch::SetUInt(0x673F44 + 1, 0x548 + 0x704);
        patch::SetUInt(0x673F4B + 1, 0x548 + 0x704);
        patch::RedirectCall(0x5EF356, OnConstructLeagueDetails);
        patch::RedirectJump(0x5EF1CC, OnDestructLeagueDetails);
        patch::RedirectCall(0x5EDDD3, OnLeagueDetailsCreateUI);
        patch::RedirectCall(0x5EE1B7, OnGetLeagueDetailsLeague);

        patch::RedirectJump(0x10CB940, GetMaxForeignersForGame);
        patch::RedirectJump(0x10CB970, GetMaxPlayersForSeason);
        patch::RedirectJump(0x10CB9A0, GetMaxForeignersForSeason);
        patch::RedirectCall(0x5EE444, GetIsNotUnlimitedMaxForeignersForGame);
        patch::RedirectCall(0x5EE4D0, GetIsNotUnlimitedMaxPlayersForSeason);
        patch::RedirectCall(0x5EE557, GetIsNotUnlimitedMaxForeignersForSeason);
        patch::RedirectCall(0x5EE3EA, (void *)0xF81970);
        patch::RedirectCall(0x5EE3F5, LeagueDetailsGetNoOfNonEUPlayers);
        patch::RedirectCall(0x5EE422, OnLeagueDeatilsCopyNonEuUnlimited);
        patch::RedirectCall(0x5EE61A, OnLeagueDetaulsGetNumYearsToBeNaturalized);
        patch::Nop(0x5EE61F, 3);
        patch::RedirectCall(0x5EE62A, OnLeagueDetailsFormatNumYearsToBeNaturalized);
        patch::SetUChar(0xF8D4D1 + 2, 11);
        patch::SetUChar(0x13D2D4D + 1, 11);
        patch::SetUChar(0x13D2DF2 + 2, 11);

		patch::SetUChar(0x11F2D5C + 1, 0); // SetupNoOfNonEUPlayers

		// Fix: Rotation line-up collector didn't use youth players in first team
		static void *MyFilterTeamForRotationCollector_Vtable[] = {
			&MyFilterTeamForRotationCollector_Filter,
			&MyFilterTeamForRotationCollector_GetName
		};
		patch::SetPointer(0x13D9848 + 4, MyFilterTeamForRotationCollector_Vtable);
    }
}
