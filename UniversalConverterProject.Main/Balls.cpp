#include "Balls.h"
#include "Utils.h"
#include "FifamReadWrite.h"

using namespace plugin;

struct BallDesc {
    unsigned int compId = 0;
    unsigned char startDay = 0;
    unsigned char startMonth = 0;
    unsigned char endDay = 0;
    unsigned char endMonth = 0;
    unsigned int ballId = 0;
};

Vector<BallDesc> &GetBallInfos() {
    static Vector<BallDesc> ballInfos;
    return ballInfos;
}

bool IsEuropeanCompId(unsigned int compId) {
    unsigned char compRegion = (compId >> 24) & 0xFF;
    unsigned char compType = (compId >> 16) & 0xFF;
    if (compRegion == 249) {
        if (compType != 11 && compType != 13)
            return true;
    }
    else if (compRegion == 255) {
        if (compType == 16 || compType == 18)
            return true;
    }
    else if ((compRegion >= 1 && compRegion <= 51) || (compRegion >= 205 && compRegion <= 207))
        return true;
    return false;
}

unsigned int GetBallModelForCurrentMatch(int) {
    if (GetBallInfos().empty())
        return 0;
    void *match = *(void **)0x3124748;
    if (!match)
        return 0;
    unsigned int compId = 0;
    CallMethod<0xE80190>(match, &compId);
    if (compId == 0)
        return 0;
    void *game = CallAndReturn<void *, 0xF61410>();
    if (!game)
        return 0;
    unsigned char day = CallMethodAndReturn<unsigned char, 0xF498A0>(game);
    unsigned char month = CallMethodAndReturn<unsigned char, 0xF498B0>(game);
    BallDesc const *resultBall = nullptr;
    BallDesc const *defaultBall = nullptr;
    for (BallDesc const &desc : GetBallInfos()) {
        bool dateCheck = false;
        if (desc.startDay == 0 || desc.startMonth == 0 || desc.endDay == 0 || desc.endMonth == 0)
            dateCheck = true;
        if (!dateCheck) {
            unsigned short currDate = day | (month << 8);
            unsigned short startDate = desc.startDay | (desc.startMonth << 8);
            unsigned short endDate = desc.endDay | (desc.endMonth << 8);
            if (startDate <= endDate)
                dateCheck = currDate >= startDate && currDate <= endDate;
            else
                dateCheck = currDate >= startDate || currDate <= endDate;
        }
        if (dateCheck) {
            if (desc.compId == 0) {
                if (!defaultBall)
                    defaultBall = &desc;
            }
            else if (desc.compId <= 0xFFFF) {
                if (!resultBall && desc.compId == (compId >> 16))
                    resultBall = &desc;
            }
            else {
                if (desc.compId == compId) {
                    resultBall = &desc;
                    break;
                }
            }
        }
    }
    if (resultBall)
        return resultBall->ballId;
    if (defaultBall) {
        if (!IsEuropeanCompId(compId))
            return 0;
        return defaultBall->ballId;
    }
    return 0;
}

unsigned int GetBallModelForCurrentMatch_FM11(int) {
    if (GetBallInfos().empty())
        return 0;
    void *match = *(void **)0x1516C08;
    if (!match)
        return 0;
    unsigned int compId = 0;
    CallMethod<0xCDBD50>(match, &compId);
    if (compId == 0)
        return 0;
    void *game = CallAndReturn<void *, 0xDDEB70>();
    if (!game)
        return 0;
    unsigned char day = CallMethodAndReturn<unsigned char, 0xDCCFE0>(game);
    unsigned char month = CallMethodAndReturn<unsigned char, 0xDCCFF0>(game);
    BallDesc const *resultBall = nullptr;
    BallDesc const *defaultBall = nullptr;
    for (BallDesc const &desc : GetBallInfos()) {
        bool dateCheck = false;
        if (desc.startDay == 0 || desc.startMonth == 0 || desc.endDay == 0 || desc.endMonth == 0)
            dateCheck = true;
        if (!dateCheck) {
            unsigned short currDate = day | (month << 8);
            unsigned short startDate = desc.startDay | (desc.startMonth << 8);
            unsigned short endDate = desc.endDay | (desc.endMonth << 8);
            if (startDate <= endDate)
                dateCheck = currDate >= startDate && currDate <= endDate;
            else
                dateCheck = currDate >= startDate || currDate <= endDate;
        }
        if (dateCheck) {
            if (desc.compId == 0) {
                if (!defaultBall)
                    defaultBall = &desc;
            }
            else if (desc.compId <= 0xFFFF) {
                if (!resultBall && desc.compId == (compId >> 16))
                    resultBall = &desc;
            }
            else {
                if (desc.compId == compId) {
                    resultBall = &desc;
                    break;
                }
            }
        }
    }
    if (resultBall)
        return resultBall->ballId;
    if (defaultBall) {
        if (!IsEuropeanCompId(compId))
            return 0;
        return defaultBall->ballId;
    }
    return 0;
}

	


unsigned int gCurrentBallId = 0;

void __declspec(naked) GetBallModelExec() {
    __asm {
        mov     gCurrentBallId, eax
        mov     eax, [edx + 4]
        add     esp, 4
        push    gCurrentBallId
        push    0x23AA228
        call    eax
        push    1
        mov     eax, gCurrentBallId
        mov     dword ptr[esi + 0x20C], eax
        mov     eax, 0x44E5A4
        jmp     eax
    }
}

void __declspec(naked) GetBallModelExec_FM11() {
	__asm {
		mov     gCurrentBallId, edx
		mov     edx, [ecx]
		add     esp, 4
		push    gCurrentBallId
		push    0x1264A94
		call    dword ptr[edx + 4]
		mov     eax, gCurrentBallId
		mov     dword ptr[esi + 0x20C], eax
		mov     eax, 0x42FFE8
		jmp     eax
	}
}

void ReadBallInfos() {
    FifamReader reader(L"plugins\\ucp\\balls.dat", 14);
    if (reader.Available()) {
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                BallDesc desc;
                String startDate, endDate;
                reader.ReadLine(Hexadecimal(desc.compId), startDate, endDate, desc.ballId);
                auto sd = Utils::Split(startDate, L'.');
                auto ed = Utils::Split(endDate, L'.');
                if (sd.size() == 2 && ed.size() == 2) {
                    desc.startDay = Utils::SafeConvertInt<UChar>(sd[0]);
                    desc.startMonth = Utils::SafeConvertInt<UChar>(sd[1]);
                    desc.endDay = Utils::SafeConvertInt<UChar>(ed[0]);
                    desc.endMonth = Utils::SafeConvertInt<UChar>(ed[1]);
                    if (desc.startDay <= 31 && desc.endDay <= 31 && desc.startMonth <= 12 && desc.endMonth <= 12)
                        GetBallInfos().push_back(desc);
                }
            }
            else
                reader.SkipLine();
        }
    }
}

void PatchBalls(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadBallInfos();
        patch::RedirectCall(0x44E57C, GetBallModelForCurrentMatch);
        patch::RedirectJump(0x44E589, GetBallModelExec);
    }
	else if (v.id() == ID_FM_11_1003) {
		ReadBallInfos();
		patch::RedirectCall(0x44E57C, GetBallModelForCurrentMatch_FM11);
		patch::RedirectJump(0x42FFB6, GetBallModelExec_FM11);
	}
}
