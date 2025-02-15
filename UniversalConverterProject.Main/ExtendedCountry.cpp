#include "ExtendedCountry.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "shared.h"
#include "FifamBeg.h"

using namespace plugin;

UInt FixRankingFromOlderVersion(UInt ranking) {
	if (ranking < 2133)
		return ranking *= 15;
	return 32'000;
}

void METHOD OnReadFifaRankingFromMaster(void *reader, DUMMY_ARG, UChar *country) {
	if (BinaryReaderIsVersionGreaterOrEqual(reader, 0x2013, 0x0F)) {
		Float rankingFloat = 0.0f;
		CallMethod<0x1338BF0>(reader, &rankingFloat);
		*raw_ptr<Float>(country, 0x284) = rankingFloat;
		*raw_ptr<UShort>(country, 0x27E) = (UShort)roundf(rankingFloat);
	}
	else {
		UShort ranking = 0;
		CallMethod<0x1338970>(reader, &ranking);
		ranking = (UShort)FixRankingFromOlderVersion(ranking);
		*raw_ptr<Float>(country, 0x284) = (Float)ranking;
		*raw_ptr<UShort>(country, 0x27E) = ranking;
	}
}

bool SortCountriesByFifaRanking(CDBCountry *c1, CDBCountry *c2) {
	Float value1 = c1->GetFifaRanking();
	Float value2 = c2->GetFifaRanking();
	if (value1 > value2)
		return true;
	if (value2 > value1)
		return false;
	return CallAndReturn<Int, 0x1493FCB>(c1->GetName(), c2->GetName()) < 0;
}

bool SortCountryIDsByFifaRanking(UShort countryId1, UShort countryId2) {
	if (countryId2 == 0)
		return true;
	if (countryId1 == 0)
		return false;
	return SortCountriesByFifaRanking(GetCountry((UChar)countryId1), GetCountry((UChar)countryId2));
}

bool SortTeamsByCountryFifaRanking(CTeamIndex const &t1, CTeamIndex const &t2) {
	return SortCountryIDsByFifaRanking(t1.countryId, t2.countryId);
}

UInt METHOD GetCountryFifaRankingUInt(CDBCountry *country, DUMMY_ARG, UChar year) {
	return (UInt)roundf(*raw_ptr<Float>(country, 0x284));
}

void METHOD SetCountryFifaRanking_Dummy(CDBCountry *country, DUMMY_ARG, UShort value, UChar year) {

}

void METHOD IncreaseCountryFifaRanking_Dummy(CDBCountry *country, DUMMY_ARG, UShort points) {

}

UInt METHOD OnMonday(CJDate *date) {
	UInt day = date->GetDayOfWeek();
	if (!day) {
		CallMethod<0x1120440>(0x3185EF0, 268, 0, 0);
		CallMethod<0x111F730>(0x3185EF0);
	}
	return day;
}

Float UpdateCountryFifaRanking(CDBCountry *country, UInt I, Float W, Float dr, Bool isFinalsKO) {
	Float We = 1.0f / (powf(10.0f, -(dr / 600.0f)) + 1.0f);
	Float PointsChange = roundf((Float)I * (W - We) * 100.0f) / 100.0f;
	if (!isFinalsKO || PointsChange > 0.0f) {
		Float newRanking = country->GetFifaRanking() + PointsChange;
		if (newRanking < 0.0f) {
			PointsChange = country->GetFifaRanking();
			newRanking = 0.0f;
		}
		country->SetFifaRanking(newRanking);
		return PointsChange;
	}
	return 0.0f;
}

void ProcessMatchInternationalResult(UInt compType, CDBOneMatch *match) {
	CCompID compId = match->GetCompID();
	CDBCompetition *comp = GetCompetition(compId);
	UInt rt = ROUND_NONE;
	if (comp && comp->GetDbType() == DB_ROUND)
		rt = comp->GetRoundType();
	Int points = 0;
	Bool isFinalsKO = false;
	switch (compType) {
	case COMP_WORLD_CUP:
		if (rt == ROUND_FINAL || rt == ROUND_FINAL_3RD_PLACE || rt == ROUND_SEMIFINAL || rt == ROUND_QUARTERFINAL) {
			points = 60;
			isFinalsKO = true;
		}
		else if (rt == ROUND_LAST_16 || rt == ROUND_3) {
			points = 50;
			isFinalsKO = true;
		}
		else
			points = 50;
		break;
	case COMP_EURO_CUP:
	case COMP_NAM_CUP:
	case COMP_AFRICA_CUP:
	case COMP_ASIA_CUP:
		if (rt == ROUND_FINAL || rt == ROUND_FINAL_3RD_PLACE || rt == ROUND_SEMIFINAL || rt == ROUND_QUARTERFINAL) {
			points = 40;
			isFinalsKO = true;
		}
		else if (rt == ROUND_LAST_16) {
			points = 35;
			isFinalsKO = true;
		}
		else
			points = 35;
		break;
	case COMP_COPA_AMERICA:
		if (rt == ROUND_FINAL || rt == ROUND_FINAL_3RD_PLACE || rt == ROUND_SEMIFINAL || rt == ROUND_QUARTERFINAL) {
			points = 40;
			isFinalsKO = true;
		}
		else if (rt == ROUND_QUALI)
			points = 25;
		else
			points = 35;
		break;
	case COMP_OFC_CUP:
		if (rt == ROUND_FINAL || rt == ROUND_FINAL_3RD_PLACE || rt == ROUND_SEMIFINAL || rt == ROUND_QUARTERFINAL) {
			points = 40;
			isFinalsKO = true;
		}
		else if (comp && comp->GetDbType() == DB_LEAGUE && comp->GetRoundType() == ROUND_1)
			points = 25;
		else
			points = 35;
		break;
	case COMP_EURO_NL:
	case COMP_NAM_NL:
		isFinalsKO = true;
		points = 25;
		break;
	case COMP_QUALI_WC:
	case COMP_QUALI_EC:
	case COMP_AFRICA_CUP_Q:
	case COMP_ASIA_CUP_Q:
	case COMP_OFC_CUP_Q:
		points = 25;
		break;
	case COMP_EURO_NL_Q:
	case COMP_NAM_NL_Q:
		points = 15;
		break;
	case COMP_INTERNATIONAL_FRIENDLY:
	case COMP_FINALISSIMA:
		points = 10;
		break;
	}
	if (points != 0) {
		UChar countryId1 = match->GetHomeTeamID().countryId;
		UChar countryId2 = match->GetAwayTeamID().countryId;
		if (countryId1 != 0 && countryId2 != 0) {
			CDBCountry *country1 = GetCountry(countryId1);
			CDBCountry *country2 = GetCountry(countryId2);
			UChar scoreHome = 0, scoreAway = 0;
			match->GetResult(scoreHome, scoreAway);
			Float W_home = 0.0f, W_away = 0.0f;
			Bool penalties = match->CheckFlag(FifamBeg::Penalty);
			if (scoreHome > scoreAway) {
				if (penalties) {
					W_home = 0.75f;
					W_away = 0.5f;
				}
				else
					W_home = 1.0f;
			}
			else if (scoreAway > scoreHome) {
				if (penalties) {
					W_away = 0.75f;
					W_home = 0.5f;
				}
				else
					W_away = 1.0f;
			}
			else
				W_home = W_away = 0.5f;
			Float dr_home = country1->GetFifaRanking() - country2->GetFifaRanking();
			Float dr_away = country2->GetFifaRanking() - country1->GetFifaRanking();
			Float u1 = UpdateCountryFifaRanking(country1, points, W_home, dr_home, isFinalsKO);
			Float u2 = UpdateCountryFifaRanking(country2, points, W_away, dr_away, isFinalsKO);
			SafeLog::WriteToFile(L"fifa_rankings_log.csv",
				Utils::Format(L"%08X,%s,%s,%s,%u,%u,%u,%u,%u,%g,%g,%g,%g,%g,%g,%g,%g",
					compId.ToInt(), comp->GetName(), country1->GetName(), country2->GetName(), scoreHome, scoreAway, penalties,
					isFinalsKO, points, W_home, W_away, dr_home, dr_away, u1, u2, country1->GetFifaRanking(), country2->GetFifaRanking()),
				L"CompID,CompName,Team1,Team2,SH,SA,Pen,KO,I,WH,WA,drH,drA,upH,upA,newH,newA");
		}
	}
}

void __declspec(naked) ProcessMatchInternationalResultExe() {
	// eax - competition type, ebx - match, return address - 0xE9F2A9
	__asm {
		push ebx
		push eax
		call ProcessMatchInternationalResult
		add esp, 8
		mov ecx, 0xE9F2A9
		jmp ecx
	}
}

void FifaRanking_InitColumnTypes(void *listBox, UInt a1, UInt a2, UInt a3, UInt a4) {
	Call<0xD19660>(listBox, a1, a2, a3, 12, a4);
}

void FifaRanking_InitColumnFormatting(void *listBox, UInt a1, UInt a2, UInt a3, UInt a4) {
	Call<0xD196A0>(listBox, a1, a2, a3, 210, a4);
}

void METHOD FifaRanking_AddColumnCountryName(void *listBox, DUMMY_ARG, Int64 countryId, UInt color, UInt unk0) {
	CallMethod<0xD22BE0>(listBox, countryId, color, unk0);
	Float ranking = 0.0f;
	if (countryId != 0)
		ranking = GetCountry((UChar)countryId)->GetFifaRanking();
	CallMethod<0xD1CE20>(listBox, ranking, color, 0);
}

void METHOD FifaRanking_FillTable(void *t) {
	CallMethod<0x658090>(raw_ptr<void>(t, 0xB90)); // vector::clear
	CallMethod<0x822E70>(t);
}

void METHOD OnLoadFifaRankingFromSaveGame(void *loader, DUMMY_ARG, UShort *out) {
	CallMethod<0x10804B0>(loader, out);
	if (SaveGameLoadGetVersion(loader) < 45)
		*out = FixRankingFromOlderVersion(*out);
}

void METHOD OnLoadFifaRankingsFromSaveGame(void *loader, DUMMY_ARG, UInt *out, UInt count) {
	CallMethod<0x1080430>(loader, out, count);
	if (SaveGameLoadGetVersion(loader) < 45)
		*(Float *)out = (Float)FixRankingFromOlderVersion((UShort)out[0] + (UShort)out[1] + (UShort)out[2] + (UShort)out[3]);
	out[1] = out[2] = out[3] = 0;
}

void PatchExtendedCountry(FM::Version v) {
	if (v.id() == ID_FM_13_1030_RLD) {
		patch::SetUChar(0xFF1F46, 0x55); // push ebp
		patch::RedirectCall(0xFF1F49, OnReadFifaRankingFromMaster);
		patch::Nop(0xFF1F4E, 71);
		patch::Nop(0xFDC04A, 11);
		patch::Nop(0xFDC069, 11);
		patch::Nop(0xFDC084, 14);
		patch::RedirectJump(0xFD7F40, GetCountryFifaRankingUInt);
		patch::RedirectCall(0xF215F1, GetCountryFifaRankingUInt);
		patch::RedirectCall(0xF215FF, GetCountryFifaRankingUInt);
		patch::RedirectCall(0x1250DCF, GetCountryFifaRankingUInt); // CFanShopsAbroad
		patch::RedirectCall(0x7F651E, GetCountryFifaRankingUInt); // season quiz
		patch::RedirectCall(0x7F652A, GetCountryFifaRankingUInt); // season quiz
		patch::RedirectJump(0xFD7FC0, IncreaseCountryFifaRanking_Dummy); // CDBCountry::IncreaseFifaRanking
		patch::RedirectJump(0xFD7F80, SetCountryFifaRanking_Dummy); // CDBCountry::SetFifaRanking
		patch::RedirectJump(0xFF7900, SortTeamsByCountryFifaRanking);
		patch::RedirectJump(0xFF7860, SortCountryIDsByFifaRanking);
		patch::RedirectJump(0xE9F22D, ProcessMatchInternationalResultExe);
		patch::RedirectCall(0xFEEE99, OnLoadFifaRankingFromSaveGame);
		patch::RedirectCall(0xFEEEAD, OnLoadFifaRankingsFromSaveGame);

		// FIFA ranking screen
		patch::RedirectCall(0x822CE5, FifaRanking_InitColumnTypes);
		patch::RedirectCall(0x822CFF, FifaRanking_InitColumnFormatting);
		patch::RedirectCall(0x822FF7, FifaRanking_AddColumnCountryName);
		patch::SetPointer(0x240F594, FifaRanking_FillTable);

		// TODO: remove this
		//patch::RedirectCall(0xF538B8, OnMonday);
	}
}
