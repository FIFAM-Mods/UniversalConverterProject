#include "GameStartTweaks.h"
#include "GameInterfaces.h"
#include "Settings.h"
#include "Random.h"

using namespace plugin;

void *METHOD OnGetPlayerContract_RandomizeTalent(CDBPlayer *player) {
    void *contract = CallMethodAndReturn<void *, 0xF98890>(player);
    if (Settings::GetInstance().getRandomizeTalents()) {
        if (player->GetAge() <= 18) {
            auto talent = player->GetTalent();
            switch (Random::Get(0, 2)) {
            case 0:
                if (talent > 0)
                    *raw_ptr<Char>(player, 0x10) = talent - 1;
                break;
            case 1:
                if (talent < 9)
                    *raw_ptr<Char>(player, 0x10) = talent + 1;
                break;
            }
        }
    }
    return contract;
}

UInt64 savedSalary = 0;

void METHOD OnClearContractAtStartOfTheSeason(void *contract) {
    if (Settings::GetInstance().getUseRealSalaries())
        savedSalary = *raw_ptr<UInt64>(contract, 0x20);
    CallMethod<0x1014E90>(contract);
}

void *METHOD OnPlayerGetConract_SalaryCalculation(CDBPlayer *player) {
    void *contract = CallMethodAndReturn<void *, 0xF98890>(player);
    if (Settings::GetInstance().getUseRealSalaries()) {
        if (savedSalary > 0) {
            CallMethod<0x10167F0>(contract, savedSalary, false);
            savedSalary = 0;
        }
    }
    return contract;
}

void *METHOD OnCreateWinterTransfersCheckbox(void *screen, DUMMY_ARG, char const *name) {
    void *originalChk = CallMethodAndReturn<void *, 0xD44260>(screen, name);
    void *randomizeTalentsCheckbox = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkRandomizeTalents");
    *raw_ptr<void *>(screen, 0xAD0) = randomizeTalentsCheckbox;
    CallVirtualMethod<84>(randomizeTalentsCheckbox, Settings::GetInstance().getRandomizeTalents());
    void *useRealSalaries = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkUseRealSalaries");
    *raw_ptr<void *>(screen, 0xAD0 + 4) = useRealSalaries;
    CallVirtualMethod<84>(useRealSalaries, Settings::GetInstance().getUseRealSalaries());
    return originalChk;
}

void METHOD OnProcessGameStartCheckboxes(void *screen, DUMMY_ARG, int *data, int unk) {
    void *randomizeTalentsCheckbox = *raw_ptr<void *>(screen, 0xAD0);
    void *useRealSalaries = *raw_ptr<void *>(screen, 0xAD0 + 4);

    if (*data == CallVirtualMethodAndReturn<int, 23>(randomizeTalentsCheckbox)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(randomizeTalentsCheckbox) != 0;
        Settings::GetInstance().setRandomizeTalents(checked);
        return;
    } else if (*data == CallVirtualMethodAndReturn<int, 23>(useRealSalaries)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(useRealSalaries) != 0;
        Settings::GetInstance().setUseRealSalaries(checked);
        return;
    }
    CallMethod<0x5247D0>(screen, data, unk);
}

void PatchGameStartTweaks(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0xFD31F6, OnGetPlayerContract_RandomizeTalent);
        patch::RedirectCall(0xF24DFC, OnClearContractAtStartOfTheSeason);
        patch::RedirectCall(0xF24E5A, OnPlayerGetConract_SalaryCalculation);
        patch::SetUInt(0x47F7A4 + 1, 0xAD0 + 8);
        patch::SetUInt(0x47F7AB + 1, 0xAD0 + 8);
        patch::RedirectCall(0x524E7A, OnCreateWinterTransfersCheckbox);
        patch::SetPointer(0x23BA8F4, OnProcessGameStartCheckboxes);
    }
}
