#include "PlayerMarks.h"
#include "UcpSettings.h"
#include "Utils.h"

using namespace plugin;

void GetPlayerMarksTypeAndFormat(Int metric, Int &type, Int &format) {
    type = Settings::GetInstance().PlayerMarksType;
    format = Settings::GetInstance().PlayerMarksFormat;
    if (type == PLAYERMARK_TYPE_DEFAULT) {
        switch (metric) {
        case METRIC_FRANCE:
            type = PLAYERMARK_TYPE_FRENCH;
            break;
        case METRIC_SPAIN:
            type = PLAYERMARK_TYPE_SPANISH;
            break;
        case METRIC_ITALY:
        case METRIC_UNITED_KINGDOM:
            type = PLAYERMARK_TYPE_ENGLISH;
            break;
        case METRIC_POLAND:
            type = PLAYERMARK_TYPE_POLISH;
            break;
        default:
            type = PLAYERMARK_TYPE_GERMAN;
            break;
        }
    }
    if (format == PLAYERMARK_FORMAT_DEFAULT) {
        switch (type) {
        case PLAYERMARK_TYPE_GERMAN:
        case PLAYERMARK_TYPE_UNITED_STATES:
            format = PLAYERMARK_FORMAT_FLOAT;
            break;
        default:
            format = PLAYERMARK_FORMAT_INTEGER;
            break;
        }
    }
}

Float MyGetPlayerMarkForMetric(Float value, Int metric, Bool bUseQuarterSteps) {
    Int type = PLAYERMARK_TYPE_DEFAULT;
    Int format = PLAYERMARK_FORMAT_DEFAULT;
    GetPlayerMarksTypeAndFormat(metric, type, format);
    Float result = 0.0f;
    switch (type) {
    case PLAYERMARK_TYPE_FRENCH:
        result = value * (13.0f / 20.0f) + 5.0f;
        break;
    case PLAYERMARK_TYPE_SPANISH:
        result = value * (3.0f / 20.0f);
        break;
    case PLAYERMARK_TYPE_ENGLISH:
    {
        Int clamped = (Int)std::clamp(value, 0.0f, 20.0f);
        static Float GradeTable[] =  { 3.0f, 4.0f, 4.0f, 5.0f, 5.0f, 5.0f, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 8.0f, 8.0f, 9.0f, 10.0f };
        static Float GradeTable2[] = { 3.0f, 3.5f, 4.0f, 4.5f, 5.0f, 5.5f, 5.5f, 6.0f, 6.0f, 6.0f, 6.5f, 6.5f, 7.0f, 7.0f, 7.5f, 7.5f, 8.0f, 8.5f, 9.0f, 9.5f, 10.0f };
        return (format == PLAYERMARK_FORMAT_FLOAT) ? GradeTable2[clamped] : GradeTable[clamped];
    }
    case PLAYERMARK_TYPE_POLISH:
        result = value / 4.0f + 1.0f;
        break;
    case PLAYERMARK_TYPE_GERMAN:
    case PLAYERMARK_TYPE_UNITED_STATES:
        result = 6.0f - value / 4.0f;
        break;
    case PLAYERMARK_TYPE_ALT1:
        result = value / 5.0f + 1.0f;
        break;
    case PLAYERMARK_TYPE_ALT2:
        result = value * 0.45f + 1.0f;
        break;
    case PLAYERMARK_TYPE_ALT3:
        result = value;
        break;
    }
    if (format == PLAYERMARK_FORMAT_FLOAT) {
        Float base = std::floor(result);
        Float fraction = result - base;
        if (bUseQuarterSteps)
            fraction = std::floor(fraction * 5.0f) / 4.0f;
        else
            fraction = std::round(fraction * 2.0f) / 2.0f;
        result = base + fraction;
    }
    return result;
}

void PlayerMarks_CreateUI(CXgFMPanel *screen, OptionsGeneral_PlayerMarksExtension *ext) {
    ext->CbPlayerMarksType = screen->GetComboBox("CbPlayerMarksType");
    ext->CbPlayerMarksType->AddItem(GetTranslation("IDS_PLAYERMARKS_DEFAULT"), PLAYERMARK_TYPE_DEFAULT);
    for (UInt i = 1; i < PLAYERMARK_COUNT; i++)
        ext->CbPlayerMarksType->AddItem(GetTranslation(Utils::Format("IDS_PLAYERMARKS_%u", i).c_str()), i);
    ext->CbPlayerMarksType->SetCurrentValue(Settings::GetInstance().PlayerMarksType);
    ext->CbPlayerMarksFormat = screen->GetComboBox("CbPlayerMarksFormat");
    ext->CbPlayerMarksFormat->AddItem(GetTranslation("IDS_PLAYERMARKS_DEFAULT"), PLAYERMARK_FORMAT_DEFAULT);
    ext->CbPlayerMarksFormat->AddItem(GetTranslation("IDS_PLAYERMARKS_INTEGER"), PLAYERMARK_FORMAT_INTEGER);
    ext->CbPlayerMarksFormat->AddItem(GetTranslation("IDS_PLAYERMARKS_FLOAT"), PLAYERMARK_FORMAT_FLOAT);
    ext->CbPlayerMarksFormat->SetCurrentValue(Settings::GetInstance().PlayerMarksFormat);
}

Bool PlayerMarks_ProcessComboBoxes(OptionsGeneral_PlayerMarksExtension *ext, GuiMessage *msg) {
    if (msg->node == ext->CbPlayerMarksType->GetGuiNode()) {
        Settings::GetInstance().PlayerMarksType = (Int)ext->CbPlayerMarksType->GetCurrentValue(PLAYERMARK_TYPE_DEFAULT);
        ext->CbPlayerMarksFormat->SetEnabled(Settings::GetInstance().PlayerMarksType != PLAYERMARK_TYPE_UNITED_STATES);
        return true;
    }
    if (msg->node == ext->CbPlayerMarksFormat->GetGuiNode()) {
        Settings::GetInstance().PlayerMarksFormat = (Int)ext->CbPlayerMarksFormat->GetCurrentValue(PLAYERMARK_FORMAT_DEFAULT);
        return true;
    }
    return false;
}

UInt PlayerMarks_IsUSType() {
    return (Settings::GetInstance().PlayerMarksType == PLAYERMARK_TYPE_UNITED_STATES) ? 1 : 0;
}

UInt PlayerMarks_IsFormatString() {
    if (PlayerMarks_IsUSType())
        return METRIC_GERMANY;
    Int type = PLAYERMARK_TYPE_DEFAULT;
    Int format = PLAYERMARK_FORMAT_DEFAULT;
    GetPlayerMarksTypeAndFormat(GetCurrentMetric(), type, format);
    return (format == PLAYERMARK_FORMAT_FLOAT) ? METRIC_GERMANY : 0;
}

Bool32 PlayerMarks_RetFalse() {
    return false;
}

void PatchPlayerMarks(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectJump(0x14AE5F0, MyGetPlayerMarkForMetric);
        patch::RedirectCall(0x14AE8F5, PlayerMarks_IsUSType);
        patch::RedirectCall(0x14AE92C, PlayerMarks_IsFormatString);
        patch::RedirectCall(0x82952F, PlayerMarks_IsFormatString);
        patch::RedirectCall(0x986949, PlayerMarks_IsFormatString);
        patch::RedirectCall(0x9877F0, PlayerMarks_IsFormatString);
        patch::RedirectCall(0xAB2BF4, PlayerMarks_IsFormatString);
        patch::RedirectCall(0xAD458F, PlayerMarks_IsFormatString);
        patch::RedirectCall(0xAD660A, PlayerMarks_IsFormatString);
        patch::RedirectCall(0xAD7B9B, PlayerMarks_IsFormatString);
        patch::RedirectCall(0xAD8CEC, PlayerMarks_IsFormatString);
        patch::RedirectCall(0xAD9167, PlayerMarks_IsFormatString);
        patch::RedirectCall(0x5E59D8, PlayerMarks_RetFalse);
    }
}
