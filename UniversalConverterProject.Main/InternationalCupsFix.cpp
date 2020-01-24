#include "InternationalCupsFix.h"
#include "GameInterfaces.h"
#include "shared.h"
#include "Utils.h"

using namespace plugin;

CDBLeague *gComp = nullptr;
uintptr_t gOriginal_117C830 = 0;
uintptr_t gOriginal_StoreCompetition = 0;

unsigned char METHOD MyFixInternationalQuali_StoreCompetition(CDBLeague *comp) {
    gComp = comp;
    return CallMethodAndReturnDynGlobal<unsigned char>(gOriginal_StoreCompetition, comp);
}

void *MyFixInternationalQuali_Fix() {
    CJDate startDate = CDBGame::GetInstance()->GetStartDate();
    if ((gComp->GetCompetitionType() == COMP_QUALI_WC || gComp->GetCompetitionType() == COMP_QUALI_EC) && (startDate.GetYear() % 2) == 1) {
        CJDate currDate = CDBGame::GetInstance()->GetCurrentDate();
        if (startDate.GetYear() == currDate.GetYear()) {
            CJDate oneYearDecreased = startDate.GetTranslated(-1);
            gComp->SetStartDate(oneYearDecreased);
        }
    }
    return CallAndReturnDynGlobal<void *>(gOriginal_117C830);
}

CDBRound *gCompRound = nullptr;
uintptr_t gOriginal_FixRound = 0;
uintptr_t gOriginal_StoreRound = 0;

bool METHOD MyFixInternationalQuali_StoreRound(CDBRound *comp) {
    gCompRound = comp;
    return CallMethodAndReturnDynGlobal<bool>(gOriginal_StoreRound, comp);
}

void *MyFixInternationalQuali_FixRound() {
    CJDate startDate = CDBGame::GetInstance()->GetStartDate();
    if ((gCompRound->GetCompetitionType() == COMP_QUALI_WC || gCompRound->GetCompetitionType() == COMP_QUALI_EC) && (startDate.GetYear() % 2) == 1) {
        CJDate currDate = CDBGame::GetInstance()->GetCurrentDate();
        if (startDate.GetYear() == currDate.GetYear()) {
            CJDate oneYearDecreased = startDate.GetTranslated(-1);
            CallMethod<0x10429E0>(gCompRound, oneYearDecreased);
            //CJDate *dates = raw_ptr<CJDate>(gCompRound, 0x2070);
            //for (UInt i = 0; i < gCompRound->GetNumMatchdays(); i++) {
            //    SafeLog::Write(Utils::Format(L"(launch) %s - %02d.%02d.%02d",
            //        gCompRound->GetCompID().ToStr(), dates[i].GetDays(), dates[i].GetMonth(), dates[i].GetYear()));
            //}
        }
    }
    return CallAndReturnDynGlobal<void *>(gOriginal_FixRound);
}

UInt METHOD MyGetRoundMatchDate(CDBRound *comp, DUMMY_ARG, UInt matchId) {
    CJDate startDate = CDBGame::GetInstance()->GetStartDate();
    if ((comp->GetCompetitionType() == COMP_QUALI_WC || comp->GetCompetitionType() == COMP_QUALI_EC) && (startDate.GetYear() % 2) == 1) {
        CJDate currDate = CDBGame::GetInstance()->GetCurrentDate();
        if (startDate.GetYear() == currDate.GetYear()) {
            CJDate oneYearDecreased = startDate.GetTranslated(-1);
            CallMethod<0x10429E0>(comp, oneYearDecreased);
            //CJDate *dates = raw_ptr<CJDate>(comp, 0x2070);
            //for (UInt i = 0; i < comp->GetNumMatchdays(); i++) {
            //    SafeLog::Write(Utils::Format(L"%s - %02d.%02d.%02d",
            //        comp->GetCompID().ToStr(), dates[i].GetDays(), dates[i].GetMonth(), dates[i].GetYear()));
            //}
        }
    }
    return CallMethodAndReturn<UInt, 0x10423F0>(comp, matchId);
}

void PatchInternationalCups(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        gOriginal_StoreCompetition = patch::RedirectCall(0x105E11F, MyFixInternationalQuali_StoreCompetition);
        gOriginal_117C830 = patch::RedirectCall(0x105E2F4, MyFixInternationalQuali_Fix);

        gOriginal_StoreRound = patch::RedirectCall(0x1044612, MyFixInternationalQuali_StoreRound);
        gOriginal_FixRound = patch::RedirectCall(0x10446A1, MyFixInternationalQuali_FixRound);

        patch::RedirectCall(0xF950D1, MyGetRoundMatchDate);
        patch::RedirectCall(0xF950E5, MyGetRoundMatchDate);
    }
}
