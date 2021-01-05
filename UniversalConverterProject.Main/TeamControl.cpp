#include "TeamControl.h"
#include "GameInterfaces.h"
#include "Settings.h"
#include "shared.h"
#include "Utils.h"
#include "FifamReadWrite.h"

using namespace plugin;

bool gHomeTeamAI = false;
bool gAwayTeamAI = false;

bool METHOD GetIsHomeTeamManagerByAI(CDBTeam *team, DUMMY_ARG, bool a) {
    void *match = *(void **)0x3124748;
    if (match) {
        CTeamIndex teamIndex;
        CallMethod<0xE7FD30>(match, &teamIndex, 1);
        if (teamIndex.type != 0)
            gHomeTeamAI = false;
        else
            gHomeTeamAI = CallMethodAndReturn<bool, 0xECA230>(team, a);
    }
    else
        gHomeTeamAI = CallMethodAndReturn<bool, 0xECA230>(team, a);
    SetVarInt("HUMAN_HOME", (Settings::GetInstance().getTeamControl() && gHomeTeamAI) ? 0 : 1);
    return gHomeTeamAI;
}

bool METHOD GetIsAwayTeamManagerByAI(CDBTeam *team, DUMMY_ARG, bool a) {
    void *match = *(void **)0x3124748;
    if (match) {
        CTeamIndex teamIndex;
        CallMethod<0xE7FD30>(match, &teamIndex, 0);
        if (teamIndex.type != 0)
            gAwayTeamAI = false;
        else
            gAwayTeamAI = CallMethodAndReturn<bool, 0xECA230>(team, a);
    }
    else
        gAwayTeamAI = CallMethodAndReturn<bool, 0xECA230>(team, a);
    SetVarInt("HUMAN_AWAY", (Settings::GetInstance().getTeamControl() && gAwayTeamAI) ? 0 : 1);
    return gAwayTeamAI;
}

void OnSetup3dMatchAudio(void *match) {
    Call<0x43F340>(match);
    bool teamControl = Settings::GetInstance().getTeamControl();
    if (teamControl && !gHomeTeamAI && gAwayTeamAI)
        SetVarString("SIDE_SELECT", "h");
    else if (teamControl && gHomeTeamAI && !gAwayTeamAI)
        SetVarString("SIDE_SELECT", "a");
    else
        SetVarString("SIDE_SELECT", "--");
    //SetVarString("SIDE_SELECT", "ha");
}

bool METHOD GetManagerShoutsStatus(void *screen) {
    if (Settings::GetInstance().getTeamControl())
        return false;
    return CallMethodAndReturn<bool, 0xC10690>(screen);
}

void ExportClubBudgets() {
    FifamWriter writer(L"ClubBudgets.csv", 14, FifamVersion());
    writer.WriteLine(L"Continent,Country,ClubId,ClubName,Cash,Salaries,SalariesLeft,Transfers,TransfersLeft,Infrstructure,InfrstructureLeft,Misc,MiscLeft,Reserve");
    auto M = [](EAGMoney const &money, UChar currency = 0) {
        return (UInt)CallMethodAndReturn<UInt64, 0x149C9D7>(&money, currency);
    };
    for (UInt i = 1; i <= 207; i++) {
        CDBCountry *country = &GetCountryStore()->m_aCountries[i];
        UInt numClubs = *raw_ptr<UInt>(country, 0x19C);
        for (UInt c = 0; c < numClubs; c++) {
            CTeamIndex teamIndex = CTeamIndex::make(i, 0, c);
            CDBTeam *club = GetTeam(teamIndex);
            if (club) {
                void *clubFinance = CallMethodAndReturn<void *, 0xED2810>(club);
                EAGMoney cash;
                CallMethod<0x10D7570>(clubFinance, &cash);
                EAGMoney budgets[5];
                CallVirtualMethod<11>(clubFinance, budgets);
                EAGMoney money[5];
                CallMethod<0x10E6BC0>(clubFinance, &money[0]);
                CallMethod<0x10E09E0>(clubFinance, &money[1]);
                CallMethod<0x10DB600>(clubFinance, &money[2]);
                CallMethod<0x10DDBC0>(clubFinance, &money[3]);
                CallVirtualMethod<53>(clubFinance, &money[4]);
                writer.WriteLine(Utils::Format(L"\"%s\",\"%s\",0x%08X,\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", country->GetContinentName(), country->GetName(), club->GetTeamUniqueID(), club->GetName(), M(cash), M(budgets[0]), M(money[0]), M(budgets[1]), M(money[1]), M(budgets[2]), M(money[2]), M(budgets[3]), M(money[3]), M(money[4])));
            }
        }
    }
}

void *METHOD OnCreateKeepOtherStadiumsCheckbox(void *screen, DUMMY_ARG, char const *name) {

    ExportClubBudgets();

    void *originalChk = CallMethodAndReturn<void *, 0xD44260>(screen, name);
    void *teamControlChk = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkEnableTeamControl");
    *raw_ptr<void *>(screen, 0xC30) = teamControlChk;
    Bool teamControlEnabled = false;
    if (Settings::GetInstance().getTeamControlDisabledAtGameStart()) {
        CallVirtualMethod<84>(teamControlChk, false);
        CallVirtualMethod<9>(teamControlChk, false);
    }
    else {
        teamControlEnabled = Settings::GetInstance().getTeamControl();
        CallVirtualMethod<84>(teamControlChk, teamControlEnabled);
    }
    SafeLog::Write(Utils::Format(L"Set : %d - %d", teamControlEnabled, CallVirtualMethodAndReturn<unsigned char, 85>(teamControlChk)));
    void *manualSwitchChk = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkManualPlayerSwitch");
    *raw_ptr<void *>(screen, 0xC34) = manualSwitchChk;
    CallVirtualMethod<84>(manualSwitchChk, Settings::GetInstance().getManualPlayerSwitch());
    CallVirtualMethod<9>(manualSwitchChk, teamControlEnabled);
    void *sponsorLogoChk = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkSponsorLogos");
    *raw_ptr<void *>(screen, 0xC38) = sponsorLogoChk;
    CallVirtualMethod<84>(sponsorLogoChk, Settings::GetInstance().getClubSponsorLogos());
    void *clubAdboardsChk = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkClubAdboards");
    *raw_ptr<void *>(screen, 0xC3C) = clubAdboardsChk;
    CallVirtualMethod<84>(clubAdboardsChk, Settings::GetInstance().getClubAdboards());
    void *new3dPitchChk = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkUseNew3dPitch");
    *raw_ptr<void *>(screen, 0xC40) = new3dPitchChk;
    CallVirtualMethod<84>(new3dPitchChk, Settings::GetInstance().getUseNew3dPitch());
    return originalChk;
}

void METHOD OnProcessOptions3DCheckboxes(void *screen, DUMMY_ARG, int *data, int unk) {
    void *teamControlChk = *raw_ptr<void *>(screen, 0xC30);
    void *manualSwitchChk = *raw_ptr<void *>(screen, 0xC34);
    void *sponsorLogoChk = *raw_ptr<void *>(screen, 0xC38);
    void *clubAdboardsChk = *raw_ptr<void *>(screen, 0xC3C);
    void *new3dPitchChk = *raw_ptr<void *>(screen, 0xC40);

    if (*data == CallVirtualMethodAndReturn<int, 23>(teamControlChk)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(teamControlChk) != 0;
        Settings::GetInstance().setTeamControl(checked);
        SafeLog::Write(Utils::Format(L"Process : %d ", checked));
        CallVirtualMethod<9>(manualSwitchChk, checked);
        return;
    }
    else if (*data == CallVirtualMethodAndReturn<int, 23>(manualSwitchChk)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(manualSwitchChk) != 0;
        Settings::GetInstance().setManualPlayerSwitch(checked);
        return;
    }
    else if (*data == CallVirtualMethodAndReturn<int, 23>(sponsorLogoChk)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(sponsorLogoChk) != 0;
        Settings::GetInstance().setClubSponsorLogos(checked);
        return;
    }
    else if (*data == CallVirtualMethodAndReturn<int, 23>(clubAdboardsChk)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(clubAdboardsChk) != 0;
        Settings::GetInstance().setClubAdboards(checked);
        return;
    }
    else if (*data == CallVirtualMethodAndReturn<int, 23>(new3dPitchChk)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(new3dPitchChk) != 0;
        Settings::GetInstance().setUseNew3dPitch(checked);
        return;
    }
    CallMethod<0x7A81E0>(screen, data, unk);
}

void METHOD OnCreateMatch3DOptionsOverlayUI(void *screen) {
    CallMethod<0xB2B0A0>(screen);
    if (Settings::GetInstance().getTeamControl()) {
        Int humanHome = gfxGetVarInt("HUMAN_HOME", 0);
        Int humanAway = gfxGetVarInt("HUMAN_AWAY", 0);
        if (humanHome != humanAway) {
            void *pBtQuickCalculation = *raw_ptr<void *>(screen, 0x5C4);
            void *pBtMatchSpeed = *raw_ptr<void *>(screen, 0x580 + 0x38);
            void *pRbDetailsFull = *raw_ptr<void *>(screen, 0x580 + 0x2C);
            void *pRbDetailsGoals = *raw_ptr<void *>(screen, 0x580 + 0x30);
            void *pRbDetailsHighlights = *raw_ptr<void *>(screen, 0x580 + 0x34);
            void *pChkx1Speed = *raw_ptr<void *>(screen, 0x5F0);
            void *pChkGoLive80 = *raw_ptr<void *>(screen, 0x5E4);

            CallVirtualMethod<11>(pBtQuickCalculation, 0);
            CallVirtualMethod<9>(pBtMatchSpeed, 0);

            //CallVirtualMethod<9>(pRbDetailsFull, 0);
            CallVirtualMethod<11>(pRbDetailsGoals, 0);
            CallVirtualMethod<11>(pRbDetailsHighlights, 0);

            CallVirtualMethod<84>(pChkx1Speed, 0);
            CallVirtualMethod<84>(pChkGoLive80, 0);
            CallVirtualMethod<9>(pChkx1Speed, 0);
            CallVirtualMethod<9>(pChkGoLive80, 0);
        }
    }
}

Array<Float, 37> gTeamControlAttributes;
Array<Float, 1001> gTeamControlTeamRating;
Array<Float, 1001> gTeamControlTeamRatingDifference;
Array<Float, 2> gTeamControlTeamSide;
Array<Float, 2> gTeamControlTeamControlSide;
CDBTeam *gTeamControlCurrentTeam = nullptr;
CTeamIndex gTeamControlCurrentTeamIndex = CTeamIndex::make(0, 0, 0);
Float gTeamControlAttrModifier = 1.0f;
Bool gTeamControlForCurrentMatch = false;

void ReadTeamControlConfig() {
    for (UInt i = 0; i < 37; i++)
        gTeamControlAttributes[i] = 0.0f;
    for (UInt i = 0; i < 1001; i++) {
        gTeamControlTeamRating[i] = 1.0f;
        gTeamControlTeamRatingDifference[i] = 1.0f;
    }
    for (UInt i = 0; i < 2; i++) {
        gTeamControlTeamSide[i] = 1.0f;
        gTeamControlTeamControlSide[i] = 1.0f;
    }
    FifamReader reader(L"plugins\\ucp\\team_control.csv", 14);
    if (reader.Available()) {
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                String arg1, arg2;
                reader.ReadLine(arg1, arg2);
                if (arg1 == L"TABLE") {
                    if (arg2 == L"ATTRIBUTES") {
                        for (UInt i = 0; i < 37; i++) {
                            String dummy;
                            Int value = 0;
                            reader.ReadLine(dummy, value);
                            gTeamControlAttributes[i] = (Float)value / 100.0f;
                        }
                    }
                    else if (arg2 == L"TEAM_RATING") {
                        Int oldRating = -1;
                        Int oldValue = 100;
                        while (!reader.IsEof()
                            && !reader.CheckLine(L"TABLE,ATTRIBUTES")
                            && !reader.CheckLine(L"TABLE,TEAM_RATING")
                            && !reader.CheckLine(L"TABLE,TEAM_RATING_DIFFERENCE")
                            && !reader.CheckLine(L"TABLE,TEAM_SIDE")
                            && !reader.CheckLine(L"TABLE,TEAM_CONTROL_SIDE"))
                        {
                            Int rating = 0;
                            Int value = 100;
                            reader.ReadLine(rating, value);
                            if (rating > oldRating && rating >= 0 && rating <= 1000) {
                                Int step = rating - oldRating;
                                Float fValue = (Float)value / 100.0f;
                                if (step > 1) {
                                    Float fOldValue = (Float)oldValue / 100.0f;
                                    Float fStep = (fValue - fOldValue) / (Float)step;
                                    for (Int i = oldRating + 1; i <= rating; i++) {
                                        fOldValue += fStep;
                                        gTeamControlTeamRating[i] = fOldValue;
                                    }
                                }
                                else
                                    gTeamControlTeamRating[rating] = fValue;
                                oldRating = rating;
                                oldValue = value;
                            }
                            else
                                break;
                        }
                    }
                    else if (arg2 == L"TEAM_RATING_DIFFERENCE") {
                        Int oldRating = -1;
                        Int oldValue = 100;
                        while (!reader.IsEof()
                            && !reader.CheckLine(L"TABLE,ATTRIBUTES")
                            && !reader.CheckLine(L"TABLE,TEAM_RATING")
                            && !reader.CheckLine(L"TABLE,TEAM_RATING_DIFFERENCE")
                            && !reader.CheckLine(L"TABLE,TEAM_SIDE")
                            && !reader.CheckLine(L"TABLE,TEAM_CONTROL_SIDE"))
                        {
                            Int rating = 0;
                            Int value = 100;
                            reader.ReadLine(rating, value);
                            if (rating > oldRating && rating >= 0 && rating <= 1000) {
                                Int step = rating - oldRating;
                                Float fValue = (Float)value / 100.0f;
                                if (step > 1) {
                                    Float fOldValue = (Float)oldValue / 100.0f;
                                    Float fStep = (fValue - fOldValue) / (Float)step;
                                    for (Int i = oldRating + 1; i <= rating; i++) {
                                        fOldValue += fStep;
                                        gTeamControlTeamRatingDifference[i] = fOldValue;
                                    }
                                }
                                else
                                    gTeamControlTeamRatingDifference[rating] = fValue;
                                oldRating = rating;
                                oldValue = value;
                            }
                            else
                                break;
                        }
                    }
                    else if (arg2 == L"TEAM_SIDE") {
                        Int value = 100;
                        reader.ReadLine(arg1, value);
                        gTeamControlTeamSide[0] = (Float)value / 100.0f;
                        value = 100;
                        reader.ReadLine(arg1, value);
                        gTeamControlTeamSide[1] = (Float)value / 100.0f;
                    }
                    else if (arg2 == L"TEAM_CONTROL_SIDE") {
                        Int value = 100;
                        reader.ReadLine(arg1, value);
                        //Error(Utils::Format(L"%d", value));
                        gTeamControlTeamControlSide[0] = (Float)value / 100.0f;
                        value = 100;
                        reader.ReadLine(arg1, value);
                        //Error(Utils::Format(L"%d", value));
                        gTeamControlTeamControlSide[1] = (Float)value / 100.0f;
                    }
                }
            }
            else
                reader.SkipLine();
        }
    }
    //for (UInt i = 0; i < 37; i++)
    //    SafeLog::Write(Utils::Format(L"ATTR_%02d: %g", i, gTeamControlAttributes[i]));
    //for (UInt i = 0; i < 1001; i++)
    //    SafeLog::Write(Utils::Format(L"RATI_%04d: %g", i, gTeamControlTeamRating[i]));
    //for (UInt i = 0; i < 1001; i++)
    //    SafeLog::Write(Utils::Format(L"RDIF_%04d: %g", i, gTeamControlTeamRatingDifference[i]));
    //for (UInt i = 0; i < 2; i++)
    //    SafeLog::Write(Utils::Format(L"SIDE_%d: %g", i, gTeamControlTeamSide[i]));
    //for (UInt i = 0; i < 2; i++)
    //    SafeLog::Write(Utils::Format(L"TCSD_%d: %g", i, gTeamControlTeamControlSide[i]));
}

CDBTeam *OnGetCurrentTeam3D(CTeamIndex teamIndex) {
    gTeamControlCurrentTeamIndex = teamIndex;
    gTeamControlCurrentTeam = GetTeam(teamIndex);
    gTeamControlAttrModifier = 1.0f;
    gTeamControlForCurrentMatch = false;
    if (Settings::GetInstance().getTeamControl()) {
        SafeLog::Write(Utils::Format(L"CurrentTeamIndex: %08X", teamIndex.ToInt()));
        if (gTeamControlCurrentTeam) {
            SafeLog::Write(Utils::Format(L"CurrentTeam: %s", gTeamControlCurrentTeam->GetName(true)));
            void *match = *(void **)0x3124748;
            if (match) {
                CTeamIndex homeTeamIndex = CTeamIndex::make(0, 0, 0);
                CallMethod<0xE7FD30>(match, &homeTeamIndex, 1);
                if (homeTeamIndex.countryId != 0) {
                    SafeLog::Write(Utils::Format(L"HomeTeamIndex: %08X", homeTeamIndex.ToInt()));
                    CTeamIndex awayTeamIndex = CTeamIndex::make(0, 0, 0);
                    CallMethod<0xE7FD30>(match, &awayTeamIndex, 0);
                    if (awayTeamIndex.countryId != 0) {
                        CDBTeam *homeTeam = GetTeam(homeTeamIndex);
                        CDBTeam *awayTeam = GetTeam(awayTeamIndex);
                        if (homeTeam && awayTeam) {
                            SafeLog::Write(Utils::Format(L"AwayTeamIndex: %08X", awayTeamIndex.ToInt()));
                            Bool homeAi = CallMethodAndReturn<Bool, 0xECA230>(homeTeam, 1);
                            Bool awayAi = CallMethodAndReturn<Bool, 0xECA230>(awayTeam, 1);
                            SafeLog::Write(Utils::Format(L"HomeAi: %d, AwayAi: %d", homeAi, awayAi));
                            if ((!homeAi || !awayAi) && homeAi != awayAi) {
                                Int teamSide = -1;
                                CTeamIndex thisTeamIndex = CTeamIndex::make(0, 0, 0);
                                CTeamIndex otherTeamIndex = CTeamIndex::make(0, 0, 0);
                                CDBTeam *thisTeam = nullptr;
                                CDBTeam *otherTeam = nullptr;
                                auto CompareTeamIDs = [](CTeamIndex const &a, CTeamIndex const &b) { return a.countryId == b.countryId && a.type == b.type && a.index == b.index; };
                                if (CompareTeamIDs(homeTeamIndex, teamIndex)) {
                                    teamSide = 0;
                                    thisTeamIndex = homeTeamIndex;
                                    otherTeamIndex = awayTeamIndex;
                                    thisTeam = homeTeam;
                                    otherTeam = awayTeam;
                                }
                                else if (CompareTeamIDs(awayTeamIndex, teamIndex)) {
                                    teamSide = 1;
                                    thisTeamIndex = awayTeamIndex;
                                    otherTeamIndex = homeTeamIndex;
                                    thisTeam = awayTeam;
                                    otherTeam = homeTeam;
                                }
                                if (teamSide != -1) {
                                    SafeLog::Write(Utils::Format(L"TeamSide: %d, ThisTeamIndex: %08X, OtherTeamIndex: %08X", teamSide, thisTeamIndex.ToInt(), otherTeamIndex.ToInt()));
                                    SafeLog::Write(Utils::Format(L"ThisTeam: %s, OtherTeam: %s", thisTeam->GetName(true), thisTeam->GetName(true)));
                                    Int thisTeamRating = CallMethodAndReturn<Int, 0xF2F290>(thisTeam, &thisTeamIndex, 1, 0);
                                    Int otherTeamRating = CallMethodAndReturn<Int, 0xF2F290>(otherTeam, &otherTeamIndex, 1, 0);
                                    if (thisTeamRating > 0 && otherTeamRating > 0) {
                                        gTeamControlForCurrentMatch = true;
                                        Int ratingDifference = thisTeamRating - otherTeamRating;
                                        SafeLog::Write(Utils::Format(L"ThisTeamRating: %d, OtherTeamRating: %d, RatingDifference: %d", thisTeamRating, otherTeamRating, ratingDifference));
                                        if (thisTeamRating > 1000)
                                            thisTeamRating = 1000;
                                        gTeamControlAttrModifier *= gTeamControlTeamRating[thisTeamRating];
                                        SafeLog::Write(Utils::Format(L"ThisTeamRating: %d, TeamControlAttrModifier: %g", thisTeamRating, gTeamControlAttrModifier));
                                        if (ratingDifference < 0) {
                                            ratingDifference = -ratingDifference;
                                            if (ratingDifference > 1000)
                                                ratingDifference = 1000;
                                            gTeamControlAttrModifier *= gTeamControlTeamRating[ratingDifference];
                                            SafeLog::Write(Utils::Format(L"RatingDifference: %d, TeamControlAttrModifier: %g", ratingDifference, gTeamControlAttrModifier));
                                        }
                                        Int teamHostSide = -1;
                                        CTeamIndex hostTeamIndex = CTeamIndex::make(0, 0, 0);
                                        CallMethod<0xE814C0>(match, &hostTeamIndex);
                                        SafeLog::Write(Utils::Format(L"HostTeamIndex: %08X", hostTeamIndex.ToInt()));
                                        if (CompareTeamIDs(homeTeamIndex, hostTeamIndex))
                                            teamHostSide = 0;
                                        else if (CompareTeamIDs(awayTeamIndex, hostTeamIndex))
                                            teamHostSide = 1;
                                        if (teamHostSide != -1) {
                                            if (teamHostSide == teamSide)
                                                gTeamControlAttrModifier *= gTeamControlTeamSide[0];
                                            else
                                                gTeamControlAttrModifier *= gTeamControlTeamSide[1];
                                            SafeLog::Write(Utils::Format(L"TeamControlAttrModifier: %g", gTeamControlAttrModifier));
                                        }
                                        if (!homeAi) {
                                            if (teamSide == 0) {
                                                gTeamControlAttrModifier *= gTeamControlTeamControlSide[0];
                                                SafeLog::Write(Utils::Format(L"TeamControlTeamControlSide: %g, TeamControlAttrModifier: %g", gTeamControlTeamControlSide[0], gTeamControlAttrModifier));
                                            }
                                            else {
                                                gTeamControlAttrModifier *= gTeamControlTeamControlSide[1];
                                                SafeLog::Write(Utils::Format(L"TeamControlTeamControlSide: %g, TeamControlAttrModifier: %g", gTeamControlTeamControlSide[1], gTeamControlAttrModifier));
                                            }
                                        }
                                        else if (!awayAi) {
                                            if (teamSide == 0) {
                                                gTeamControlAttrModifier *= gTeamControlTeamControlSide[1];
                                                SafeLog::Write(Utils::Format(L"TeamControlTeamControlSide: %g, TeamControlAttrModifier: %g", gTeamControlTeamControlSide[1], gTeamControlAttrModifier));
                                            }
                                            else {
                                                gTeamControlAttrModifier *= gTeamControlTeamControlSide[0];
                                                SafeLog::Write(Utils::Format(L"TeamControlTeamControlSide: %g, TeamControlAttrModifier: %g", gTeamControlTeamControlSide[0], gTeamControlAttrModifier));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return gTeamControlCurrentTeam;
}

UChar METHOD OnGetPlayerAttribute3D(CDBPlayer *player, DUMMY_ARG, UInt attrId, UInt unk) {
    UChar result = CallMethodAndReturn<UChar, 0xFA57D0>(player, attrId, unk);
    UChar oldValue = result;
    if (gTeamControlForCurrentMatch && gTeamControlAttrModifier != 1.0f && attrId < 37 && gTeamControlAttributes[attrId] > 0.0f) {
        Bool isZero = result == 0;
        Float diff = 1.0f - gTeamControlAttrModifier;
        diff *= gTeamControlAttributes[attrId];
        Float modifier = 1.0f - diff;
        result = Utils::Clamp(UChar((Float)result * modifier), 0, 99);
        if (result == 0 && !isZero)
            result = 1;
    }
    //SafeLog::Write(Utils::Format(L"Player: %s, AttrId: %d, OldValue: %d, NewValue: %d", CallMethodAndReturn<WideChar *, 0xFA2010>(player, 0), attrId, oldValue, result));
    return result;
}

void PatchTeamControl(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

        patch::SetUInt(0x573AA4 + 1, 0xC30 + 20);
        patch::SetUInt(0x573AAB + 1, 0xC30 + 20);
        patch::RedirectCall(0x7A8614, OnCreateKeepOtherStadiumsCheckbox);
        patch::SetPointer(0x24003BC, OnProcessOptions3DCheckboxes);

        if (!Settings::GetInstance().getTeamControlDisabledAtGameStart()) {
            ReadTeamControlConfig();

            patch::RedirectCall(0x44E5B3, GetIsHomeTeamManagerByAI);
            patch::RedirectCall(0x44E5CD, GetIsAwayTeamManagerByAI);
            patch::Nop(0x44E64B, 31);
            patch::RedirectCall(0x44E6A1, OnSetup3dMatchAudio);
            patch::RedirectCall(0xC10BDC, GetManagerShoutsStatus);
            patch::RedirectCall(0xC11119, GetManagerShoutsStatus);
            //patch::SetPointer(0x44E65A + 1, "h");

            patch::RedirectCall(0xC0110A, OnCreateMatch3DOptionsOverlayUI);

            patch::RedirectCall(0x416ABC, OnGetCurrentTeam3D);
            patch::RedirectCall(0x41403B, OnGetPlayerAttribute3D);
            patch::RedirectCall(0x414118, OnGetPlayerAttribute3D);
        }
    }
}
