#include "ForeignersLimit.h"
#include "GameInterfaces.h"
#include "FifamCompID.h"
#include "shared.h"

using namespace plugin;

void __declspec(naked) CheckSpainNominationDateMonth_13() {
    __asm {
        cmp dword ptr[esp + 0x2D8], 1
        je LABEL_TRUE
        cmp dword ptr[esp + 0x2D8], 8
        je LABEL_TRUE

        mov eax, 0xF2342D
        jmp eax
    LABEL_TRUE:
        mov eax, 0xF23114
        jmp eax
    }
}

CDBTeam *gTeamNominationTeam = nullptr;
int gTeamNominationYear = 0;
int gTeamNominationMonth = 0;
int gTeamNominationDay = 0;

void METHOD OnProcessSquadNominating(CDBTeam *team, DUMMY_ARG, int year, int month, int day) {
    gTeamNominationYear = year;
    gTeamNominationMonth = month;
    gTeamNominationDay = day;
    gTeamNominationTeam = team;
    CallMethod<0xF23030>(team, year, month, day);
    gTeamNominationTeam = nullptr;
}

bool RequiresAdditionalTeamNominationForContinentalComps(CDBCompetition **outComp) {
    if (outComp)
        *outComp = nullptr;
    if (gTeamNominationTeam) {
        CJDate nominationDate;
        nominationDate.Set(gTeamNominationYear, gTeamNominationMonth, gTeamNominationDay);
        UInt nomd = nominationDate.Value();
        auto IsRoundPlayedToday = [&](CDBRound *r) {
            UInt date = CallMethodAndReturn<UInt, 0x10423F0>(r, 0);
            if (nomd == date)
                return true;
            return false;
        };
        static CCompID compIDsAry[] = {
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 1),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 2),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 3),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 4),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 5),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 6),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 7),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 8),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 9),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::ChampionsLeague, 10),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::UefaCup, 1),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::UefaCup, 2),
            CCompID::Make(FifamCompRegion::Europe, FifamCompType::UefaCup, 3)
        };
        for (auto const &id : compIDsAry) {
            CDBRound *round = GetRound(id.countryId, id.type, id.index);
            if (round && IsRoundPlayedToday(round) && round->IsTeamPresent(gTeamNominationTeam->GetTeamID())) {
                SafeLog::Write(Utils::Format(L"found team %s in round %08X", gTeamNominationTeam->GetName(), id));
                if (outComp)
                    *outComp = round;
                return true;
            }
        }
    }
    return false;
}

Bool32 RequiresTeamNominationForContinentalComps() {
    CDBCompetition *comp = nullptr;
    if (RequiresAdditionalTeamNominationForContinentalComps(&comp))
        return true;
    return (gTeamNominationMonth == 1 || gTeamNominationMonth == 8) && gTeamNominationDay == 31;
}

void __declspec(naked) TeamNominationForContinentalCompsCheck() {
    __asm {
        call RequiresTeamNominationForContinentalComps
        test eax, eax
        jz SKIP_NOMINATION
//  DO_NOMINATION:
        mov eax, 0xF23146
        jmp eax
    SKIP_NOMINATION:
        mov eax, 0xF2308B
        jmp eax
    }
}

Int METHOD GetTeamQualifiedForContinentalCompetition(CDBTeam *team, DUMMY_ARG, int unk, CCompID *outCompId) {
    CDBCompetition *comp = nullptr;
    if (RequiresAdditionalTeamNominationForContinentalComps(&comp)) {
        *outCompId = comp->GetCompID();
        return 1;
    }
    return CallMethodAndReturn<Int, 0xEFE330>(team, unk, outCompId);
}

void PatchForeignersLimit(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::SetUChar(0xF8D672, 4); // default for Turkey
        patch::Nop(0xF8D540, 2);

        // Spain players registration
        patch::RedirectJump(0xF23106, CheckSpainNominationDateMonth_13);

        // CL/EL players registration
        patch::RedirectCall(0xF35F0A, OnProcessSquadNominating);
        patch::RedirectCall(0xF3D641, OnProcessSquadNominating);

        //patch::SetUShort(0xF2313E, 0xC085);
        patch::RedirectJump(0xF23074, TeamNominationForContinentalCompsCheck);
        //patch::Nop(0xF23125, 20);

        //patch::RedirectCall(0xF23161, GetTeamQualifiedForContinentalCompetition);
        //patch::RedirectCall(0xF23268, GetTeamQualifiedForContinentalCompetition);
        //patch::RedirectCall(0xF233AE, GetTeamQualifiedForContinentalCompetition);

        patch::Nop(0xF2324E, 6);

        // (Test limits)
        //patch::SetUChar(0xF8D53B + 1, 1);
        //patch::SetUChar(0xF8D53D + 2, FifamCompRegion::Russia);
        //patch::SetUChar(0xF8D62E + 4, FifamCompRegion::Russia);
        //patch::SetUChar(0xF8D672, 4); // default for Turkey
        //patch::SetUChar(0xF8D66A, 3); // limit for Russia

        patch::RedirectCall(0x13DF7F9, (void *)0x13D2CD0); // fix the bug with line-up filler
    }
}
