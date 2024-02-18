#include "PlayerMatchEvents.h"
#include "FifamReadWrite.h"

//#define DEBUG_PLAYER_MATCH_EVENTS

using namespace plugin;

enum ePlayerId {
    PLAYERID_NONE = 0x0,
    PLAYERID_WITH_FOUL = 0x1,
    PLAYERID_FOULED = 0x2,
    PLAYERID_WITH_FOUL_OPPONENT = 0x3,
    PLAYERID_FOULED_OPPONENT = 0x4,
    PLAYERID_DISTANCE = 0x5,
    PLAYERID_HEADER = 0x6,
    PLAYERID_HEADER_SET_PIECE = 0x7,
    PLAYERID_HEADER_DEFENDING = 0x8,
    PLAYERID_DRIBBLING = 0x9,
    PLAYERID_BOX = 0xA,
    PLAYERID_THROUGH_PASS = 0xB,
    PLAYERID_ATTACKER = 0xC,
    PLAYERID_ATTACKER_2 = 0xD,
    PLAYERID_ATTACKING_MIDFIELDER = 0xE,
    PLAYERID_BUILD_UP_MIDFIELDER = 0xF,
    PLAYERID_DEFENDING_MIDFIELDER = 0x10,
    PLAYERID_LEFT_ATTACKER = 0x11,
    PLAYERID_RIGHT_ATTACKER = 0x12,
    PLAYERID_WING_ATTACKER = 0x13,
    PLAYERID_LEFT_BUILD_UP_ATTACKER = 0x14,
    PLAYERID_RIGHT_BUILD_UP_ATTACKER = 0x15,
    PLAYERID_LEFT_MIDFIELDER = 0x16,
    PLAYERID_RIGHT_MIDFIELDER = 0x17,
    PLAYERID_WING_MIDFIELDER = 0x18,
    PLAYERID_CENTRAL_DEFENDER = 0x19,
    PLAYERID_AGILE_DEFENDER = 0x1A,
    PLAYERID_ATTACKING_DEFENDER = 0x1B,
    PLAYERID_LEFT_DEFENDER = 0x1C,
    PLAYERID_RIGHT_DEFENDER = 0x1D,
    PLAYERID_WING_DEFENDER = 0x1E,
    PLAYERID_REPLACED_1 = 0x1F,
    PLAYERID_REPLACED_2 = 0x20,
    PLAYERID_RETURNS_1 = 0x21,
    PLAYERID_RETURNS_2 = 0x22,
    PLAYERID_OLD_CLUB = 0x23,
    PLAYERID_NEW_1 = 0x24,
    PLAYERID_NEW_2 = 0x25,
    PLAYERID_DIRECT_FREE_KICK = 0x26,
    PLAYERID_FREE_KICK_LEFT = 0x27,
    PLAYERID_FREE_KICK_RIGHT = 0x28,
    PLAYERID_CROSSES_LEFT = 0x29,
    PLAYERID_CROSSES_RIGHT = 0x2A,
    PLAYERID_PLAYS_FOR_TIME = 0x2B,
    PLAYERID_COMPLAINING = 0x2C,
    PLAYERID_BEST = 0x2D,
    PLAYERID_TECHNICAL_PROBLEMS = 0x2E,
    PLAYERID_WEAKEST = 0x2F,
    PLAYERID_LEADER = 0x30,
    PLAYERID_DIVING = 0x31,
    PLAYERID_WARNED = 0x32,
    PLAYERID_FIRST_GOAL = 0x33,
    PLAYERID_LAST_GOAL = 0x34,
    PLAYERID_ATTACKER_SPEED = 0x35,
    PLAYERID_ATTACKER_TECHNIQUE = 0x36,
    PLAYERID_BALL_WINNER = 0x37,
    PLAYERID_GOALKEEPER = 0x38,
    PLAYERID_CORNER_LEFT = 0x39,
    PLAYERID_CORNER_RIGHT = 0x3A,
    PLAYERID_GOALKEEPER_ATTACKING = 0x3B,
    PLAYERID_SUPERSTAR = 0x3C,
    PLAYERID_BUILD_UP_ATTACKER = 0x3D,
    PLAYERID_INJURED = 0x3E,
    PLAYERID_PENALTY_TAKER = 0x3F,
    PLAYERID_SUBBED_IN = 0x40,
    PLAYERID_SUBBED_OUT = 0x41,
    PLAYERID_YELLOW = 0x42,
    PLAYERID_RED_CARD_1 = 0x43,
    PLAYERID_YELLOW_RED_CARD_1 = 0x44,
    PLAYERID_LONG_INJURED_FIRST_ELEVEN = 0x45,
    PLAYERID_LONG_INJURED_BENCH = 0x46,
    PLAYERID_PREINJURED = 0x47,
    PLAYERID_SELF_PREINJURED = 0x48,
    PLAYERID_AGAINST_OLD_CLUB = 0x49,
    PLAYERID_WORST = 0x4A,
    PLAYERID_LEFT_ATTACKER_OPPONENT = 0x4B,
    PLAYERID_RIGHT_DEFENDER_OPPONENT = 0x4C,
    PLAYERID_HEADER_OPPONENT = 0x4D
};

enum ePlayerPos {
    POS_NONE = 0x0,
    POS_GK = 0x1,
    POS_RB = 0x2,
    POS_LB = 0x3,
    POS_CB = 0x4,
    POS_DM = 0x5,
    POS_RM = 0x6,
    POS_LM = 0x7,
    POS_CM = 0x8,
    POS_RW = 0x9,
    POS_LW = 0xA,
    POS_AM = 0xB,
    POS_CF = 0xC,
    POS_ST = 0xD
};

struct EventData {
    unsigned int posn[14];
    int flag;
};

static_assert(sizeof(EventData) == 15 * 4, "Wrong size");

void METHOD ModifyChanceDistributionWithPlayerLevel(unsigned int dst, int, unsigned int type) {
    unsigned int v2 = 100;
    if (!type || type == 1) {
        unsigned int level = (unsigned int)*(unsigned char *)(*(unsigned int *)(dst + 760) + 144);
        v2 = (level * level) / 50 + 200;
    }
    *(unsigned int *)(dst + 772) = (signed int)(v2 * *(unsigned int *)(dst + 772)) / 100;
}

void GeneratePlayerMatchEventsFile(String const &fileName) {
    static char const *eventNames[] = {
        "NONE",
        "WITH_FOUL",
        "FOULED",
        "WITH_FOUL_OPPONENT",
        "FOULED_OPPONENT",
        "DISTANCE",
        "HEADER",
        "HEADER_SET_PIECE",
        "HEADER_DEFENDING",
        "DRIBBLING",
        "BOX",
        "THROUGH_PASS",
        "ATTACKER",
        "ATTACKER_2",
        "ATTACKING_MIDFIELDER",
        "BUILD_UP_MIDFIELDER",
        "DEFENDING_MIDFIELDER",
        "LEFT_ATTACKER",
        "RIGHT_ATTACKER",
        "WING_ATTACKER",
        "LEFT_BUILD_UP_ATTACKER",
        "RIGHT_BUILD_UP_ATTACKER",
        "LEFT_MIDFIELDER",
        "RIGHT_MIDFIELDER",
        "WING_MIDFIELDER",
        "CENTRAL_DEFENDER",
        "AGILE_DEFENDER",
        "ATTACKING_DEFENDER",
        "LEFT_DEFENDER",
        "RIGHT_DEFENDER",
        "WING_DEFENDER",
        "REPLACED_1",
        "REPLACED_2",
        "RETURNS_1",
        "RETURNS_2",
        "OLD_CLUB",
        "NEW_1",
        "NEW_2",
        "DIRECT_FREE_KICK",
        "FREE_KICK_LEFT",
        "FREE_KICK_RIGHT",
        "CROSSES_LEFT",
        "CROSSES_RIGHT",
        "PLAYS_FOR_TIME",
        "COMPLAINING",
        "BEST",
        "TECHNICAL_PROBLEMS",
        "WEAKEST",
        "LEADER",
        "DIVING",
        "WARNED",
        "FIRST_GOAL",
        "LAST_GOAL",
        "ATTACKER_SPEED",
        "ATTACKER_TECHNIQUE",
        "BALL_WINNER",
        "GOALKEEPER",
        "CORNER_LEFT",
        "CORNER_RIGHT",
        "GOALKEEPER_ATTACKING",
        "SUPERSTAR",
        "BUILD_UP_ATTACKER",
        "INJURED",
        "PENALTY_TAKER",
        "SUBBED_IN",
        "SUBBED_OUT",
        "YELLOW",
        "RED_CARD_1",
        "YELLOW_RED_CARD_1",
        "LONG_INJURED_FIRST_ELEVEN",
        "LONG_INJURED_BENCH",
        "PREINJURED",
        "SELF_PREINJURED",
        "AGAINST_OLD_CLUB",
        "WORST",
        "LEFT_ATTACKER_OPPONENT",
        "RIGHT_DEFENDER_OPPONENT",
        "HEADER_OPPONENT"
    };
    EventData *events = (EventData *)0x24DFD10;
    FifamWriter w(fileName, 13, FifamVersion(), false);
    w.WriteLine("//                        NO, GK, RB, LB, CD, DM, RM, LM, CM, RW, LW, AM, CF, ST, Flag");
    for (UInt e = 0; e < 78; e++) {
        w.WriteLine(Format("%-25s %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %d",
            eventNames[e], events[e].posn[0], events[e].posn[1], events[e].posn[2], events[e].posn[3], events[e].posn[4],
            events[e].posn[5], events[e].posn[6], events[e].posn[7], events[e].posn[8], events[e].posn[9], events[e].posn[10],
            events[e].posn[11], events[e].posn[12], events[e].posn[13], events[e].flag));
    }
}

void PatchPlayerMatchEvents(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
#ifdef DEBUG_PLAYER_MATCH_EVENTS
        GeneratePlayerMatchEventsFile(L"Textmode Events Matrix - Original.txt");
#endif
        patch::RedirectJump(0x13B0BE0, ModifyChanceDistributionWithPlayerLevel);

        DWORD oldProtect;
        VirtualProtect((void *)0x24DFD10, 78 * 15 * 4, PAGE_READWRITE, &oldProtect);
        EventData *events = (EventData *)0x24DFD10;

        events[PLAYERID_DISTANCE].posn[POS_LW] = 90;
        events[PLAYERID_DISTANCE].posn[POS_RW] = 90;

        events[PLAYERID_HEADER].posn[POS_LW] = 60;
        events[PLAYERID_HEADER].posn[POS_RW] = 60;

        events[PLAYERID_HEADER_SET_PIECE].posn[POS_LW] = 50;
        events[PLAYERID_HEADER_SET_PIECE].posn[POS_RW] = 50;

        events[PLAYERID_DRIBBLING].posn[POS_LW] = 80;
        events[PLAYERID_DRIBBLING].posn[POS_RW] = 80;

        events[PLAYERID_BOX].posn[POS_LW] = 80;
        events[PLAYERID_BOX].posn[POS_RW] = 80;

        events[PLAYERID_ATTACKER].posn[POS_LW] = 135;
        events[PLAYERID_ATTACKER].posn[POS_RW] = 135;
        events[PLAYERID_ATTACKER].posn[POS_CF] = 135;
        events[PLAYERID_ATTACKER].posn[POS_ST] = 140;

        events[PLAYERID_ATTACKER_2].posn[POS_LW] = 135;
        events[PLAYERID_ATTACKER_2].posn[POS_RW] = 135;
        events[PLAYERID_ATTACKER_2].posn[POS_CF] = 135;
        events[PLAYERID_ATTACKER_2].posn[POS_ST] = 140;

        events[PLAYERID_ATTACKER_SPEED].posn[POS_LW] = 100;
        events[PLAYERID_ATTACKER_SPEED].posn[POS_RW] = 100;

        events[PLAYERID_ATTACKER_TECHNIQUE].posn[POS_LW] = 100;
        events[PLAYERID_ATTACKER_TECHNIQUE].posn[POS_RW] = 100;

        events[PLAYERID_DIVING].posn[POS_LW] = 90;
        events[PLAYERID_DIVING].posn[POS_RW] = 90;

        events[PLAYERID_HEADER_OPPONENT].posn[POS_LW] = 55;
        events[PLAYERID_HEADER_OPPONENT].posn[POS_RW] = 55;
#ifdef DEBUG_PLAYER_MATCH_EVENTS
        GeneratePlayerMatchEventsFile(L"Textmode Events Matrix.txt");
        Vector<UChar> matchEventsBefore(78 * 15 * 4);
        memcpy(matchEventsBefore.data(), events, matchEventsBefore.size());
#endif
        FifamReader reader(L"fmdata\\ParameterFiles\\Textmode Events Matrix.txt", 13, false, false);
        if (reader.Available()) {
            reader.SkipLine();
            for (UInt e = 0; e < 78; e++) {
                auto line = reader.ReadFullLine();
                swscanf(line.c_str(), L"%*s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                    &events[e].posn[0],  &events[e].posn[1],  &events[e].posn[2],  &events[e].posn[3],  &events[e].posn[4],
                    &events[e].posn[5],  &events[e].posn[6],  &events[e].posn[7],  &events[e].posn[8],  &events[e].posn[9],
                    &events[e].posn[10], &events[e].posn[11], &events[e].posn[12], &events[e].posn[13], &events[e].flag);
            }
        }
#ifdef DEBUG_PLAYER_MATCH_EVENTS
        Vector<UChar> matchEventsAfter(78 * 15 * 4);
        memcpy(matchEventsAfter.data(), events, matchEventsAfter.size());
        if (memcmp(matchEventsBefore.data(), matchEventsAfter.data(), matchEventsBefore.size()))
            ::Error("matchEventsBefore and matchEventsAfter are different");
        GeneratePlayerMatchEventsFile(L"Textmode Events Matrix - After reading.txt");
#endif
    }
}
