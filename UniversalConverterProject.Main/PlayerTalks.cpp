#include "PlayerTalks.h"
#include "GameInterfaces.h"
#include "Random.h"

using namespace plugin;

void *gCurrPlayerTalkData = nullptr;
CDBPlayer *gPlayerTalkPlayer = nullptr;

struct PlayerTalkResult {
    unsigned int moraleChange;
    unsigned int messageId;
    unsigned char formatter[0x270];
};

class CPlayerTalkData {
public:
    char morale;
    char form;
    char field_2;
    char field_3;
    char atmosphere;
    char field_5;
    char field_6;
    char field_7;
    char leagueMatchday;
    char numBetterPlayers;
    char field_A;
    char energy2;
    char energy;
    char fitness;
    char level;
    char position;
    char fieldPart;
    char field_11;
    char field_12;
    char field_13;
    char field_14;
    char field_15;
    char injury;
    char field_17;
    char isInFirstTeam;
private:
    char _pad19[3];
public:
    int field_1C;
    float field_20;
    int contractLeftDays;
    int playerId;
    CTeamIndex teamIndex;
    int field_30;
    CCompID compId;
    CDBTeam *team;
    CDBPlayer *player;
    CDBLeague *league;
};

class CPlayerTalk {
public:
    CPlayerTalkData data;
    int otherPlayerId;
    char skillId;
private:
    char _pad4D[3];
public:
    unsigned int type;

    CPlayerTalk(CPlayerTalkData const &_data, unsigned int _type) {
        CallMethod<0x1097740>(this, &_data, _type);
    }
    virtual bool IsAvailable() = 0;
    virtual PlayerTalkResult &Result(PlayerTalkResult &outResult) = 0;
    virtual void Message(wchar_t *out, int outLen) {
        CallMethod<0x10941A0>(this, out, outLen);
    }
    virtual bool Unknown() = 0;
    static void *operator new(size_t size) {
        return CallAndReturn<void *, 0x15738F3>(size);
    }
    static void operator delete(void *data) {
        Call<0x157347A>(data);
    }
};

class CPlayerTalkTest : public CPlayerTalk {
public:
    CPlayerTalkTest(CPlayerTalkData const &data) : CPlayerTalk(data, 84) {}

    bool IsAvailable() override {
        return data.player->GetAge() >= 30; // test condition - if age is greater than 30
    }

    PlayerTalkResult &Result(PlayerTalkResult &outResult) override {
        if (Random::Get(0, 1)) { // random result
            outResult.messageId = 4000; // result 1
        }
        else {
            outResult.moraleChange = -20; // decrease morale
            outResult.messageId = 4001; // result 2
        }
        return outResult;
    }

    bool Unknown() override {
        return true;
    }
};

void METHOD InitialisePlayerTalks(void *t, DUMMY_ARG, void *vec) {
    CPlayerTalkData data;
    CallMethod<0x1093C40>(&data, *raw_ptr<void *>(t));
    static const unsigned int origTalks[] = {
        0x1097770, 0x10977A0, 0x10977E0, 0x1097820, 0x1097860, 0x10978A0, 0x10978E0, 0x1097920, 0x1097960, 0x10979A0,
        0x10979E0, 0x1097A20, 0x1097A60, 0x1097AA0, 0x1097AE0, 0x1097B20, 0x1097B60, 0x1097BA0, 0x1097BE0, 0x1097C20,
        0x1097C60, 0x1097CA0, 0x1097CE0, 0x1097D20, 0x1097D60, 0x1097DA0, 0x1097DE0, 0x1097E20, 0x1097E60, 0x1097EA0,
        0x1097EE0, 0x1097F20, 0x1097F60, 0x1097FA0, 0x1097FE0, 0x1098020, 0x1098060, 0x10980A0, 0x10980E0, 0x1098120,
        0x1098160, 0x10981A0, 0x10981E0, 0x1098220, 0x1098260, 0x10982A0, 0x10982E0, 0x1098320, 0x1098360, 0x10983A0,
        0x10983E0, 0x1098420, 0x1098460, 0x10984A0, 0x10984E0, 0x1098520, 0x1098560, 0x10985A0, 0x10985E0, 0x1098620,
        0x1098660, 0x10986A0, 0x10986E0, 0x1098720, 0x1098760, 0x10987A0, 0x10987E0, 0x1098820, 0x1098860, 0x10988A0,
        0x10988E0
    };
    for (auto i : origTalks) {
        CPlayerTalk *talk = CallAndReturn<CPlayerTalk *, 0x15738F3>(sizeof(CPlayerTalk));
        CallMethodDynGlobal(i, talk, &data);
        if (talk->IsAvailable())
            CallMethod<0x10997A0>(vec, &talk);
        else
            Call<0x157347A>(talk);
    }
    auto RegisterTalk = [&](CPlayerTalk *talk) {
        if (talk->IsAvailable())
            CallMethod<0x10997A0>(vec, &talk);
        else
            delete talk;
    };
    RegisterTalk(new CPlayerTalkTest(data));
}

void PatchPlayerTalks(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectJump(0x1099830, InitialisePlayerTalks);
    }
}
