#pragma once
#include <vector>

class Settings {
    std::pair<bool, bool> mExtendLoansLimit = { true, true };
    std::pair<bool, bool> mTeamControl = { false, false };
    std::pair<bool, bool> mManualPlayerSwitch = { false, false };
    std::pair<int, int> mTeamControlDifficulty = { 100, 100 };
    std::pair<int, int> mNTBudgetMultiplier = { 15, 15 };
    std::pair<bool, bool> mClubSponsorLogos = { true, true };
    std::pair<bool, bool> mClubAdboards = { true, true };
    std::pair<bool, bool> mDisableTeamControl = { true, true };
    std::pair<bool, bool> mUseNew3dPitch = { false, false };
    std::pair<bool, bool> mNoHardwareAccelerationFix = { false, false };
    std::pair<int, int> mCrowdResolution = { 1024, 1024 };
    std::pair<bool, bool> mRandomizeTalents = { false, false };
    std::pair<bool, bool> mUseRealSalaries = { false, false };
    std::pair<bool, bool> mUseHairLODs = { true, true };
    std::pair<bool, bool> mEnableSpeechInAllMatches = { false, false };
    std::pair<int, int> mAdboardTransitionDuration = { 1, 1 };
    std::pair<int, int> mAdboardDisplayDuration = { 15, 15 };
    std::pair<bool, bool> mEnableDefaultStadiums = { false, false };
    std::pair<bool, bool> mEnableCommentaryPatches = { false, false };
    std::pair<bool, bool> mJerseyNamesInAllMatches = { false, false };
    std::pair<int, int> mPlayerNameEditMode = { 0, 0 };

public:
    bool getExtendLoansLimit();
    void setExtendLoansLimit(bool set);
    bool getTeamControl();
    void setTeamControl(bool set);
    bool getManualPlayerSwitch();
    void setManualPlayerSwitch(bool set);
    int getTeamControlDifficulty();
    void setTeamControlDifficulty(int difficulty);
    int getNTBudgetMultiplier();
    void setNTBudgetMultiplier(int multiplier);
    bool getClubSponsorLogos();
    void setClubSponsorLogos(bool set);
    bool getClubAdboards();
    void setClubAdboards(bool set);
    bool getTeamControlDisabled();
    bool getTeamControlDisabledAtGameStart();
    void setTeamControlDisabled(bool set);
    bool getUseNew3dPitch();
    void setUseNew3dPitch(bool set);
    bool getNoHardwareAccelerationFix();
    void setNoHardwareAccelerationFix(bool set);
    int getCrowdResolution();
    void setCrowdResolution(int resolution);
    bool getRandomizeTalents();
    void setRandomizeTalents(bool set);
    bool getUseRealSalaries();
    void setUseRealSalaries(bool set);
    bool getUseHairLODs();
    void setUseHairLODs(bool set);
    bool getEnableSpeechInAllMatches();
    void setEnableSpeechInAllMatches(bool set);
    int getAdboardTransitionDuration();
    void setAdboardTransitionDuration(int duration);
    int getAdboardDisplayDuration();
    void setAdboardDisplayDuration(int duration);
    bool getEnableDefaultStadiums();
    void setEnableDefaultStadiums(bool set);
    bool getJerseyNamesInAllMatches();
    void setJerseyNamesInAllMatches(bool set);
    int getPlayerNameEditMode();
    void setPlayerNameEditMode(int mode);

    bool getEnableCommentaryPatches();
    void setEnableCommentaryPatches(bool set);

    static Settings &GetInstance();
    void save();
private:
    Settings();
};
