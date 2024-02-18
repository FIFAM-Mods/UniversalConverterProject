#include "Achievements.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "Utils.h"

using namespace plugin;

const UInt GAME_OBJECTIVES_PROFILE_VERSION = 24;
const UInt GAME_OBJECTIVES_NUM_ACHIEVEMENTS = 157;
const UInt GAME_OBJECTIVES_NUM_REWARDS = 29;

template <UInt Size>
struct GameObjectiveBits {
    unsigned char data[Size / 8 + (Size % 8 != 0)] = {};

    constexpr bool Get(UInt index) const noexcept {
        return (data[index / 8] >> (index % 8)) & 1U;
    }

    constexpr void Set(UInt index) noexcept {
        data[index / 8] |= (1U << (index % 8));
    }

    constexpr void Clear(UInt index) noexcept {
        data[index / 8] &= ~(1U << (index % 8));
    }
};

struct GameObjectivesData {
    UInt unk0;
    UInt coinsCount;
    GameObjectiveBits<64> rewards;
    GameObjectiveBits<256> achievements;
};

struct GameObjectivesProfile {
    UInt unk0;
    UInt coins;
    Vector<Bool> achievements;
    Vector<Bool> rewards;
};

Map<UInt, GameObjectivesProfile> &GameObjectivesProfiles() {
    static Map<UInt, GameObjectivesProfile> gameObjectivesProfiles;
    return gameObjectivesProfiles;
}

void SyncGameObjectiveProfile(GameObjectivesProfile *profile, UInt profileVersion, SyncFile *syncFile, UInt mode) {
    syncFile->Chunk('GPRO', 1);
    UInt defaultCount = 0;
    syncFile->UInt32(&profileVersion, &defaultCount, 1);
    syncFile->UInt32(&profile->unk0, &defaultCount, 1);
    syncFile->UInt32(&profile->coins, &defaultCount, 1);
    UInt numAchievements = profile->achievements.size();
    syncFile->UInt32(&numAchievements, &defaultCount, 1);
    if (mode == SyncFile::Read)
        profile->achievements.resize(numAchievements);
    for (UInt i = 0; i < numAchievements; i++) {
        UChar defaultAchievement = 0;
        UChar achievement = profile->achievements[i];
        syncFile->UInt8(&achievement, &defaultAchievement, 1);
    }
    UInt numRewards = profile->rewards.size();
    syncFile->UInt32(&numRewards, &defaultCount, 1);
    if (mode == SyncFile::Read)
        profile->rewards.resize(numRewards);
    for (UInt i = 0; i < numRewards; i++) {
        UChar defaultReward = 0;
        UChar reward = profile->rewards[i];
        syncFile->UInt8(&reward, &defaultReward, 1);
    }
}

void LoadGameObjectivesFile() {
    GameObjectivesProfiles().clear();
    SyncFile syncFile;
    WideChar filePath[260];
    void *resolver = CallAndReturn<void *, 0x40BF10>();
    CallVirtualMethod<3>(resolver, filePath, 3, L"");
    wcscat(filePath, L"GameObjectives.dat");
    if (syncFile.Load(filePath, SyncFile::Read, 1)) {
        if (syncFile.Chunk('GOBJ', 1)) {
            UInt defaultCount = 0;
            UInt numProfiles = 0;
            syncFile.UInt32(&numProfiles, &defaultCount, 1);
            for (UInt i = 0; i < numProfiles; i++) {
                syncFile.Chunk('GPRO', 1);
                UInt profileVersion = 0;
                syncFile.UInt32(&profileVersion, &defaultCount, 1);
                auto &profile = GameObjectivesProfiles()[profileVersion];
                syncFile.UInt32(&profile.unk0, &defaultCount, 1);
                syncFile.UInt32(&profile.coins, &defaultCount, 1);
                UInt numAchievements = 0;
                syncFile.UInt32(&numAchievements, &defaultCount, 1);
                profile.achievements.resize(numAchievements);
                for (UInt i = 0; i < numAchievements; i++) {
                    UChar defaultAchievement = 0;
                    UChar achievement = 0;
                    syncFile.UInt8(&achievement, &defaultAchievement, 1);
                    profile.achievements[i] = achievement;
                }
                UInt numRewards = 0;
                syncFile.UInt32(&numRewards, &defaultCount, 1);
                profile.rewards.resize(numRewards);
                for (UInt i = 0; i < numRewards; i++) {
                    UChar defaultReward = 0;
                    UChar reward = 0;
                    syncFile.UInt8(&reward, &defaultReward, 1);
                    profile.rewards[i] = reward;
                }
            }
        }
        syncFile.Close();
    }
}

void SaveGameObjectivesFile() {
    SyncFile syncFile;
    WideChar filePath[260];
    void *resolver = CallAndReturn<void *, 0x40BF10>();
    CallVirtualMethod<3>(resolver, filePath, 3, L"");
    wcscat(filePath, L"GameObjectives.dat");
    if (syncFile.Load(filePath, SyncFile::Write, 1)) {
        if (syncFile.Chunk('GOBJ', 1)) {
            UInt defaultCount = 0;
            UInt numProfiles = GameObjectivesProfiles().size();
            syncFile.UInt32(&numProfiles, &defaultCount, 1);
            for (auto &[profileKey, profile] : GameObjectivesProfiles()) {
                syncFile.Chunk('GPRO', 1);
                UInt profileVersion = profileKey;
                syncFile.UInt32(&profileVersion, &defaultCount, 1);
                syncFile.UInt32(&profile.unk0, &defaultCount, 1);
                syncFile.UInt32(&profile.coins, &defaultCount, 1);
                UInt numAchievements = profile.achievements.size();
                syncFile.UInt32(&numAchievements, &defaultCount, 1);
                for (UInt i = 0; i < numAchievements; i++) {
                    UChar defaultAchievement = 0;
                    UChar achievement = profile.achievements[i];
                    syncFile.UInt8(&achievement, &defaultAchievement, 1);
                }
                UInt numRewards = profile.rewards.size();
                syncFile.UInt32(&numRewards, &defaultCount, 1);
                for (UInt i = 0; i < numRewards; i++) {
                    UChar defaultReward = 0;
                    UChar reward = profile.rewards[i];
                    syncFile.UInt8(&reward, &defaultReward, 1);
                }
            }
        }
        syncFile.Close();
    }
}

Bool OnGetIsConnected() {
    return true;
}

Bool OnGetIsLoggedIn() {
    return true;
}

void *METHOD OnConstructDBGameObjectiveController(void *objectives) {
    CallMethod<0x11218F0>(objectives); // CDBGameObjectiveController::CDBGameObjectiveController()
    LoadGameObjectivesFile();
    GameObjectivesData data;
    memset(&data, 0, sizeof(GameObjectivesData));
    if (GameObjectivesProfiles().contains(GAME_OBJECTIVES_PROFILE_VERSION)) {
        auto &profile = GameObjectivesProfiles()[GAME_OBJECTIVES_PROFILE_VERSION];
        data.unk0 = profile.unk0;
        data.coinsCount = profile.coins;
        if (profile.achievements.size() > 0) {
            for (UInt i = 1; i <= Utils::Min(profile.achievements.size(), GAME_OBJECTIVES_NUM_ACHIEVEMENTS); i++) {
                if (profile.achievements[i - 1])
                    data.achievements.Set(i);
            }
        }
        if (profile.rewards.size() > 0) {
            for (UInt i = 1; i <= Utils::Min(profile.rewards.size(), GAME_OBJECTIVES_NUM_REWARDS); i++) {
                if (profile.rewards[i - 1])
                    data.rewards.Set(i);
            }
        }
    }
    CallMethod<0x11222B0>(objectives, &data); // CDBGameObjectiveController::SetData()
    CallMethod<0x1121320>(objectives, true); // CDBGameObjectiveController::SetActive()
    //*raw_ptr<UInt>(objectives, 0x170) = 0;
    return objectives;
}

void METHOD OnUploadGameObjectivesData(void *t, DUMMY_ARG, void *d) {
    CallMethod<0x13BB0C0>(t, d);
    GameObjectivesData *data = raw_ptr<GameObjectivesData>(t, 0x5F0);
    auto &profile = GameObjectivesProfiles()[GAME_OBJECTIVES_PROFILE_VERSION];
    profile.unk0 = data->unk0;
    profile.coins = data->coinsCount;
    profile.achievements.clear();
    profile.achievements.resize(GAME_OBJECTIVES_NUM_ACHIEVEMENTS, false);
    profile.rewards.clear();
    profile.rewards.resize(GAME_OBJECTIVES_NUM_REWARDS, false);
    for (UInt i = 1; i <= GAME_OBJECTIVES_NUM_ACHIEVEMENTS; i++)
        profile.achievements[i - 1] = data->achievements.Get(i);
    for (UInt i = 1; i <= GAME_OBJECTIVES_NUM_REWARDS; i++)
        profile.rewards[i - 1] = data->rewards.Get(i);
    SaveGameObjectivesFile();
}

void PatchAchievements(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x9CBC63, OnGetIsConnected);
        patch::RedirectCall(0xE4C29D, OnGetIsLoggedIn);
        patch::RedirectCall(0xE4BC8D, OnGetIsLoggedIn);
        patch::RedirectCall(0x4B9E6D, OnConstructDBGameObjectiveController);
        patch::RedirectCall(0x1120AB2, OnUploadGameObjectivesData);
    }
}
