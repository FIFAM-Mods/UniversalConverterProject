#include "Achievements.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "Utils.h"

using namespace plugin;

const UInt GAME_OBJECTIVES_PROFILE_VERSION = 24;
const UInt GAME_OBJECTIVES_NUM_ACHIEVEMENTS = 157;
const UInt GAME_OBJECTIVES_NUM_REWARDS = 29;

enum eNewRewardId {
	NEWREWARD_UNKNOWN,
	NEWREWARD_SPECIAL_TRAINING,
	NEWREWARD_SPECIAL_TRANSFERS,
	NEWREWARD_SPECIAL_CLUB,
	NEWREWARD_SPECIAL_CAREER,
	NEWREWARD_KITPACK1,
	NEWREWARD_KITPACK2,
	NEWREWARD_KITPACK3,

};

struct RewardInfo {
	const Char *pHeader;
	const Char *pText;
	Int nCoins;
	const WideChar *pIconName;
	UInt nId;
};

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
    if (Utils::Contains(GameObjectivesProfiles(), GAME_OBJECTIVES_PROFILE_VERSION)) {
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

Bool METHOD OriginalRewardUnlocked(void *obj, DUMMY_ARG, UInt rewardId) {
	return true;
}

Bool METHOD Rewards_RewardUnlocked(void *rewards, DUMMY_ARG, UInt id) {
	if (id < 30)
		return ((1 << (id & 0x1F)) & *(UInt *)(((UInt)rewards) + 4 * (id >> 5))) != 0;
	return false;
}

Bool METHOD ObjectiveController_RewardUnlocked(void *obj, DUMMY_ARG, UInt id) {
	return Rewards_RewardUnlocked(raw_ptr<void>(obj, 0x16C), 0, id);
}

Bool IsRewardUnlocked(UInt id) {
	return ObjectiveController_RewardUnlocked(CallAndReturn<void *, 0x4B9E30>(), 0, id);
}

WideChar const *METHOD GetRewardIconPath(UInt *r, WideChar *out, UInt maxLen) {
	//switch (*r) {
	//case NEWREWARD_KITPACK1:
	//	wcscpy(out, L"art\\items\\Icons\\Rewards\\reward_icon_kitpack1.tga");
	//	return out;
	//case NEWREWARD_KITPACK2:
	//	wcscpy(out, L"art\\items\\Icons\\Rewards\\reward_icon_kitpack2.tga");
	//	return out;
	//case NEWREWARD_KITPACK3:
	//	wcscpy(out, L"art\\items\\Icons\\Rewards\\reward_icon_kitpack3.tga");
	//	return out;
	//}
	out[maxLen - 1] = L'\0';
	//_snwprintf(out, maxLen - 1, L"art\\items\\Icons\\Rewards\\reward_icon_%s.tga", gRewardInfo[*r].pIconName);
	return out;
}

UChar GetMaxShirt() {
	UChar maxShirt = 66;
	if (IsRewardUnlocked(NEWREWARD_KITPACK1)) {
		maxShirt += 20;
		if (IsRewardUnlocked(NEWREWARD_KITPACK2)) {
			maxShirt += 30;
			if (IsRewardUnlocked(NEWREWARD_KITPACK3))
				maxShirt += 35;
		}
	}
	return maxShirt;
}

UChar GetMaxShorts() {
	UChar maxShorts = 13;
	if (IsRewardUnlocked(NEWREWARD_KITPACK1)) {
		maxShorts += 5;
		if (IsRewardUnlocked(NEWREWARD_KITPACK2)) {
			maxShorts += 10;
			if (IsRewardUnlocked(NEWREWARD_KITPACK3))
				maxShorts += 10;
		}
	}
	return maxShorts;
}

UChar GetMaxSocks() {
	UChar maxSocks = 4;
	if (IsRewardUnlocked(NEWREWARD_KITPACK1)) {
		maxSocks += 5;
		if (IsRewardUnlocked(NEWREWARD_KITPACK2)) {
			maxSocks += 10;
			if (IsRewardUnlocked(NEWREWARD_KITPACK3))
				maxSocks += 10;
		}
	}
	return maxSocks;
}

void METHOD Shirt_SetMinMax(void *sb) {
	UInt64 values[2] = { 1, GetMaxShirt() };
	CallVirtualMethod<26>(sb, &values);
}

void METHOD Shorts_SetMinMax(void *sb) {
	UInt64 values[2] = { 1, GetMaxShorts() };
	CallVirtualMethod<26>(sb, &values);
}

void METHOD Socks_SetMinMax(void *sb) {
	UInt64 values[2] = { 1, GetMaxSocks() };
	CallVirtualMethod<26>(sb, &values);
}

UChar METHOD GetKitPartType_Ret1(void *t, DUMMY_ARG, UChar bAway, UChar part) {
	return 1;
}

UChar METHOD Shirt_GetKitPartType(void *t, DUMMY_ARG, UChar bAway, UChar part) {
	UChar type = CallMethodAndReturn<UChar, 0xFFD090>(t, bAway, part);
	if (type <= GetMaxShirt())
		return 1;
	return 66;
}

UChar METHOD Shorts_GetKitPartType(void *t, DUMMY_ARG, UChar bAway, UChar part) {
	UChar type = CallMethodAndReturn<UChar, 0xFFD090>(t, bAway, part);
	if (type <= GetMaxShorts())
		return 1;
	return 13;
}

UChar METHOD Socks_GetKitPartType(void *t, DUMMY_ARG, UChar bAway, UChar part) {
	UChar type = CallMethodAndReturn<UChar, 0xFFD090>(t, bAway, part);
	if (type <= GetMaxSocks())
		return 1;
	return 4;
}

void PatchAchievements(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x9CBC63, OnGetIsConnected);
		patch::RedirectCall(0x9E9B96, OnGetIsConnected); // Mobile phone
		patch::RedirectCall(0x9E9DC1, OnGetIsConnected); // Mobile phone
        patch::RedirectCall(0xE4C29D, OnGetIsLoggedIn);
        patch::RedirectCall(0xE4BC8D, OnGetIsLoggedIn);
        patch::RedirectCall(0x4B9E6D, OnConstructDBGameObjectiveController);
        patch::RedirectCall(0x1120AB2, OnUploadGameObjectivesData);

	//	// unlock all original rewards
	//	patch::SetUChar(0x6B079F, 0xEB); // disable "special sponsor" completely
	//	patch::RedirectJump(0x1120FA0, OriginalRewardUnlocked);
	//	patch::RedirectCall(0xE4C642, ObjectiveController_RewardUnlocked);
	//	patch::RedirectCall(0x11210C4, Rewards_RewardUnlocked);
	//	patch::RedirectCall(0x1121668, Rewards_RewardUnlocked);
	//	patch::RedirectCall(0x112168C, Rewards_RewardUnlocked);
	//	patch::RedirectCall(0x1121791, Rewards_RewardUnlocked);
	//
	//	RewardInfo *gRewardInfo = (RewardInfo *)0x3098B78;
	//	gRewardInfo[NEWREWARD_KITPACK1].nCoins = 10;
	//	gRewardInfo[NEWREWARD_KITPACK1].pIconName = L"kitpack1";
	//	gRewardInfo[NEWREWARD_KITPACK2].nCoins = 20;
	//	gRewardInfo[NEWREWARD_KITPACK2].pIconName = L"kitpack2";
	//	gRewardInfo[NEWREWARD_KITPACK3].nCoins = 30;
	//	gRewardInfo[NEWREWARD_KITPACK3].pIconName = L"kitpack3";
	//
	//	// Shop - New generic kits packs
	//
	//	// CClubShirtsDesign::CreateUI
	//	patch::RedirectCall(0x59AB02, Shirt_SetMinMax);
	//	patch::Nop(0x59AB02 + 5, 2);
	//	patch::RedirectCall(0x59AB28, Shorts_SetMinMax);
	//	patch::Nop(0x59AB28 + 5, 2);
	//	patch::RedirectCall(0x59AB4E, Socks_SetMinMax);
	//	patch::Nop(0x59AB4E + 5, 2);
	//	patch::RedirectCall(0x59AC41, GetKitPartType_Ret1);
	//	patch::RedirectCall(0x59AC91, GetKitPartType_Ret1);
	//	patch::RedirectCall(0x59ACE5, GetKitPartType_Ret1);
	//	patch::RedirectCall(0x59AD3A, GetKitPartType_Ret1);
	//	patch::RedirectCall(0x59AD92, GetKitPartType_Ret1);
	//	patch::RedirectCall(0x59ADE7, GetKitPartType_Ret1);
	//	patch::RedirectCall(0x59AC5F, Shirt_GetKitPartType);
	//	patch::RedirectCall(0x59ACB1, Shirt_GetKitPartType);
	//	patch::RedirectCall(0x59AD05, Shorts_GetKitPartType);
	//	patch::RedirectCall(0x59AD5C, Shorts_GetKitPartType);
	//	patch::RedirectCall(0x59ADB2, Socks_GetKitPartType);
	//	patch::RedirectCall(0x59AE09, Socks_GetKitPartType);
    }
}
