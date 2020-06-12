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
}
