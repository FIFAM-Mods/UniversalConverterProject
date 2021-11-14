#include "LoansLimit.h"
#include "UcpSettings.h"

using namespace plugin;

int METHOD GetMaxLoans(int) {
    return 100;
}

void PatchLoansLimit(FM::Version v) {
    if (Settings::GetInstance().ExtendLoansLimit) {
        if (v.id() == ID_FM_13_1030_RLD) {
            patch::RedirectJump(0x10CB820, GetMaxLoans);
            patch::RedirectJump(0x10CB8B0, GetMaxLoans);
            patch::RedirectJump(0x10CB850, GetMaxLoans);
            patch::RedirectJump(0x10CB880, GetMaxLoans);
            patch::RedirectJump(0x10CB790, GetMaxLoans);
            patch::RedirectJump(0x10CB760, GetMaxLoans);
            patch::RedirectJump(0x10CB7C0, GetMaxLoans);
            patch::RedirectJump(0x10CB7F0, GetMaxLoans);
            patch::RedirectJump(0x10CB730, GetMaxLoans);
            patch::RedirectJump(0x10CB700, GetMaxLoans);
            patch::RedirectJump(0x10CB910, GetMaxLoans);
            patch::RedirectJump(0x10CB8E0, GetMaxLoans);
        }
    }
}
