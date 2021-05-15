#include "PlayerTalks.h"
#include "GameInterfaces.h"

using namespace plugin;

void *gCurrPlayerTalkData = nullptr;
CDBPlayer *gPlayerTalkPlayer = nullptr;

struct PlayerTalkResult {
    unsigned int moraleChange;
    unsigned int messageId;
    unsigned char formatter[0x270];
};

class CPlayerTalk {
public:
    char data[0x3C];
    CDBPlayer *player;
    CDBLeague *teamLeague;
    int field_48;
    char skillId;
private:
    char _pad4D[3];
public:
    unsigned int type;

    CPlayerTalk(unsigned int _type) {
        CallMethod<0x10940A0>(raw_ptr<void *>(this, 4), gCurrPlayerTalkData);
        field_48 = 0;
        skillId = -1;
        type = _type;
    }
    virtual bool IsAvailable() = 0;
    virtual PlayerTalkResult &Result(PlayerTalkResult &outResult) = 0;
    virtual void Message(wchar_t *out, int outLen) {
        CallMethod<0x10941A0>(this, out, outLen);
    }
    virtual bool Unknown() = 0;
    void *operator new(size_t size) {
        return CallAndReturn<void *, 0x15738F3>(size);
    }
    void operator delete(void *data) {
        Call<0x157347A>(data);
    }
};

class CPlayerTalkTest : public CPlayerTalk {
public:
    CPlayerTalkTest() : CPlayerTalk(100) {}

    bool IsAvailable() override {
        //Error(L"Player age: %d", player->GetAge());
        return player->GetAge() >= 40;
    }

    PlayerTalkResult &Result(PlayerTalkResult &outResult) override {
        Error("Result");
        outResult.moraleChange = 0;
        outResult.messageId = 3423;
        return outResult;
    }

    void Message(wchar_t *out, int outLen) override {
        wcscpy_s(out, outLen, GetTranslation("PLAYER_TALK_100"));
    }

    bool Unknown() override {
        Error("Unknown");
        return true;
    }
};

void METHOD OnRegisterTalk(void *t, DUMMY_ARG, void *vec) {
    CallMethod<0x1099830>(t, vec);
    char data[0x44];
    //CallMethod<0x1093C40>();
    auto RegisterTalk = [&](CPlayerTalk *talk) {
        if (talk->IsAvailable())
            CallMethod<0x10997A0>(vec, &talk);
        else
            delete talk;
    };
    RegisterTalk(new CPlayerTalkTest);
}

void METHOD OnPlayerTalkGetPlayerData(void *data, DUMMY_ARG, CDBPlayer *player) {
    
    //Error(L"Player %s", name);
    gCurrPlayerTalkData = data;
    gPlayerTalkPlayer = player;
    CallMethod<0x1093C40>(data, player);
}

void PatchPlayerTalks(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        //patch::RedirectCall(0x58550F, OnRegisterTalk);
        //patch::RedirectCall(0x5935DE, OnRegisterTalk);
        //patch::RedirectCall(0x5AFB14, OnRegisterTalk);
        //patch::RedirectCall(0x5B989F, OnRegisterTalk);
        //patch::RedirectCall(0x62F22C, OnRegisterTalk);
        //patch::RedirectCall(0x109983E, OnPlayerTalkGetPlayerData);
    }
}
