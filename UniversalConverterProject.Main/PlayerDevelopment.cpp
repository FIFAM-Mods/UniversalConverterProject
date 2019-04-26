#include "3dPatches.h"

using namespace plugin;

#define CHANGE_SKILL_DEVELOPMENT 0xFBA8C0
#define GET_PLAYER_FORM 0xFA2AD0
#define GET_PLAYER_AGE 0xF9B2F0
#define GET_PLAYER_TALENT 0xFB2590
#define GET_PLAYER_BEST_POSN 0xFAAE10
#define GET_PLAYER_LEVEL 0xFAAD60

class CDBPlayer;

float TranslateChangeCoeff(float value, float min, float max, float coeff, bool highest) {
    if (value < min)
        return highest ? 0.0f : coeff;
    else if (value > max)
        return highest ? coeff : 0.0f;
    float divisor = max - min + 2.0f;
    if (highest)
        return (value - min + 1.0f) / divisor * coeff;
    return (divisor - (value - min + 1.0f)) / divisor * coeff;
}

template <unsigned int Addr>
bool METHOD ChangeSkillDevelopment(CDBPlayer *player, DUMMY_ARG, unsigned char skillId, int change) {
    unsigned char form = CallMethodAndReturn<unsigned char, GET_PLAYER_FORM>(player); // 0 to 20
    unsigned int age = CallMethodAndReturn<unsigned int, GET_PLAYER_AGE>(player);
    unsigned char talent = CallMethodAndReturn<unsigned char, GET_PLAYER_TALENT>(player, 0);
    //char posn = CallMethodAndReturn<char, GET_PLAYER_BEST_POSN>(player);
    //unsigned char level = CallMethodAndReturn<unsigned char, GET_PLAYER_LEVEL>(player, posn, 0);
    //unsigned char fitness = CallMethodAndReturn<unsigned char, 0xF9BC00>(player);
    //unsigned char morale = 0;
    //if (change < 0) {
    //    // regress - age part, form part, talent part
    //    float agePart = TranslateChangeCoeff((float)age, 25, 31, 0.3f, true);
    //    float talentPart = TranslateChangeCoeff((float)talent, 4, 8, 0.2f, false);
    //    //float formPart = TranslateChangeCoeff(form, 4, 10, 0.1f, false);
    //
    //    change = (int)((float)change * (0.5f + agePart + talentPart));
    //    if (change == 0)
    //        change = -1;
    //}
    //else {
    //    //float agePart = TranslateChangeCoeff(age, 25, 29, 0.1f, false);
    //    //float talentPart = TranslateChangeCoeff(talent, 4, 8, 0.3f, true);
    //    //float formPart = TranslateChangeCoeff(form, 4, 12, 0.2f, true);
    //    //
    //    //change = (int)((float)change * (1.0f + agePart + talentPart + formPart));
    //}
    bool result = CallMethodAndReturn<bool, CHANGE_SKILL_DEVELOPMENT>(player, skillId, change);
    return result;
}

double decreaseMp = 0.1;

void PatchPlayerDevelopment(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        //patch::SetPointer(0x103166A + 2, &decreaseMp);
        patch::RedirectCall(0xFBC9B0,  ChangeSkillDevelopment<0xFBC9B0 >);
        patch::RedirectCall(0xFBCADB, ChangeSkillDevelopment <0xFBCADB >);
        patch::RedirectCall(0x12A7EBB, ChangeSkillDevelopment<0x12A7EBB>);
        patch::RedirectCall(0x12A8226, ChangeSkillDevelopment<0x12A8226>);
        patch::RedirectCall(0x12A87A5, ChangeSkillDevelopment<0x12A87A5>);
        patch::RedirectCall(0x12A9709, ChangeSkillDevelopment<0x12A9709>);
        patch::RedirectCall(0x12AEABC, ChangeSkillDevelopment<0x12AEABC>);
        patch::RedirectCall(0x12AEC59, ChangeSkillDevelopment<0x12AEC59>);
        patch::RedirectCall(0x12B01E7, ChangeSkillDevelopment<0x12B01E7>);
        patch::RedirectCall(0x13FD58B, ChangeSkillDevelopment<0x13FD58B>);
    }
}
