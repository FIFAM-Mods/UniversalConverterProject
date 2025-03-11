#include "MiscFormerCupOpponents.h"
#include "FifamTypes.h"
#include "FifamCompRegion.h"
#include "GameInterfaces.h"
#include "UEFALeaguePhase.h"

void __declspec(naked) MiscFormerCupOpponents_CompTypeCheck() {
    __asm {
        cmp eax, 0xA // COMP_UEFA_CUP
        jz loc_4B1613
        cmp eax, 0x33 // COMP_CONFERENCE_LEAGUE
        jz loc_4B1613
        mov eax, 0x4B1322
        jmp eax
    loc_4B1613:
        mov eax, 0x4B1613
        jmp eax
    }
}

const Char *GetMiscFormerCupOpponentsScreenName(CCompID compId) {
    if (compId.type == COMP_CHAMPIONSLEAGUE || compId.type == COMP_UEFA_CUP)
        return "Screens/12MiscFormerCupOpponents2.xml";
    else if (compId.type == COMP_CONFERENCE_LEAGUE)
        return "Screens/12MiscFormerCupOpponents3.xml";
    return "Screens/12MiscFormerCupOpponents.xml";
}

Bool MiscFormerCupOpponents_SetupUEFA(void *t, CCompID compId, CTeamIndex teamID) {
    if (compId.countryId != FifamCompRegion::Europe)
        return false;
    CDBRound *round = GetRound(compId);
    if (!round)
        return false;
    CTeamIndex teams[2] = { CTeamIndex::null(), CTeamIndex::null() };
    for (UInt i = 0; i < round->GetNumOfPairs(); i++) {
        RoundPair &rp = round->GetRoundPair(i);
        if (rp.Get1stTeam() == teamID || rp.Get2ndTeam() == teamID) {
            teams[0] = rp.Get1stTeam();
            teams[1] = rp.Get2ndTeam();
            break;
        }
    }
    if (teams[0].isNull() || teams[1].isNull())
        return false;
    if (compId.type == COMP_CONFERENCE_LEAGUE)
        CallMethod<0x896950>(t, 20, 28);// MiscFormerCupOpponents::SetupTextBoxesAndFixtures
    else
        CallMethod<0x896950>(t, 24, 32);// MiscFormerCupOpponents::SetupTextBoxesAndFixtures
    CallMethod<0x896A90>(t, compId, teams[0], teams[1]); // MiscFormerCupOpponents::SetupCompAndTeamNames
    UInt numCompIds = 0;
    UInt baseCompID = compId.ToInt() & 0xFFFF0000;
    UInt *compIds = GetUEFALeaguePhaseMatchdaysCompIDs(baseCompID, numCompIds);
    Vector<UInt> leaguePhaseIDs((compId.type == COMP_CONFERENCE_LEAGUE) ? 6 : 8, 0);
    for (UInt i = 0; i < Utils::Min(leaguePhaseIDs.size(), numCompIds); i++)
        leaguePhaseIDs[i] = compIds[i];
    UInt roundTypes[] = { ROUND_2, ROUND_LAST_16, ROUND_QUARTERFINAL, ROUND_SEMIFINAL, ROUND_FINAL };
    Vector<UInt> koRoundIDs(std::size(roundTypes));;
    for (UInt i = 0; i < std::size(roundTypes); i++) {
        CDBRound *r = GetRoundByRoundType(compId.countryId, compId.type, roundTypes[i]);
        koRoundIDs[i] = (r ? r->GetCompID().ToInt() : baseCompID);
    }
    for (UInt teamIndex = 0; teamIndex < 2; teamIndex++) {
        UInt tbIndex = 0;
        UInt fixtureIndex = 0;
        for (UInt i = 0; i < leaguePhaseIDs.size(); i++) {
            CallMethod<0x8973C0>(t, teamIndex + tbIndex, teamIndex + tbIndex,
                leaguePhaseIDs[i], teams[teamIndex]); // MiscFormerCupOpponents::AddRoundFixture
            tbIndex += 2;
            fixtureIndex += 2;
        }
        for (UInt i = 0; i < koRoundIDs.size(); i++) {
            CallMethod<0x8973C0>(t, teamIndex + tbIndex, teamIndex + fixtureIndex,
                koRoundIDs[i], teams[teamIndex]); // MiscFormerCupOpponents::AddRoundFixture
            tbIndex += 2;
            fixtureIndex += 4;
        }
    }
    return true;
}

Bool METHOD MiscFormerCupOpponents_Setup(void *t, DUMMY_ARG, CCompID compId) {
    *raw_ptr<CCompID>(t, 0x48C) = compId;
    CDBEmployee *manager = CallMethodAndReturn<CDBEmployee *, 0x4316A0>((void *)0x3062D28);
    if (manager) {
        if (compId.type == COMP_FA_CUP)
            return CallMethodAndReturn<Bool, 0x896E80>(t, compId, manager->GetTeamID()); // MiscFormerCupOpponents::SetupFACup
        else if (compId.type == COMP_CHAMPIONSLEAGUE || compId.type == COMP_UEFA_CUP || compId.type == COMP_CONFERENCE_LEAGUE)
            return MiscFormerCupOpponents_SetupUEFA(t, compId, manager->GetTeamID());
    }
    return false;
}

void PatchMiscFormerCupOpponents(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectJump(0x896760, GetMiscFormerCupOpponentsScreenName);
        patch::RedirectJump(0x8979C0, MiscFormerCupOpponents_Setup);
        patch::RedirectJump(0x4B1319, MiscFormerCupOpponents_CompTypeCheck);
        //patch::SetUChar(0x896D24, 0xEB);
    }
}
