#include "InternationalCupsFix.h"
#include "GameInterfaces.h"

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

void PatchInternationalCups(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        gOriginal_StoreCompetition = patch::RedirectCall(0x105E11F, MyFixInternationalQuali_StoreCompetition);
        gOriginal_117C830 = patch::RedirectCall(0x105E2F4, MyFixInternationalQuali_Fix);
    }
}
