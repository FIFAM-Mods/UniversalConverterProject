#include "Balls.h"
#include "Utils.h"
#include "FifamReadWrite.h"
#include "license_check/license_check.h"
#include "GameInterfaces.h"
#include "FifamCompRegion.h"
#include "FifamContinent.h"

using namespace plugin;

struct BallDesc {
    unsigned int compId = 0;
    unsigned int ballId = 0;
    unsigned int ballIdWinter = 0;
};

Vector<BallDesc> &GetBallInfos() {
    static Vector<BallDesc> ballInfos;
    return ballInfos;
}

Bool IsEuropeanCompId(UInt compId) {
    UChar compRegion = (compId >> 24) & 0xFF;
    UChar compType = (compId >> 16) & 0xFF;
    if (compRegion == FifamCompRegion::Europe) {
        if (compType != COMP_TOYOTA && compType != COMP_WORLD_CLUB_CHAMP && compType != COMP_ICC)
            return true;
    }
    else if (compRegion == FifamCompRegion::International) {
        if (compType == COMP_EURO_CUP || compType == COMP_QUALI_EC || compType == COMP_EURO_NL || compType == COMP_EURO_NL_Q)
            return true;
    }
    else {
        if (compRegion >= 1 && compRegion <= 207) {
            auto countries = GetCountryStore();
            if (countries && countries->m_aCountries[compRegion].GetContinent() == FifamContinent::Europe)
                return true;
        }
    }
    return false;
}

unsigned int GetBallModelForCurrentMatch(int) {
    if (GetBallInfos().empty())
        return 0;
    UInt defaultBallId = 0;
    UInt defaultBallIdWinter = 0;
    void *match = *(void **)0x3124748;
    UInt compId = 0;
    CallMethod<0xE80190>(match, &compId);
    Bool winterBall = false;
    if (compId != 0 && IsEuropeanCompId(compId)) {
        CDBGame *game = CDBGame::GetInstance();
        if (game) {
            auto currDate = game->GetCurrentDate();
            auto currMonth = currDate.GetMonth();
            if (currMonth == 1 || currMonth == 2 || currMonth == 11 || currMonth == 12)
                winterBall = true;
        }
    }
    if (!winterBall) {
        if (match) {
            UChar weather = CallMethodAndReturn<UChar, 0xE81160>(match);
            if (weather == 1)
                winterBall = true;
        }
    }
    for (BallDesc const &desc : GetBallInfos()) {
        if (desc.compId == 0) {
            defaultBallId = desc.ballId;
            defaultBallIdWinter = desc.ballIdWinter;
            break;
        }
    }
    UInt defaultBallForWeather = winterBall ? defaultBallIdWinter : defaultBallId;
    if (!match)
        return defaultBallForWeather;
    if (compId == 0)
        return defaultBallForWeather;
    BallDesc const *resultBall = nullptr;
    for (BallDesc const &desc : GetBallInfos()) {
        if (desc.compId <= 0xFFFF) {
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
    if (resultBall)
        return winterBall ? resultBall->ballIdWinter : resultBall->ballId;
    return defaultBallForWeather;
}

//unsigned int GetBallModelForCurrentMatch_FM11(int) {
//    if (GetBallInfos().empty())
//        return 0;
//    void *match = *(void **)0x1516C08;
//    if (!match)
//      return 0;
//    unsigned int compId = 0;
//   CallMethod<0xCDBD50>(match, &compId);
//    if (compId == 0)
//        return 0;
//    void *game = CallAndReturn<void *, 0xDDEB70>();
//    if (!game)
//        return 0;
//    unsigned char day = CallMethodAndReturn<unsigned char, 0xDCCFE0>(game);
//    unsigned char month = CallMethodAndReturn<unsigned char, 0xDCCFF0>(game);
//    BallDesc const *resultBall = nullptr;
//    BallDesc const *defaultBall = nullptr;
//    for (BallDesc const &desc : GetBallInfos()) {
//        bool dateCheck = false;
//        if (desc.startDay == 0 || desc.startMonth == 0 || desc.endDay == 0 || desc.endMonth == 0)
//            dateCheck = true;
//        if (!dateCheck) {
//            unsigned short currDate = day | (month << 8);
//            unsigned short startDate = desc.startDay | (desc.startMonth << 8);
//            unsigned short endDate = desc.endDay | (desc.endMonth << 8);
//            if (startDate <= endDate)
//                dateCheck = currDate >= startDate && currDate <= endDate;
//            else
//                dateCheck = currDate >= startDate || currDate <= endDate;
//        }
//        if (dateCheck) {
//            if (desc.compId == 0) {
//                if (!defaultBall)
//                    defaultBall = &desc;
//            }
//            else if (desc.compId <= 0xFFFF) {
//                if (!resultBall && desc.compId == (compId >> 16))
//                    resultBall = &desc;
//            }
//            else {
//                if (desc.compId == compId) {
//                    resultBall = &desc;
//                    break;
//                }
//            }
//        }
//    }
//    if (resultBall)
//        return resultBall->ballId;
//    if (defaultBall) {
//        if (!IsEuropeanCompId(compId))
//            return 0;
//        return defaultBall->ballId;
//    }
//    return 0;
//}

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
        mov     gCurrentBallId, eax
        add     esp, 4
        push    gCurrentBallId
        push    0x1264A94
        call    dword ptr[edx + 4]
        mov     eax, gCurrentBallId
        mov     dword ptr[esi + 0x20C], eax
        mov     eax, 0x42FFCF
        jmp     eax
    }
}

void ReadBallInfos() {
    FifamReader reader(Magic<'p','l','u','g','i','n','s','\\','u','c','p','\\','b','a','l','l','s','.','d','a','t'>(777301247), 14);
    if (reader.Available()) {
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                BallDesc desc;
                reader.ReadLine(Hexadecimal(desc.compId), desc.ballId, desc.ballIdWinter);
                GetBallInfos().push_back(desc);
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
    //else if (v.id() == ID_FM_11_1003) {
    //    ReadBallInfos();
    //    patch::RedirectCall(0x42FFAB, GetBallModelForCurrentMatch_FM11);
    //    patch::RedirectJump(0x42FFB8, GetBallModelExec_FM11);
    //}
}
