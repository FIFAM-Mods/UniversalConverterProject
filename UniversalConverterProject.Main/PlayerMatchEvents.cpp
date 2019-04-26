#include "PlayerMatchEvents.h"

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

enum ePlayerId_11 {
    PLAYERID_11_NONE = 0,
    PLAYERID_11_WITH_FOUL = 1,
    PLAYERID_11_FOULED = 2,
    PLAYERID_11_WITH_FOUL_OPPONENT = 3,
    PLAYERID_11_FOULED_OPPONENT = 4,
    PLAYERID_11_DISTANCE = 5,
    PLAYERID_11_HEADER = 6,
    PLAYERID_11_HEADER_SET_PIECE = 7,
    PLAYERID_11_HEADER_DEFENDING = 8,
    PLAYERID_11_DRIBBLING = 9,
    PLAYERID_11_BOX = 10,
    PLAYERID_11_THROUGH_PASS = 11,
    PLAYERID_11_ATTACKER = 12,
    PLAYERID_11_ATTACKER_2 = 13,
    PLAYERID_11_ATTACKING_MIDFIELDER = 14,
    PLAYERID_11_BUILD_UP_MIDFIELDER = 15,
    PLAYERID_11_DEFENDING_MIDFIELDER = 16,
    PLAYERID_11_LEFT_ATTACKER = 17,
    PLAYERID_11_RIGHT_ATTACKER = 18,
    PLAYERID_11_WING_ATTACKER = 19,
    PLAYERID_11_LEFT_BUILD_UP_ATTACKER = 20,
    PLAYERID_11_RIGHT_BUILD_UP_ATTACKER = 21,
    PLAYERID_11_LEFT_MIDFIELDER = 22,
    PLAYERID_11_RIGHT_MIDFIELDER = 23,
    PLAYERID_11_WING_MIDFIELDER = 24,
    PLAYERID_11_CENTRAL_DEFENDER = 25,
    PLAYERID_11_AGILE_DEFENDER = 26,
    PLAYERID_11_ATTACKING_DEFENDER = 27,
    PLAYERID_11_LEFT_DEFENDER = 28,
    PLAYERID_11_RIGHT_DEFENDER = 29,
    PLAYERID_11_WING_DEFENDER = 30,
    PLAYERID_11_REPLACED_1 = 31,
    PLAYERID_11_REPLACED_2 = 32,
    PLAYERID_11_RETURNS_1 = 33,
    PLAYERID_11_RETURNS_2 = 34,
    PLAYERID_11_OLD_CLUB = 35,
    PLAYERID_11_NEW_1 = 36,
    PLAYERID_11_NEW_2 = 37,
    PLAYERID_11_DIRECT_FREE_KICK = 38,
    PLAYERID_11_FREE_KICK_LEFT = 39,
    PLAYERID_11_FREE_KICK_RIGHT = 40,
    PLAYERID_11_CROSSES_LEFT = 41,
    PLAYERID_11_CROSSES_RIGHT = 42,
    PLAYERID_11_PLAYS_FOR_TIME = 43,
    PLAYERID_11_COMPLAINING = 44,
    PLAYERID_11_BEST = 45,
    PLAYERID_11_TECHNICAL_PROBLEMS = 46,
    PLAYERID_11_WEAKEST = 47,
    PLAYERID_11_LEADER = 48,
    PLAYERID_11_DIVING = 49,
    PLAYERID_11_WARNED = 50,
    PLAYERID_11_FIRST_GOAL = 51,
    PLAYERID_11_LAST_GOAL = 52,
    PLAYERID_11_ATTACKER_SPEED = 53,
    PLAYERID_11_ATTACKER_TECHNIQUE = 54,
    PLAYERID_11_BALL_WINNER = 55,
    PLAYERID_11_GOALKEEPER = 56,
    PLAYERID_11_CORNER_LEFT = 57,
    PLAYERID_11_CORNER_RIGHT = 58,
    PLAYERID_11_GOALKEEPER_ATTACKING = 59,
    PLAYERID_11_SUPERSTAR = 60,
    PLAYERID_11_BUILD_UP_ATTACKER = 61,
    PLAYERID_11_INJURED = 62,
    PLAYERID_11_PENALTY_TAKER = 63,
    PLAYERID_11_SUBBED_IN = 64,
    PLAYERID_11_SUBBED_OUT = 65,
    PLAYERID_11_YELLOW = 66,
    PLAYERID_11_RED_CARD_1 = 67,
    PLAYERID_11_YELLOW_RED_CARD_1 = 68,
    PLAYERID_11_LONG_INJURED_FIRST_ELEVEN = 69,
    PLAYERID_11_LONG_INJURED_BENCH = 70
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

enum ePlayerPos_11 {
    POS_11_NONE = 0,
    POS_11_GK = 1,
    POS_11_RB = 2,
    POS_11_LB = 3,
    POS_11_CB = 4,
    POS_11_SW = 5,
    POS_11_RWB = 6,
    POS_11_LWB = 7,
    POS_11_ANC = 8,
    POS_11_DM = 9,
    POS_11_RM = 10,
    POS_11_LM = 11,
    POS_11_CM = 12,
    POS_11_RW = 13,
    POS_11_LW = 14,
    POS_11_AM = 15,
    POS_11_CF = 16,
    POS_11_ST = 17
};

struct EventData {
    unsigned int posn[14];
    int flag;
};

struct EventData_11 {
    unsigned int posn[18];
    int flag;
};

static_assert(sizeof(EventData) == 15 * 4, "Wrong size");
static_assert(sizeof(EventData_11) == 19 * 4, "Wrong size");

void METHOD ModifyChanceDistributionWithPlayerLevel(unsigned int dst, int, unsigned int type) {
    unsigned int v2 = 100;
    if (!type || type == 1) {
        unsigned int level = (unsigned int)*(unsigned char *)(*(unsigned int *)(dst + 760) + 144);
        v2 = (level * level) / 50 + 200;
    }
    *(unsigned int *)(dst + 772) = (signed int)(v2 * *(unsigned int *)(dst + 772)) / 100;
}

void METHOD ModifyChanceDistributionWithPlayerLevel_11(unsigned int dst, int, unsigned int type) {
    unsigned int v2 = 100;
    if (!type || type == 1) {
        unsigned int level = (unsigned int)*(unsigned char *)(*(unsigned int *)(dst + 876) + 108);
        v2 = (level * level) / 50 + 200;
    }
    *(unsigned int *)(dst + 888) = (signed int)(v2 * *(unsigned int *)(dst + 888)) / 100;
}

void PatchPlayerMatchEvents(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

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

        events[PLAYERID_ATTACKER].posn[POS_LW] = 140;
        events[PLAYERID_ATTACKER].posn[POS_RW] = 140;
        events[PLAYERID_ATTACKER].posn[POS_CF] = 135;
        events[PLAYERID_ATTACKER].posn[POS_ST] = 140;

        events[PLAYERID_ATTACKER_2].posn[POS_LW] = 140;
        events[PLAYERID_ATTACKER_2].posn[POS_RW] = 140;
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
    }
    else if (v.id() == ID_FM_11_1003) {

        patch::RedirectJump(0x1093610, ModifyChanceDistributionWithPlayerLevel_11);

        DWORD oldProtect;
        VirtualProtect((void *)0x136C6D8, 71 * 19 * 4, PAGE_READWRITE, &oldProtect);
        EventData_11 *events = (EventData_11 *)0x136C6D8;

        events[PLAYERID_11_DISTANCE].posn[POS_11_LW] = 90;
        events[PLAYERID_11_DISTANCE].posn[POS_11_RW] = 90;

        events[PLAYERID_11_HEADER].posn[POS_11_LW] = 60;
        events[PLAYERID_11_HEADER].posn[POS_11_RW] = 60;

        events[PLAYERID_11_HEADER_SET_PIECE].posn[POS_11_LW] = 50;
        events[PLAYERID_11_HEADER_SET_PIECE].posn[POS_11_RW] = 50;

        events[PLAYERID_11_DRIBBLING].posn[POS_11_LW] = 80;
        events[PLAYERID_11_DRIBBLING].posn[POS_11_RW] = 80;

        events[PLAYERID_11_BOX].posn[POS_11_LW] = 80;
        events[PLAYERID_11_BOX].posn[POS_11_RW] = 80;

        events[PLAYERID_11_ATTACKER].posn[POS_11_LW] = 140;
        events[PLAYERID_11_ATTACKER].posn[POS_11_RW] = 140;
        events[PLAYERID_11_ATTACKER].posn[POS_11_CF] = 135;
        events[PLAYERID_11_ATTACKER].posn[POS_11_ST] = 140;

        events[PLAYERID_11_ATTACKER_2].posn[POS_11_LW] = 140;
        events[PLAYERID_11_ATTACKER_2].posn[POS_11_RW] = 140;
        events[PLAYERID_11_ATTACKER_2].posn[POS_11_CF] = 135;
        events[PLAYERID_11_ATTACKER_2].posn[POS_11_ST] = 140;

        events[PLAYERID_11_ATTACKER_SPEED].posn[POS_11_LW] = 100;
        events[PLAYERID_11_ATTACKER_SPEED].posn[POS_11_RW] = 100;

        events[PLAYERID_11_ATTACKER_TECHNIQUE].posn[POS_11_LW] = 100;
        events[PLAYERID_11_ATTACKER_TECHNIQUE].posn[POS_11_RW] = 100;

        events[PLAYERID_11_DIVING].posn[POS_11_LW] = 90;
        events[PLAYERID_11_DIVING].posn[POS_11_RW] = 90;
    }
}
