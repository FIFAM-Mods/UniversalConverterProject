#include "ClubIDs.h"

struct ClubInfo {
    void *pClub; // club interface
    int nLeagueLevel; // league level (max 16)
    int nRating; // club rating ((teams in league - league position (starting from 0)) * 10'000 + avg11 * 100)
    int nPositionWeight; // some hardcoded value based on how team is 'important' (EA/BF: "f*ck the IDs!")
};

static int MyCompareClubs(const void *pvA, const void *pvB) {
    ClubInfo *a = (ClubInfo *)pvA;
    ClubInfo *b = (ClubInfo *)pvB;
    if (!a->pClub && !b->pClub) // if A and B are not clubs (but national team) - they are same
        return 0;
    if (!a->pClub) // if A is not a club - put it after the B
        return 1;
    if (!b->pClub) // if B is not a club - put it after the A
        return -1;
    if (a->nLeagueLevel < b->nLeagueLevel) return -1; // if A plays on higher league level - put it before the B
    if (b->nLeagueLevel < a->nLeagueLevel) return 1; // if B plays on higher league level - put it before the A
    if (a->nRating > b->nRating) return -1; // if A is higher in the table than B - put it before the B
    if (b->nRating > a->nRating) return 1; // if B is higher in the table than A - put it before the B
    return 0; // otherwise teams are same
}

static void MySortClubs(ClubInfo *begin, ClubInfo *end, size_t size, int, int, int, int) {
    qsort(begin, end - begin, sizeof(ClubInfo), MyCompareClubs);
}


void PatchClubIDs(FM::Version v) {
    if (v.id() == VERSION_ED_13) {
        patch::Nop(0x5450D1, 1); // remove the 8 prior clubs
        patch::SetUChar(0x5450D1 + 1, 0xE9); // remove the 8 prior clubs
        patch::Nop(0x54506D, 2); // save team position for league levels (saved only for level 0 originally)
        patch::RedirectCall(0x5450C6, MySortClubs); // apply our sorting algorithm instead of original
    }
}
