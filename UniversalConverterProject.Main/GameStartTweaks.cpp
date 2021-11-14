#include "GameStartTweaks.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"
#include "Random.h"
#include "FifamReadWrite.h"

using namespace plugin;

Char RandomTalent(Char talent, Int a, Int b, Int c) {
    Int rnd = Random::Get(1, a + b + c);
    if (talent == 0) {
        if (rnd > b)
            return talent + 1;
    }
    else if (talent == 9) {
        if (rnd <= a)
            return talent - 1;
    }
    else {
        if (rnd <= a)
            return  talent - 1;
        else if (rnd > (a + b))
            return  talent + 1;
    }
    return talent;
}

void *METHOD OnGetPlayerContract_RandomizeTalent(CDBPlayer *player) {
    void *contract = CallMethodAndReturn<void *, 0xF98890>(player);
    if (Settings::GetInstance().RandomizeTalents) {
        if (player->GetAge() <= 18) {
            //static FifamWriter w("test_talents.csv");
            auto talent = player->GetTalent();
            if (talent <= 0)
                talent = 0;
            else if (talent >= 9)
                talent = 9;
            if (talent == 9) // 5 - 1
                talent = RandomTalent(talent, 50, 50, 0);
            else if (talent == 8) // 4.5 - 8
                talent = RandomTalent(talent, 45, 45, 10);
            else if (talent == 7) // 4 - 26
                talent = RandomTalent(talent, 42, 42, 16);
            else if (talent == 6) // 3.5 - 123
                talent = RandomTalent(talent, 45, 45, 10);
            else if (talent == 5) // 3 - 1704
                talent = RandomTalent(talent, 48, 48, 4);
            else if (talent == 4) // 2.5 - 3976
                talent = RandomTalent(talent, 39, 39, 22);
            else if (talent == 3) // 2 - 5582
                talent = RandomTalent(talent, 23, 49, 28);
            else if (talent == 2) // 1.5 - 3776
                talent = RandomTalent(talent, 33, 33, 33);
            else if (talent == 1) // 1 - 2157
                talent = RandomTalent(talent, 33, 33, 33);
            else if (talent == 0) // 0.5 - 403
                talent = RandomTalent(talent, 33, 33, 33);
            //w.WriteLine(CallMethodAndReturn<WideChar *, 0xFA2010>(player, 0), *raw_ptr<Char>(player, 0x10), talent);
            *raw_ptr<Char>(player, 0x10) = talent;
        }
    }
    return contract;
}

UInt64 savedSalary = 0;

void METHOD OnClearContractAtStartOfTheSeason(void *contract) {
    if (Settings::GetInstance().UseRealSalaries)
        savedSalary = *raw_ptr<UInt64>(contract, 0x20);
    CallMethod<0x1014E90>(contract);
}

void *METHOD OnPlayerGetConract_SalaryCalculation(CDBPlayer *player) {
    void *contract = CallMethodAndReturn<void *, 0xF98890>(player);
    if (Settings::GetInstance().UseRealSalaries) {
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
    CallVirtualMethod<84>(randomizeTalentsCheckbox, Settings::GetInstance().RandomizeTalents);
    void *useRealSalaries = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkUseRealSalaries");
    *raw_ptr<void *>(screen, 0xAD0 + 4) = useRealSalaries;
    CallVirtualMethod<84>(useRealSalaries, Settings::GetInstance().UseRealSalaries);
    return originalChk;
}

void METHOD OnProcessGameStartCheckboxes(void *screen, DUMMY_ARG, int *data, int unk) {
    void *randomizeTalentsCheckbox = *raw_ptr<void *>(screen, 0xAD0);
    void *useRealSalaries = *raw_ptr<void *>(screen, 0xAD0 + 4);

    if (*data == CallVirtualMethodAndReturn<int, 23>(randomizeTalentsCheckbox)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(randomizeTalentsCheckbox) != 0;
        Settings::GetInstance().RandomizeTalents = checked;
        return;
    } else if (*data == CallVirtualMethodAndReturn<int, 23>(useRealSalaries)) {
        bool checked = CallVirtualMethodAndReturn<unsigned char, 85>(useRealSalaries) != 0;
        Settings::GetInstance().UseRealSalaries = checked;
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
