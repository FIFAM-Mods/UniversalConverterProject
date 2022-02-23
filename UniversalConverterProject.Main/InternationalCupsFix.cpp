#include "InternationalCupsFix.h"
#include "GameInterfaces.h"
#include "shared.h"
#include "Utils.h"
#include "FifamBeg.h"
#include "shared.h"

using namespace plugin;

CDBLeague *gCompLeague = nullptr;
CDBRound *gCompRound = nullptr;
uintptr_t gOriginal_117C830 = 0;

Bool IsStartingSeason() {
    UShort startYear = CDBGame::GetInstance()->GetStartDate().GetYear();
    CJDate currDate = CDBGame::GetInstance()->GetCurrentDate();
    return currDate.GetYear() == startYear || (currDate.GetYear() == (startYear + 1) && currDate.GetMonth() <= 6);
}

void METHOD OnLeagueLaunch(CDBLeague *league) {
    gCompLeague = league;
    CallMethod<0x106B950>(league);
    if (IsStartingSeason()) {
        if (league->GetCompetitionType() == COMP_QUALI_WC || league->GetCompetitionType() == COMP_QUALI_EC) {
            if (league->GetCurrentMatchday() >= league->GetNumMatchdays()) {
                league->Finish();
                //Message(L"Finished %s", league->GetCompID().ToStr().c_str());
            }
        }
    }
}

void *MyFixInternationalQuali_Fix() {
    CJDate startDate = CDBGame::GetInstance()->GetStartDate();
    if ((gCompLeague->GetCompetitionType() == COMP_QUALI_WC || gCompLeague->GetCompetitionType() == COMP_QUALI_EC) && (startDate.GetYear() % 2) == 1) {
        if (IsStartingSeason())
            gCompLeague->SetStartDate(CJDate::DateFromDayOfWeek(6, 7, startDate.GetYear() - 1));
    }
    return CallAndReturnDynGlobal<void *>(gOriginal_117C830);
}

uintptr_t gOriginal_FixRound = 0;

Bool gRoundLaunchRegisterFirst = false;
Bool gRoundLaunchRegisterSecond = false;

void GenerateResult(CTeamIndex const &team1, CTeamIndex const &team2, UInt &out1, UInt &out2, Bool extraTime = false) {
    out1 = 0;
    out2 = 0;
    if (team1.countryId != 0 && team2.countryId == 0) {
        if (!extraTime)
            out1 = 3;
    }
    else if (team1.countryId == 0 && team2.countryId != 0) {
        if (!extraTime)
            out1 = 0;
    }
    else if (team1.countryId != 0 && team2.countryId != 0) {
        Int level1 = GetCountryStore()->m_aCountries[team1.countryId].GetLeagueAverageLevel();
        Int level2 = GetCountryStore()->m_aCountries[team2.countryId].GetLeagueAverageLevel();
        Int diff = level1 - level2;
        if (diff <= 22) {
            if (diff < 11) {
                if (diff < 0) {
                    if (diff < -11)
                        out1 = CRandom::GetRandomInt(2);
                    else
                        out1 = CRandom::GetRandomInt(3) + 1;
                    out2 = CRandom::GetRandomInt(3) + 1;
                }
                else {
                    out1 = CRandom::GetRandomInt(3) + 1;
                    out2 = CRandom::GetRandomInt(3);
                }
            }
            else {
                out1 = CRandom::GetRandomInt(3) + 1;
                out2 = CRandom::GetRandomInt(2);
            }
        }
        else {
            out1 = diff / 25 + CRandom::GetRandomInt(4) + 1;
            out2 = CRandom::GetRandomInt(2);
        }
        if (extraTime) {
            out1 /= 2;
            if (out1 > 2)
                out1 = 2;
            out2 /= 2;
            if (out2 > 2)
                out2 = 2;
            if (out1 == out2) {
                if (out2 == 0)
                    out1 = 1;
                else
                    out2 -= 1;
            }
        }
    }
}

void METHOD OnRoundLaunch(CDBRound *round) {
    gCompRound = round;
    gRoundLaunchRegisterFirst = true;
    gRoundLaunchRegisterSecond = true;
    CallMethod<0x10445D0>(round);
    if (!gRoundLaunchRegisterFirst) {
        //SafeLog::WriteToFile("round_results.txt", Utils::Format(L"Competition %s", round->GetCompID().ToStr()));
        UInt firstLegFlags = *raw_ptr<UInt>(round, 0x2080);
        UInt secondLegFlags = *raw_ptr<UInt>(round, 0x2084);
        RoundPair *pairs = raw_ptr<RoundPair>(round, 0x2088);
        UInt numPairs = round->GetNumOfTeams() / 2;
        for (UInt i = 0; i < numPairs; i++) {
            UInt flagsBefore = pairs[i].m_nFlags;
            Bool extraTime = false;
            UInt res1 = 0;
            UInt res2 = 0;
            UInt res2nd1 = 0;
            UInt res2nd2 = 0;
            UInt ex1 = 0;
            UInt ex2 = 0;
            GenerateResult(pairs[i].m_n1stTeam, pairs[i].m_n2ndTeam, res1, res2);
            //Message("Result: %d:%d", res1, res2);
            pairs[i].result1[0] = res1;
            pairs[i].result2[0] = res2;
            pairs[i].field_C[0] = res1 / 2;
            pairs[i].field_E[0] = res2 / 2;
            pairs[i].m_anMatchEventsStartIndex[0] = -1;
            pairs[i].m_nFlags |= FifamBeg::_1stPlayed;
            if (firstLegFlags & FifamBeg::With2ndLeg) {
                if (!gRoundLaunchRegisterSecond) {
                    GenerateResult(pairs[i].m_n1stTeam, pairs[i].m_n2ndTeam, res2nd1, res2nd2);
                    if (res1 + res2nd1 == res2 + res2nd2) {
                        Bool playExtraTime = false;
                        if (secondLegFlags & FifamBeg::WithoutAwayGoal)
                            playExtraTime = true;
                        else
                            playExtraTime = res2nd1 == res2;
                        if (playExtraTime) {
                            pairs[i].m_nFlags |= FifamBeg::ExtraTime;
                            extraTime = true;
                            if (secondLegFlags & FifamBeg::WithGoldenGoal || secondLegFlags & FifamBeg::WithSilverGoal) {
                                GenerateResult(pairs[i].m_n1stTeam, pairs[i].m_n2ndTeam, ex1, ex2);
                                if (ex2 > ex1)
                                    ex2 = 1;
                                else
                                    ex1 = 1;
                                if (secondLegFlags & FifamBeg::WithGoldenGoal)
                                    pairs[i].m_nFlags |= FifamBeg::GoldenGoal;
                                else
                                    pairs[i].m_nFlags |= FifamBeg::SilverGoal;
                            }
                            else
                                GenerateResult(pairs[i].m_n1stTeam, pairs[i].m_n2ndTeam, ex1, ex2, true);
                        }
                    }
                    pairs[i].result1[1] = res2nd1 + ex1;
                    pairs[i].result2[1] = res2nd2 + ex2;
                    pairs[i].field_C[1] = res2nd1 / 2;
                    pairs[i].field_E[1] = res2nd2 / 2;
                    pairs[i].m_anMatchEventsStartIndex[1] = -1;
                    pairs[i].m_nFlags |= FifamBeg::_2ndPlayed | FifamBeg::Finished;
                    //SafeLog::WriteToFile("round_results.txt", Utils::Format(L"%s - %s - %d:%d, %d:%d%s, flags %d->%d", TeamName(pairs[i].m_n1stTeam),
                    //    TeamName(pairs[i].m_n2ndTeam), res1, res2, res2nd1, res2nd2, extraTime ? Utils::Format(L" (ET %d:%d)", ex1, ex2) : L"", flagsBefore, pairs[i].m_nFlags));
                }
            }
            else {
                if (res1 == res2) {
                    if (firstLegFlags & FifamBeg::WithReplay) {
                        if (res2 == 0)
                            res1 = 1;
                        else
                            res2 -= 1;
                    }
                    else {
                        pairs[i].m_nFlags |= FifamBeg::ExtraTime;
                        extraTime = true;
                        if (firstLegFlags & FifamBeg::WithGoldenGoal || firstLegFlags & FifamBeg::WithSilverGoal) {
                            GenerateResult(pairs[i].m_n1stTeam, pairs[i].m_n2ndTeam, ex1, ex2);
                            if (ex2 > ex1)
                                ex2 = 1;
                            else
                                ex1 = 1;
                            if (firstLegFlags & FifamBeg::WithGoldenGoal)
                                pairs[i].m_nFlags |= FifamBeg::GoldenGoal;
                            else
                                pairs[i].m_nFlags |= FifamBeg::SilverGoal;
                        }
                        else
                            GenerateResult(pairs[i].m_n1stTeam, pairs[i].m_n2ndTeam, ex1, ex2, true);
                    }
                }
                pairs[i].result1[0] = res1 + ex1;
                pairs[i].result2[0] = res2 + ex2;
                pairs[i].field_C[0] = res1 + ex1;
                pairs[i].field_E[0] = res2 + ex2;
                pairs[i].m_nFlags |= FifamBeg::Finished;
                //SafeLog::WriteToFile("round_results.txt", Utils::Format(L"%s - %s - %d:%d%s, flags %d->%d", TeamName(pairs[i].m_n1stTeam),
                //    TeamName(pairs[i].m_n2ndTeam), res1, res2, extraTime ? Utils::Format(L" (ET %d:%d)", ex1, ex2) : L"", flagsBefore, pairs[i].m_nFlags));
            }
        }
        if (!(firstLegFlags & FifamBeg::With2ndLeg) || !gRoundLaunchRegisterSecond)
            round->Finish();
    }
}

void *MyFixInternationalQuali_FixRound() {
    CJDate startDate = CDBGame::GetInstance()->GetStartDate();
    if ((gCompRound->GetCompetitionType() == COMP_QUALI_WC || gCompRound->GetCompetitionType() == COMP_QUALI_EC) && (startDate.GetYear() % 2) == 1) {
        if (IsStartingSeason()) {
            CallMethod<0x10429E0>(gCompRound, CJDate::DateFromDayOfWeek(6, 7, startDate.GetYear() - 1));
            //CJDate *dates = raw_ptr<CJDate>(gCompRound, 0x2070);
            //for (UInt i = 0; i < gCompRound->GetNumMatchdays(); i++) {
            //    SafeLog::WriteToFile("round_dates_fix.log", Utils::Format(L"(launch) %s - %02d.%02d.%02d",
            //        gCompRound->GetCompID().ToStr(), dates[i].GetDays(), dates[i].GetMonth(), dates[i].GetYear()));
            //}
            CJDate *dates = raw_ptr<CJDate>(gCompRound, 0x2070);
            CJDate currentDate = CDBGame::GetInstance()->GetCurrentDate();
            if (currentDate.Value() > dates[0].Value()) {
                gRoundLaunchRegisterFirst = false;
                UInt firstLegFlags = *raw_ptr<UInt>(gCompRound, 0x2080);
                if (firstLegFlags & FifamBeg::With2ndLeg && currentDate.Value() > dates[1].Value())
                    gRoundLaunchRegisterSecond = false;
            }
        }
    }
    return CallAndReturnDynGlobal<void *>(gOriginal_FixRound);
}

UInt METHOD MyGetRoundMatchDate(CDBRound *comp, DUMMY_ARG, UInt matchId) {
    CJDate startDate = CDBGame::GetInstance()->GetStartDate();
    if ((comp->GetCompetitionType() == COMP_QUALI_WC || comp->GetCompetitionType() == COMP_QUALI_EC) && (startDate.GetYear() % 2) == 1) {
        if (IsStartingSeason()) {
            CallMethod<0x10429E0>(comp, CJDate::DateFromDayOfWeek(6, 7, startDate.GetYear() - 1));
            //CJDate *dates = raw_ptr<CJDate>(comp, 0x2070);
            //for (UInt i = 0; i < comp->GetNumMatchdays(); i++) {
            //    SafeLog::Write(Utils::Format(L"%s - %02d.%02d.%02d",
            //        comp->GetCompID().ToStr(), dates[i].GetDays(), dates[i].GetMonth(), dates[i].GetYear()));
            //}
        }
    }
    return CallMethodAndReturn<UInt, 0x10423F0>(comp, matchId);
}

void __declspec(naked) OnRoundLaunchCreateMatch1() {
    __asm {
        cmp gRoundLaunchRegisterFirst, 0
        je  SKIP_FIRST
        lea eax, [esp + 0x48]
        push eax
        mov eax, 0x1044BF2
        jmp eax
        SKIP_FIRST:
        mov eax, 0x1044ECC
        jmp eax
    }
}

void __declspec(naked) OnRoundLaunchCreateMatch2() {
    __asm {
        test byte ptr[esi + 0x2080], 2
        jz SKIP_SECOND
        cmp gRoundLaunchRegisterSecond, 0
        je  SKIP_SECOND
        mov eax, 0x1044EED
        jmp eax
        SKIP_SECOND:
        mov eax, 0x10451F6
        jmp eax
    }
}


void PatchInternationalCups(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        gOriginal_117C830 = patch::RedirectCall(0x105E2F4, MyFixInternationalQuali_Fix);
        gOriginal_FixRound = patch::RedirectCall(0x10446A1, MyFixInternationalQuali_FixRound);

        //patch::RedirectCall(0xF950D1, MyGetRoundMatchDate);
        //patch::RedirectCall(0xF950E5, MyGetRoundMatchDate);

        patch::SetPointer(0x24ADECC, OnLeagueLaunch);
        patch::SetPointer(0x24AD3AC, OnRoundLaunch);

        patch::RedirectJump(0x1044BED, OnRoundLaunchCreateMatch1); // jmp back to 0x1044BF2/0x1044ECC
        patch::RedirectJump(0x1044EE0, OnRoundLaunchCreateMatch2); // jmp back to 0x1044EED/0x10451F6

        // result for statistics

    }
}
