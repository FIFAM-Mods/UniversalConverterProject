#include "AbilityColors.h"
#include "GameInterfaces.h"
#include "FifamReadWrite.h"
#include "UcpSettings.h"
#include "InterfaceTheme.h"
#include "shared.h"

using namespace plugin;

struct AbilitiesColorSchema {
    String name;
    Map<UChar, UInt> values;
};

const UInt AbilityColorMarker = 0xABCDEF00;
const UInt OptionsGeneralDefaultSize = 0x580;

Vector<AbilitiesColorSchema> &GetPlayerAbilityColorSchemas() {
    static Vector<AbilitiesColorSchema> schemas;
    return schemas;
}

void ReadPlayerAbilityColorSchemas() {
    AbilitiesColorSchema schema;
    auto &schemas = GetPlayerAbilityColorSchemas();
    schemas.clear();
    FifamReader r("fmdata\\ParameterFiles\\Ability Colors.txt");
    if (r.Available()) {
        Map<String, UChar> guiColors = {
            { L"COL_BLANK", 0 },
            { L"COL_TEXT_STANDARD", 1 },
            { L"COL_TEXT_STANDARD_ACTIVE" , 2 },
            { L"COL_TEXT_STANDARD_INACTIVE" , 3 },
            { L"COL_TEXT_STANDARD_DISABLED" , 4 },
            { L"COL_TEXT_STANDARD_MOUSEOVER", 5 },
            { L"COL_TEXT_INVERTED", 6 },
            { L"COL_TEXT_INDICATOR_HIGHLIGHT" , 7 },
            { L"COL_TEXT_INDICATOR_LINK", 8 },
            { L"COL_TEXT_TICKER_STANDARD" , 9 },
            { L"COL_TEXT_TICKER_RSS", 10 },
            { L"COL_GEN_FINANCES_EARNINGS", 11 },
            { L"COL_GEN_FINANCES_EXPENDITURES", 12 },
            { L"COL_GEN_DISTINCT_1" , 13 },
            { L"COL_GEN_DISTINCT_2" , 14 },
            { L"COL_GEN_DISTINCT_3" , 15 },
            { L"COL_GEN_GRADUAL_VPOS" , 16 },
            { L"COL_GEN_GRADUAL_POS", 17 },
            { L"COL_GEN_GRADUAL_AVG", 18 },
            { L"COL_GEN_GRADUAL_NEG", 19 },
            { L"COL_GEN_GRADUAL_VNEG" , 20 },
            { L"COL_GEN_GRADUAL_PROGRESS" , 21 },
            { L"COL_GEN_CAPTIONCOLOR_1" , 22 },
            { L"COL_GEN_CAPTIONCOLOR_2" , 23 },
            { L"COL_GEN_CAPTIONCOLOR_3" , 24 },
            { L"COL_GEN_CAPTIONCOLOR_4" , 25 },
            { L"COL_GEN_CAPTIONCOLOR_5" , 26 },
            { L"COL_GEN_CAPTIONCOLOR_6" , 27 },
            { L"COL_GEN_CAPTIONCOLOR_7" , 28 },
            { L"COL_GEN_CAPTIONCOLOR_8" , 29 },
            { L"COL_GEN_CAPTIONCOLOR_9" , 30 },
            { L"COL_GEN_CAPTIONCOLOR_10", 31 },
            { L"COL_GEN_CAPTIONCOLOR_11", 32 },
            { L"COL_GEN_CAPTIONCOLOR_12", 33 },
            { L"COL_GEN_CAPTIONCOLOR_1T", 34 },
            { L"COL_GEN_CAPTIONCOLOR_2T", 35 },
            { L"COL_GEN_CAPTIONCOLOR_3T", 36 },
            { L"COL_GEN_CAPTIONCOLOR_4T", 37 },
            { L"COL_GEN_CAPTIONCOLOR_5T", 38 },
            { L"COL_GEN_CAPTIONCOLOR_6T", 39 },
            { L"COL_GEN_CAPTIONCOLOR_7T", 40 },
            { L"COL_GEN_CAPTIONCOLOR_8T", 41 },
            { L"COL_GEN_CAPTIONCOLOR_9T", 42 },
            { L"COL_GEN_CAPTIONCOLOR_10T" , 43 },
            { L"COL_GEN_CAPTIONCOLOR_11T" , 44 },
            { L"COL_GEN_CAPTIONCOLOR_12T" , 45 },
            { L"COL_BG_FILL_EMPTY", 46 },
            { L"COL_BG_FILL_STANDARD" , 47 },
            { L"COL_BG_FILL_ACTIVE" , 48 },
            { L"COL_BG_FILL_INACTIVE" , 49 },
            { L"COL_BG_FILL_DISABLED" , 50 },
            { L"COL_BG_FILL_MOUSEOVER", 51 },
            { L"COL_BG_TABLEPOS_CHAMPION" , 52 },
            { L"COL_BG_TABLEPOS_INTCOMP1ST" , 53 },
            { L"COL_BG_TABLEPOS_INTCOMP2ND" , 54 },
            { L"COL_BG_TABLEPOS_INTCOMP3RD" , 55 },
            { L"COL_BG_TABLEPOS_PROMOTION", 56 },
            { L"COL_BG_TABLEPOS_PROMOTIONMATCH" , 57 },
            { L"COL_BG_TABLEPOS_RELEGATIONMATCH", 58 },
            { L"COL_BG_TABLEPOS_RELEGATION" , 59 },
            { L"COL_BG_PLAYERSTATE_FIRST11" , 60 },
            { L"COL_BG_PLAYERSTATE_BENCH" , 61 },
            { L"COL_BG_PLAYERSTATE_NOTINSQUAD", 62 }
        };
        while (!r.IsEof()) {
            if (!r.EmptyLine()) {
                auto line = r.ReadFullLine();
                Utils::Trim(line);
                if (Utils::StartsWith(line, L"BEGIN")) {
                    line = line.substr(5);
                    Utils::Trim(line);
                    if (Utils::StartsWith(line, L"(") && Utils::EndsWith(line, L")")) {
                        line = line.substr(1, line.size() - 2);
                        Utils::Trim(line);
                        if (!line.empty()) {
                            schema.name = line;
                            schema.values.clear();
                        }
                    }
                }
                else if (line == L"END") {
                    if (!schema.name.empty() && !schema.values.empty()) {
                        schemas.push_back(schema);
                        schema.name.clear();
                        schema.values.clear();
                    }
                }
                else {
                    auto parts = Utils::Split(line, L',', true, false, false);
                    if ((parts.size() == 2 || parts.size() == 3) && Utils::IsNumber(parts[0])) {
                        UInt n = Utils::SafeConvertInt<UInt>(parts[0]);
                        if (n >= 1 && n <= 99) {
                            String colorValue = parts[(parts.size() == 3 && IsDarkTheme) ? 2 : 1];
                            if (Utils::IsNumber(colorValue, true))
                                schema.values[n] = Utils::SafeConvertInt<UInt>(colorValue, true);
                            else if (Utils::Contains(guiColors, colorValue))
                                schema.values[n] = AbilityColorMarker | guiColors[colorValue];
                        }
                    }
                }
            }
            else
                r.SkipLine();
        }
    }
    if (schemas.empty()) {
        AbilitiesColorSchema schema;
        schema.name = L"IDS_ABILITIES_SCHEMA_DEFAULT";
        schema.values[1] = AbilityColorMarker | COL_GEN_GRADUAL_VNEG;
        schema.values[11] = AbilityColorMarker | COL_GEN_GRADUAL_NEG;
        schema.values[31] = AbilityColorMarker | COL_TEXT_STANDARD;
        schema.values[80] = AbilityColorMarker | COL_GEN_GRADUAL_POS;
        schema.values[90] = AbilityColorMarker | COL_GEN_GRADUAL_VPOS;
    }
}

UInt GetCustomColorForAbility(Char abilityValue) {
    if (abilityValue >= 1 && abilityValue <= 99 && !GetPlayerAbilityColorSchemas().empty()) {
        AbilitiesColorSchema schema;
        Bool schemaFound = false;
        for (auto &i : GetPlayerAbilityColorSchemas()) {
            if (i.name == Settings::GetInstance().AbilitiesColorSchema) {
                schema = i;
                schemaFound = true;
                break;
            }
        }
        if (!schemaFound)
            schema = GetPlayerAbilityColorSchemas()[0];
        for (auto it = schema.values.rbegin(); it != schema.values.rend(); it++) {
            if (abilityValue >= (*it).first) {
                if (((*it).second & 0xFFFFFF00) == AbilityColorMarker)
                    return GetGuiColor((*it).second & 0xFF);
                else
                    return (*it).second;
            }
        }
    }
    return GetGuiColor(COL_TEXT_STANDARD);
}

UInt GetCustomColorForPlayerAbility(CDBPlayer *player, UShort ability, CDBEmployee *employee) {
    return GetCustomColorForAbility(player->GetAbility(ability, employee));
}

void SetListBoxColumnFont(CFMListBox *listBox, Int columnIndex, Char const *fontName) {
    if (columnIndex < listBox->GetNumColumns()) {
        for (Int i = 0; i < listBox->GetTotalRows(); i++) {
            CXgTextBox *textBox = listBox->GetCellTextBox(i, columnIndex);
            if (textBox)
                textBox->SetFont(fontName);
        }
    }
}

void METHOD OnPlayerInfoSkillsCreateUI(void *t) {
    CallMethod<0x5E0130>(t);
    if (Settings::GetInstance().AbilitiesBoldFont) {
        CFMListBox *listBoxes[4] = {
            raw_ptr<CFMListBox>(t, 1184),
            raw_ptr<CFMListBox>(t, 2980),
            raw_ptr<CFMListBox>(t, 4776),
            raw_ptr<CFMListBox>(t, 6572)
        };
        for (UInt i = 0; i < 4; i++)
            SetListBoxColumnFont(listBoxes[i], 5, "SubHeadline");
    }
}

void UpdateStaffAbilityColors(void *screen) {
    CXgTextBox **textBoxes = raw_ptr<CXgTextBox *>(screen, 0x56C);
    for (UInt i = 0; i < 8; i++) {
        String strValue = textBoxes[i]->GetText();
        SetTextBoxColorRGBA(textBoxes[i], Utils::IsNumber(strValue) ?
            GetCustomColorForAbility(Utils::SafeConvertInt<Char>(strValue)) :
            GetGuiColor(COL_TEXT_STANDARD));
        if (Settings::GetInstance().AbilitiesBoldFont)
            textBoxes[i]->SetFont("SubHeadline");
    }
}

struct OptionsGeneralExtension {
    CXgComboBox *CbAbilitiesColorSchema;
    CXgCheckBox *ChkAbilitiesBoldFont;
};

CXgCheckBox *METHOD OnOptionsGeneralSetupUI(CXgFMPanel *screen, DUMMY_ARG, Char const *name) {
    if (Settings::GetInstance().AbilitiesAutoReload)
        ReadPlayerAbilityColorSchemas();
    auto ext = raw_ptr<OptionsGeneralExtension>(screen, OptionsGeneralDefaultSize);
    ext->CbAbilitiesColorSchema = screen->GetComboBox("CbAbilitiesColorSchema");
    Int schemaIndex = 0;
    Int selectedIndex = -1;
    for (auto &[name, schema] : GetPlayerAbilityColorSchemas()) {
        if (name == Settings::GetInstance().AbilitiesColorSchema)
            selectedIndex = schemaIndex;
        String schemaTitle = Utils::StartsWith(name, L"IDS_") ? GetTranslation(Utils::WtoA(name).c_str()) : name;
        ext->CbAbilitiesColorSchema->AddItem(schemaTitle.c_str(), schemaIndex++);
    }
    ext->CbAbilitiesColorSchema->SetCurrentIndex(selectedIndex);
    ext->ChkAbilitiesBoldFont = screen->GetCheckBox("ChkAbilitiesBoldFont");
    ext->ChkAbilitiesBoldFont->SetIsChecked(Settings::GetInstance().AbilitiesBoldFont);
    return screen->GetCheckBox(name);
}

void METHOD OnOptionsGeneralProcessCheckBoxes(CXgFMPanel *screen, DUMMY_ARG, GuiMessage *msg, Int unk) {
    auto ext = raw_ptr<OptionsGeneralExtension>(screen, OptionsGeneralDefaultSize);
    if (msg->node == ext->ChkAbilitiesBoldFont->GetGuiNode()) {
        Settings::GetInstance().AbilitiesBoldFont = ext->ChkAbilitiesBoldFont->GetIsChecked();
        return;
    }
    CallMethod<0x7AFCF0>(screen, msg, unk);
}

void METHOD OnOptionsGeneralProcessComboBoxes(CXgFMPanel *screen, DUMMY_ARG, GuiMessage *msg, Int unk1, Int unk2) {
    auto ext = raw_ptr<OptionsGeneralExtension>(screen, OptionsGeneralDefaultSize);
    if (msg->node == ext->CbAbilitiesColorSchema->GetGuiNode()) {
        Int currentIndex = ext->CbAbilitiesColorSchema->GetCurrentIndex();
        if (currentIndex >= 0 && currentIndex < (Int)GetPlayerAbilityColorSchemas().size())
            Settings::GetInstance().AbilitiesColorSchema = GetPlayerAbilityColorSchemas()[currentIndex].name;
        return;
    }
    CallMethod<0x7AE640>(screen, msg, unk1, unk2);
}

void PatchAbilityColors(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        ReadPlayerAbilityColorSchemas();
        patch::RedirectJump(0x4E8CB0, GetCustomColorForPlayerAbility);
        patch::SetPointer(0x23D23BC, OnPlayerInfoSkillsCreateUI);

        const UInt OptionsGeneralNewSize = OptionsGeneralDefaultSize + sizeof(OptionsGeneralExtension);
        patch::SetUInt(0x573004 + 1, OptionsGeneralNewSize);
        patch::SetUInt(0x57300B + 1, OptionsGeneralNewSize);
        patch::RedirectCall(0x7AF893, OnOptionsGeneralSetupUI);
        patch::SetPointer(0x2400C9C, OnOptionsGeneralProcessCheckBoxes);
        patch::SetPointer(0x2400C88, OnOptionsGeneralProcessComboBoxes);
    }
}
