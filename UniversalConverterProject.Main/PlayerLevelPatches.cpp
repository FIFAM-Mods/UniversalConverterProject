#include "PlayerLevelPatches.h"

using namespace plugin;

void METHOD OnPlayerInfoCareerDrawLevelLine(CXgFMPanel *screen, DUMMY_ARG, Int *points,
    UInt numPoints, void *rect, UInt color, void *data)
{
    auto ChkFit = *raw_ptr<CXgCheckBox *>(screen, 0x12C4);
    if (ChkFit->IsVisible())
        CallMethod<0x5D68C0>(screen, points, numPoints, rect, color, data);
}

void DisablePlayerLevelOnPlayerCareerScreen(CXgFMPanel *screen, CDBPlayer *player) {
    if (!HidePlayerLevel()) {
        auto employee = GetManagerWhoLooksForPlayer(player->GetID());
        Bool hide = employee && employee->GetPlayerKnowledge(player) != 10;
        Bool display = hide == false;
        auto ChkFit = *raw_ptr<CXgCheckBox *>(screen, 0x12C4);
        auto TbColor4 = *raw_ptr<CXgTextBox *>(screen, 0x12F8);
        auto TbLev = *raw_ptr<CXgTextBox *>(screen, 0x12D4);
        TbColor4->SetVisible(display);
        ChkFit->SetVisible(display);
        TbLev->SetVisible(display);
    }
}

void METHOD OnPlayerInfoHistoryDrawLevelLine(CXgFMPanel *screen, DUMMY_ARG, Int *points,
    UInt numPoints, void *rect, UInt color, void *data)
{
    auto ChkLevel = *raw_ptr<CXgCheckBox *>(screen, 0xBB8);
    if (ChkLevel->IsVisible())
        CallMethod<0x5DD000>(screen, points, numPoints, rect, color, data);
}

void METHOD OnPlayerInfoHistoryDrawMarketValueLine(CXgFMPanel *screen, DUMMY_ARG, Int *points,
    UInt numPoints, void *rect, UInt color, void *data)
{
    auto ChkMarketValue = *raw_ptr<CXgCheckBox *>(screen, 0xBB4);
    if (ChkMarketValue->IsVisible())
        CallMethod<0x5DD000>(screen, points, numPoints, rect, color, data);
}

Int METHOD OnPlayerInfoHistoryFill(CXgFMPanel *screen) {
    UInt playerId = CallMethodAndReturn<Int, 0x5DE220>(screen); // CPlayerInfoPanel::GetCurrentPlayerId
    CDBPlayer *player = GetPlayer(playerId);
    if (player) {
        auto employee = GetManagerWhoLooksForPlayer(player->GetID());
        UChar knowledgeLevel = employee ? employee->GetPlayerKnowledge(player) : 10;
        if (!HidePlayerLevel()) {
            Bool display = knowledgeLevel == 10;
            auto ChkLevel = *raw_ptr<CXgCheckBox *>(screen, 0xBB8);
            auto TbColor3 = *raw_ptr<CXgTextBox *>(screen, 0xBB0);
            auto TbLev = *raw_ptr<CXgTextBox *>(screen, 0xBC0);
            TbColor3->SetVisible(display);
            ChkLevel->SetVisible(display);
            TbLev->SetVisible(display);
        }
        Bool displayMV = knowledgeLevel >= 7;
        auto ChkMarketValue = *raw_ptr<CXgCheckBox *>(screen, 0xBB4);
        auto TbColor1 = screen->GetTextBox("_tbColor1");
        auto TbMarketValue = *raw_ptr<CXgTextBox *>(screen, 0xBBC);
        TbColor1->SetVisible(displayMV);
        ChkMarketValue->SetVisible(displayMV);
        TbMarketValue->SetVisible(displayMV);
    }
    return playerId;
}

UChar METHOD ObClubTransfersGetPlayerLevel(CDBPlayer *player, DUMMY_ARG, UInt position, CDBEmployee *) {
    return player->GetLevel(position, GetManagerWhoLooksForPlayer(player->GetID()));
}

void METHOD OnClubTransfersAddColumnPlayerLevel(CFMListBox *listBox, DUMMY_ARG, Int64 value, UInt color, UInt unk) {
    if (HidePlayerLevel())
        listBox->AddColumnInt(value, color, unk);
    else
        listBox->AddPlayerLevel((Char)value, color, unk);
}

void PatchPlayerLevelPatches(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x5D6AF7, OnPlayerInfoCareerDrawLevelLine);
        patch::RedirectCall(0x5DD1CB, OnPlayerInfoHistoryDrawLevelLine);
        patch::RedirectCall(0x5DD195, OnPlayerInfoHistoryDrawMarketValueLine);
        patch::RedirectCall(0x5DD841, OnPlayerInfoHistoryFill);
        patch::RedirectCall(0x666494, ObClubTransfersGetPlayerLevel);
        patch::RedirectCall(0x66673B, ObClubTransfersGetPlayerLevel);
        patch::RedirectCall(0x666E41, ObClubTransfersGetPlayerLevel);
        patch::RedirectCall(0x66527F, ObClubTransfersGetPlayerLevel);
        patch::RedirectCall(0x667156, ObClubTransfersGetPlayerLevel);
        patch::SetUChar(0x667C6E + 1, LBT_PLAYER_LEVEL);
        patch::SetUChar(0x667CCA + 1, LBT_PLAYER_LEVEL);
        patch::SetUChar(0x667D2B + 1, LBT_PLAYER_LEVEL);
        patch::SetUChar(0x667D8C + 1, LBT_PLAYER_LEVEL);
        patch::SetUChar(0x667DE7 + 1, LBT_PLAYER_LEVEL);
        patch::RedirectCall(0x6664A1, OnClubTransfersAddColumnPlayerLevel);
        patch::RedirectCall(0x666748, OnClubTransfersAddColumnPlayerLevel);
        patch::RedirectCall(0x666E4E, OnClubTransfersAddColumnPlayerLevel);
        patch::RedirectCall(0x66528C, OnClubTransfersAddColumnPlayerLevel);
        patch::RedirectCall(0x667163, OnClubTransfersAddColumnPlayerLevel);
    }
}
