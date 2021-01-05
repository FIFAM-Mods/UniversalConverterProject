#pragma once
#include "EasyEdit.h"
#include "FifamTypes.h"
#include "FifamReadWrite.h"

using namespace plugin;

class UIntHex {
public:
    UInt value = 0;
    UIntHex(UInt const &rhs) { value = rhs; }
    void operator=(UInt const &rhs) { value = rhs; }
    operator UInt() const { return value; }
};

namespace std {
    template<> struct less<UIntHex> {
        bool operator() (const UIntHex& lhs, const UIntHex& rhs) const { return lhs.value < rhs.value; }
    };
}

UInt gCountryLoaderAddr = 0;
UInt gTeamLoaderAddr[5] = {};
UInt gPlayerLoaderAddr[3] = {};
UInt gCompLoaderAddr = 0;

void *gCountry = nullptr;
UInt gCountryId = 0;
void *gTeam = nullptr;
void *gPlayer = nullptr;
void *gLeague = nullptr;
UInt gCompId = 0;

UInt gNumCountryEdits = 0;
UInt gNumTeamEdits = 0;
UInt gNumPlayerEdits = 0;
UInt gNumLeagueEdits = 0;
UInt gNumCompEdits = 0;

struct PlayerAdditionalInformation {
    UInt mStarHeadId = 0;
    UChar mLevel = 0;
    Vector<Pair<UChar, UChar>> mAbilities;
};

Map<String, PlayerAdditionalInformation> &GetAdditionalPlayerInfoMap() {
    static Map<String, PlayerAdditionalInformation> m;
    return m;
}

static String GetPlayerId(void *player, bool empics) {
    UInt empicsId = 0;
    if (empics) {
        empicsId = *raw_ptr<UInt>(player, 0xE0);
        if (!empicsId)
            return String();
    }
    static WideChar str[260];
    Call<0x4DC530>(str,
        *raw_ptr<UChar>(player, 0x19C),
        *raw_ptr<UShort>(player, 0x19E),
        *raw_ptr<UShort>(player, 0x1A0),
        *raw_ptr<UShort>(player, 0xC6),
        1,
        *raw_ptr<UInt>(player, 0xE4),
        empicsId
        );
    return str;
}

Int METHOD CountryLoader(void *country, DUMMY_ARG, void *loader, Int countryId) {
    gCountry = country;
    gCountryId = countryId;
    Int result = CallMethodAndReturnDynGlobal<Int>(gCountryLoaderAddr, country, loader, countryId);
    gCountry = nullptr;
    gCountryId = 0;
    return result;
}

template<UInt id>
void METHOD TeamLoader(void *team, DUMMY_ARG, void *loader, Int countryId, Int index) {
    gTeam = team;
    CallMethodDynGlobal(gTeamLoaderAddr[id], team, loader, countryId, index);
    gTeam = nullptr;
}

Bool gPlayerStarheadEditUsed = false;
Bool gPlayerLevelEditUsed = false;
Bool gPlayerAbilityEditUsed = false;

template<UInt id>
void METHOD PlayerLoader(void *player, DUMMY_ARG, void *loader) {
    gPlayer = player;
    CallMethodDynGlobal(gPlayerLoaderAddr[id], player, loader);
    if (gPlayerStarheadEditUsed || gPlayerLevelEditUsed || gPlayerAbilityEditUsed) {
        auto it = GetAdditionalPlayerInfoMap().find(GetPlayerId(player, false));
        if (it == GetAdditionalPlayerInfoMap().end())
            it = GetAdditionalPlayerInfoMap().find(GetPlayerId(player, true));
        if (it != GetAdditionalPlayerInfoMap().end()) {
            if ((*it).second.mStarHeadId != 0 && exists("data\\assets\\" + Utils::Format("m228__%d.o", (*it).second.mStarHeadId)))
                *raw_ptr<UInt>(player, 0xDC) = (*it).second.mStarHeadId;
            if ((*it).second.mLevel > 0) {
                UChar desiredLevel = Utils::Clamp((*it).second.mLevel, 1, 99);
                UChar currLevel = CallMethodAndReturn<UChar, 0xFAAD60>(player, *raw_ptr<UChar>(player, 0x6F), false);
                Int levelDiff = desiredLevel - currLevel;
                if (levelDiff != 0) {
                    Int levelCorrection = 1;
                    Int levelIteration = levelDiff * 3;
                    if (levelDiff < 0) {
                        levelCorrection = -1;
                        levelIteration *= -1;
                    }
                    UChar *attr = raw_ptr<UChar>(player, 0x24);
                    do {
                        for (UInt i = 0; i < 37; i++) {
                            if (i < 28 || i > 30) {
                                if (i >= 31) {
                                    if (*raw_ptr<UChar>(player, 0x6F) == 1 || (levelCorrection == -1 && desiredLevel <= 20))
                                        attr[i] = Utils::Clamp((Int)attr[i] + levelCorrection, 1, 99);
                                }
                                else {
                                    if (*raw_ptr<UChar>(player, 0x6F) != 1 || (levelCorrection == -1 && desiredLevel <= 20))
                                        attr[i] = Utils::Clamp((Int)attr[i] + levelCorrection, 1, 99);
                                }
                            }
                        }
                        levelIteration--;
                        *raw_ptr<Int>(player, 0xF4) = -1;
                        currLevel = CallMethodAndReturn<UChar, 0xFAAD60>(player, *raw_ptr<UChar>(player, 0x6F), false);
                    } while (levelIteration && ((levelCorrection == -1 && currLevel > desiredLevel) || (levelCorrection == 1 && currLevel < desiredLevel)));
                }
            }
            if (!(*it).second.mAbilities.empty()) {
                UChar *attr = raw_ptr<UChar>(player, 0x24);
                for (auto const &ability : (*it).second.mAbilities) {
                    if (ability.first < 37)
                        attr[ability.first] = Utils::Clamp(ability.second, 1, 99);
                }
                *raw_ptr<Int>(player, 0xF4) = -1;
            }
        }
    }
    gPlayer = nullptr;
}

template<typename FieldType>
class DefaultReader {
public:
    using Type = FieldType;

    static void Read(FieldType *dst, FieldType const &src) {
        *dst = src;
    }

    static void ReadArray(FieldType *dst, UInt arySize, Vector<FieldType> const &src) {
        UInt size = Utils::Min(src.size(), arySize);
        if (size > 0)
            memcpy(dst, &src[0], size * sizeof(FieldType));
    }

    static void ReadString(WideChar *dst, UInt maxLen, String const &src) {
        UInt size = Utils::Min(src.size() + 1, maxLen);
        if (size > 0) {
            if (size > 1)
                memcpy(dst, src.c_str(), size * 2);
            dst[size - 1] = L'\0';
        }
    }

    static void ReadStringArray(WideChar *dst, UInt maxLen, Vector<String> const &src) {
        UInt size = Utils::Min(src.size(), 6u);
        if (size > 0) {
            if (size == 1) {
                for (UInt i = 0; i < 6; i++)
                    ReadString(&dst[maxLen * i * 2], maxLen, src[0]);
            }
            for (UInt i = 0; i < size; i++)
                ReadString(&dst[maxLen * i * 2], maxLen, src[i]);
        }
    }
};

template<typename FieldType>
using CustomReader = DefaultReader<FieldType>;

class CountryIdAccessor {
public:
    using Type = UInt;
    static const bool UsesSecondId = false;

    static Type GetId() {
        return gCountryId;
    }

    static Type GetSecondId() {
        return 0;
    }

    static Bool HasFirstId() {
        return gCountryId != 0;
    }

    static Bool HasSecondId() {
        return false;
    }
};

class TeamIdAccessor {
public:
    using Type = UIntHex;
    static const bool UsesSecondId = false;

    static Type GetId() {
        if (gTeam)
            return *raw_ptr<UIntHex>(gTeam, 0xF0);
        return 0;
    }

    static Type GetSecondId() {
        return 0;
    }

    static Bool HasFirstId() {
        return gTeam != nullptr;
    }

    static Bool HasSecondId() {
        return false;
    }
};

class PlayerIdAccessor {
public:
    using Type = String;
    static const bool UsesSecondId = true;

    static Type GetId() {
        if (gPlayer)
            return GetPlayerId(gPlayer, 0);
        return String();
    }

    static Type GetSecondId() {
        if (gPlayer)
            return GetPlayerId(gPlayer, 1);
        return String();
    }

    static Bool HasFirstId() {
        return gPlayer != nullptr;
    }

    static Bool HasSecondId() {
        return gPlayer && *raw_ptr<UInt>(gPlayer, 0xE0);
    }
};

class CompIdAccessor {
public:
    using Type = UIntHex;
    static const bool UsesSecondId = false;

    static Type GetId() {
        return gCompId;
    }

    static Type GetSecondId() {
        return 0;
    }

    static Bool HasFirstId() {
        return gCompId != 0;
    }

    static Bool HasSecondId() {
        return false;
    }
};

class EmployeeIdAccessor {
public:
    using Type = String;
    static const bool UsesSecondId = false;

    static Type GetId() {
        if (gPlayer)
            return GetPlayerId(gPlayer, 0);
        return String();
    }

    static Type GetSecondId() {
        return String();
    }
};

template<typename ToType>
ToType ConvertStringParameter(String const &idStr) {
    return Utils::SafeConvertInt<ToType>(idStr);
}

template<>
String ConvertStringParameter<String>(String const &idStr) {
    return idStr;
}

template<>
UIntHex ConvertStringParameter<UIntHex>(String const &idStr) {
    return Utils::SafeConvertInt<UInt>(idStr, true);
}

class EditorBase {
public:
    virtual void RegisterEdit(Vector<String> const& params) = 0;
};

template<typename IdAccessor, UInt ReadAddr, typename FieldType, Bool IsArray = false, typename Reader = DefaultReader<FieldType>>
class Editor : public EditorBase {
    inline static UInt DefaultFunc;

    static Map<typename IdAccessor::Type, FieldType> &GetMap() {
        static Map<typename IdAccessor::Type, FieldType> m;
        return m;
    }

    static Map<typename IdAccessor::Type, Vector<FieldType>> &GetMapArray() {
        static Map<typename IdAccessor::Type, Vector<FieldType>> m;
        return m;
    }

    template<typename GetMapFunc, typename FuncType, typename ...ArgTypes>
    static void Read(GetMapFunc getMapFunc, FuncType func, void *loader, ArgTypes... args) {
        CallMethodDynGlobal(DefaultFunc, loader, args...);
        if (IdAccessor::HasFirstId()) {
            auto it = getMapFunc().find(IdAccessor::GetId());
            if (it != getMapFunc().end()) {
                func(args..., (*it).second);
                return;
            }
        }
        if constexpr (IdAccessor::UsesSecondId) {
            if (IdAccessor::HasSecondId()) {
                auto it2 = getMapFunc().find(IdAccessor::GetSecondId());
                if (it2 != getMapFunc().end())
                    func(args..., (*it2).second);
            }
        }
    }

    static void METHOD ReadHandler(void *loader, DUMMY_ARG, FieldType *out) {
        Read(&GetMap, &Reader::Read, loader, out);
    }

    static void METHOD ReadArrayHandler(void *loader, DUMMY_ARG, FieldType *out, UInt size) {
        Read(&GetMapArray, &Reader::ReadArray, loader, out, size);
    }

    static void METHOD ReadStringHandler(void *loader, DUMMY_ARG, WideChar *out, UInt maxLen) {
        Read(&GetMap, &Reader::ReadString, loader, out, maxLen);
    }

    static void METHOD ReadStringArrayHandler(void *loader, DUMMY_ARG, WideChar *out, UInt maxLen) {
        Read(&GetMapArray, &Reader::ReadStringArray, loader, out, maxLen);
    }

    bool mInstalled = false;
public:
    void RegisterEdit(Vector<String> const& params) override {
        if (!mInstalled) {
            if constexpr (IsArray == true) {
                if constexpr (std::is_same_v<FieldType, String>)
                    DefaultFunc = patch::RedirectCall(ReadAddr, ReadStringArrayHandler);
                else
                    DefaultFunc = patch::RedirectCall(ReadAddr, ReadArrayHandler);
            }
            else if constexpr (std::is_same_v<FieldType, String>)
                DefaultFunc = patch::RedirectCall(ReadAddr, ReadStringHandler);
            else
                DefaultFunc = patch::RedirectCall(ReadAddr, ReadHandler);
            mInstalled = true;
        }
        if constexpr (IsArray == true) {
            auto &p = GetMapArray()[ConvertStringParameter<typename IdAccessor::Type>(params[1])];
            p.resize(params.size() - 2);
            for (UInt i = 0; i < (params.size() - 2); i++)
                p[i] = ConvertStringParameter<FieldType>(params[i + 2]);
        }
        else
            GetMap()[ConvertStringParameter<typename IdAccessor::Type>(params[1])] = ConvertStringParameter<FieldType>(params[2]);
        if constexpr (std::is_same_v<IdAccessor, CountryIdAccessor>)
            gNumCountryEdits++;
        else if constexpr (std::is_same_v<IdAccessor, TeamIdAccessor>)
            gNumTeamEdits++;
        else if constexpr (std::is_same_v<IdAccessor, PlayerIdAccessor>)
            gNumPlayerEdits++;
    }
};

template<unsigned int ReadAddr, typename FieldType, bool IsArray = false, typename Reader = DefaultReader<FieldType>>
class CountryEditor : public Editor<CountryIdAccessor, ReadAddr, FieldType, IsArray, Reader> {
    ~CountryEditor() {
        GetMap().clear();
        GetMapArray().clear();
    }
};

template<unsigned int ReadAddr, typename FieldType, bool IsArray = false, typename Reader = DefaultReader<FieldType>>
class TeamEditor : public Editor<TeamIdAccessor, ReadAddr, FieldType, IsArray, Reader> {
    ~TeamEditor() {
        GetMap().clear();
        GetMapArray().clear();
    }
};

template<unsigned int ReadAddr, typename FieldType, bool IsArray = false, typename Reader = DefaultReader<FieldType>>
class PlayerEditor : public Editor<PlayerIdAccessor, ReadAddr, FieldType, IsArray, Reader> {
    ~PlayerEditor() {
        GetMap().clear();
        GetMapArray().clear();
    }
};

template<unsigned int ReadAddr, typename FieldType, bool IsArray = false, typename Reader = DefaultReader<FieldType>>
class CompEditor : public Editor<CompIdAccessor, ReadAddr, FieldType, IsArray, Reader> {
    ~CompEditor() {
        GetMap().clear();
        GetMapArray().clear();
    }
};

class EditorManager {
public:
    Map<String, EditorBase *> editors;

    template<typename T>
    void RegisterEditor(String const &name) {
        auto it = editors.find(name);
        if (it != editors.end())
            delete (*it).second;
        editors[name] = new T;
    }

    EditorBase *FindEditor(String const &name) {
        auto it = editors.find(name);
        if (it != editors.end())
            return (*it).second;
        return nullptr;
    }

    ~EditorManager() {
        for (auto[name, editor] : editors)
            delete editor;
    }
};

template<typename T, T Min, T Max>
class MinMaxReader : public CustomReader<T> {
public:
    static void Read(Type *dst, Type const &src) {
        *dst = Utils::Clamp(src, Min, Max);
    }
};

template<typename T, T Min, T Max>
class MinMaxArrayReader : public CustomReader<T> {
public:
    static void ReadArray(Type *dst, UInt arySize, Vector<Type> const &src) {
        UInt size = Utils::Min(arySize, src.size());
        for (UInt i = 0; i < size; i++) {
            if (src[i] >= 0)
                dst[i] = Utils::Clamp(src[i], Min, Max);
        }
    }
};

class RichGuyControlledReader : public CustomReader<UChar> {
public:
    static void Read(Type *dst, Type const &src) {
        if (src != 0)
            *dst |= 0x400000;
    }
};

using MoneyReader = MinMaxReader<Int, -2'000'000'000, 2'000'000'000>;
using MoneyPositiveReader = MinMaxReader<Int, 0, 2'000'000'000>;
using BoolReader = MinMaxReader<UChar, 0, 1>;
using Bool32Reader = MinMaxReader<UInt, 0, 1>;
using RatingReader = MinMaxReader<UChar, 1, 99>;
using RatingArrayReader = MinMaxArrayReader<Char, -1, 99>;

void InstallEasyEdit(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {

        static EditorManager edManager;

        edManager.RegisterEditor<CountryEditor<0xFF1E1A, UChar, false, MinMaxReader<UChar, 0, 9>>>(L"SET_COUNTRY_YEARS_FOR_NATURALIZATION");
        edManager.RegisterEditor<CountryEditor<0xFF1E88, UChar, false, RatingReader>>(L"SET_COUNTRY_PLAYERS_AVERAGE_RATING");
        edManager.RegisterEditor<CountryEditor<0xFF1DF2, Char, true, RatingArrayReader>>(L"SET_COUNTRY_PLAYERS_LEAGUE_RATING");

        edManager.RegisterEditor<TeamEditor<0xF32D54, String>>(L"SET_TEAM_NAME");
        edManager.RegisterEditor<TeamEditor<0xF32D64, String>>(L"SET_TEAM_SHORT_NAME");
        edManager.RegisterEditor<TeamEditor<0xF3304B, String>>(L"SET_TEAM_ABBREVIATION");
        edManager.RegisterEditor<TeamEditor<0xF3330E, Char, true, MinMaxArrayReader<Char, -1, 100>>>(L"SET_TEAM_COLORS");
        edManager.RegisterEditor<TeamEditor<0xF33330, Char, false, MinMaxReader<Char, -100, -1>>>(L"SET_TEAM_START_POINTS_PENALTY");
        edManager.RegisterEditor<TeamEditor<0xF333A9, Int, false, MoneyReader>>(L"SET_TEAM_BALANCE");
        edManager.RegisterEditor<TeamEditor<0xF333D5, Int, false, MoneyPositiveReader>>(L"SET_TEAM_TRANSFERS_BUDGET");
        edManager.RegisterEditor<TeamEditor<0xF3345E, UChar, false, BoolReader>>(L"SET_TEAM_JOINT_STOCK_COMPANY");
        edManager.RegisterEditor<TeamEditor<0xF334A7, Int, false, MoneyPositiveReader>>(L"SET_TEAM_SPONSOR_AMOUNT");
        edManager.RegisterEditor<TeamEditor<0xF334B3, UChar, false, MinMaxReader<UChar, 1, 100>>>(L"SET_TEAM_SPONSOR_YEARS");
        edManager.RegisterEditor<TeamEditor<0xF334BA, UChar, false, BoolReader>>(L"SET_TEAM_SPONSOR_SPECIAL");
        edManager.RegisterEditor<TeamEditor<0xF33622, UChar, false, MinMaxReader<UChar, 0, 4>>>(L"SET_TEAM_MEDIA_PRESSURE");
        edManager.RegisterEditor<TeamEditor<0xF33648, Int, false, MoneyPositiveReader>>(L"SET_TEAM_INDIVIDUAL_TV_CONTRACT");
        edManager.RegisterEditor<TeamEditor<0xF33684, UInt, false, Bool32Reader>>(L"SET_TEAM_RICH_GUY_CONTROLLED");
        edManager.RegisterEditor<TeamEditor<0xF33909, UChar, false, MinMaxReader<UChar, 0, 20>>>(L"SET_TEAM_NATIONAL_PRESTIGE");
        edManager.RegisterEditor<TeamEditor<0xF33917, UChar, false, MinMaxReader<UChar, 0, 20>>>(L"SET_TEAM_INTERNATIONAL_PRESTIGE");
        edManager.RegisterEditor<TeamEditor<0xF33E09, UInt, false, MinMaxReader<UInt, 0, 1'000'000>>>(L"SET_TEAM_FAN_BASE");
        edManager.RegisterEditor<TeamEditor<0xF33E17, UInt, false, MinMaxReader<UInt, 0, 250'000>>>(L"SET_TEAM_AVERAGE_ATTENDANCE");
        edManager.RegisterEditor<TeamEditor<0xF33E25, UInt, false, MinMaxReader<UInt, 0, 250'000>>>(L"SET_TEAM_TICKETS_SOLD");
        edManager.RegisterEditor<TeamEditor<0xF33E33, UInt, false, MinMaxReader<UInt, 0, 1'000'000>>>(L"SET_TEAM_MEMBERS");
        edManager.RegisterEditor<TeamEditor<0xF33E54, UInt, false, MinMaxReader<UInt, 0, 20>>>(L"SET_TEAM_PREFERRED_FORMATION");
        edManager.RegisterEditor<TeamEditor<0xF33E63, UInt, false, MinMaxReader<UInt, 0, 20>>>(L"SET_TEAM_SECONDARY_FORMATION");
        edManager.RegisterEditor<TeamEditor<0xF33F8E, UChar, true, MinMaxArrayReader<UChar, 1, 207>>>(L"SET_TEAM_TRANSFER_COUNTRIES");
        edManager.RegisterEditor<TeamEditor<0xF80B5C, String>>(L"SET_TEAM_STADIUM_NAME");
        edManager.RegisterEditor<TeamEditor<0xF80AF5, UInt, true, MinMaxArrayReader<UInt, 0, 250'000>>>(L"SET_TEAM_STADIUM_CAPACITY");
        //edManager.RegisterEditor<TeamEditor<0xF80AC7, UInt>>(L"SET_TEAM_VENUE_ID");
        edManager.RegisterEditor<TeamEditor<0xFFE202, UInt>>(L"SET_TEAM_FIFA_KIT_ID");
        edManager.RegisterEditor<TeamEditor<0xFFE310, UChar, true, MinMaxArrayReader<UChar, 0, 20>>>(L"SET_TEAM_KIT_NUMBER_COLORS");
        edManager.RegisterEditor<TeamEditor<0xF3404C, UChar, false, MinMaxReader<UChar, 0, 6>>>(L"SET_TEAM_CLUB_FACILITIES");
        edManager.RegisterEditor<TeamEditor<0xF34058, UChar, false, MinMaxReader<UChar, 1, 10>>>(L"SET_TEAM_YOUTH_CENTRE");
        edManager.RegisterEditor<TeamEditor<0xF34064, UChar, false, MinMaxReader<UChar, 0, 10>>>(L"SET_TEAM_YOUTH_BOARDING_SCHOOL");
        edManager.RegisterEditor<TeamEditor<0xF34070, UChar, false, MinMaxReader<UChar, 0, 6>>>(L"SET_TEAM_AI_STRATEGY");
        edManager.RegisterEditor<TeamEditor<0xF3407C, UChar, false, MinMaxReader<UChar, 0, 4>>>(L"SET_TEAM_LANDSCAPE");
        edManager.RegisterEditor<TeamEditor<0xF34088, UChar, false, MinMaxReader<UChar, 0, 2>>>(L"SET_TEAM_SETTLEMENT");

        edManager.RegisterEditor<PlayerEditor<0xFD1708, UChar, false, MinMaxReader<UChar, 0, 9>>>(L"SET_PLAYER_TALENT");
        edManager.RegisterEditor<PlayerEditor<0xFD1765, Char, true, RatingArrayReader>>(L"SET_PLAYER_ABILITIES");
        edManager.RegisterEditor<PlayerEditor<0xFD2CAB, Char, true, MinMaxArrayReader<Char, -1, 100>>>(L"SET_PLAYER_POSITIONS_BIAS");
        edManager.RegisterEditor<PlayerEditor<0xFD2CB3, UChar, false, MinMaxReader<UChar, 1, 13>>>(L"SET_PLAYER_MAIN_POSITION");
        edManager.RegisterEditor<PlayerEditor<0xFD2FC7, UChar, false, MinMaxReader<UChar, 1, 10>>>(L"SET_PLAYER_LEADERSHIP");
        edManager.RegisterEditor<PlayerEditor<0xFD3018, UChar, false, MinMaxReader<UChar, 0, 13>>>(L"SET_PLAYER_AGENT");
        edManager.RegisterEditor<PlayerEditor<0xFD3023, UChar, false, MinMaxReader<UChar, 0, 23>>>(L"SET_PLAYER_STYLE");
        edManager.RegisterEditor<PlayerEditor<0xFD3046, UInt64>>(L"SET_PLAYER_CHARACTER_FLAGS");
        edManager.RegisterEditor<PlayerEditor<0xFD3175, UChar, false, MinMaxReader<UChar, 150, 220>>>(L"SET_PLAYER_HEIGHT");
        edManager.RegisterEditor<PlayerEditor<0xFD3183, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_WEIGHT");
        edManager.RegisterEditor<PlayerEditor<0xFD3191, UChar, false, MinMaxReader<UChar, 1, 19>>>(L"SET_PLAYER_SHOE");
        edManager.RegisterEditor<PlayerEditor<0xFD319D, UChar, false, MinMaxReader<UChar, 0, 10>>>(L"SET_PLAYER_HERO_STATUS");
        edManager.RegisterEditor<PlayerEditor<0xFD31A9, UChar, false, MinMaxReader<UChar, 0, 18>>>(L"SET_PLAYER_EXPERIENCE");
        edManager.RegisterEditor<PlayerEditor<0xEA172A, UShort, false, MinMaxReader<UShort, 50, 150>>>(L"SET_PLAYER_GENERIC_FACE");
        edManager.RegisterEditor<PlayerEditor<0xEA1734, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_GENERIC_HAIR");
        edManager.RegisterEditor<PlayerEditor<0xEA173E, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_GENERIC_HAIR_COLOR");
        edManager.RegisterEditor<PlayerEditor<0xEA1754, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_GENERIC_BEARD");
        edManager.RegisterEditor<PlayerEditor<0xEA175E, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_GENERIC_SIDEBURNS");
        edManager.RegisterEditor<PlayerEditor<0xEA1771, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_GENERIC_BEARD_COLOR");
        edManager.RegisterEditor<PlayerEditor<0xEA1784, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_GENERIC_SKIN_COLOR");
        edManager.RegisterEditor<PlayerEditor<0xEA1797, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_GENERIC_FACE_VARIATION");
        edManager.RegisterEditor<PlayerEditor<0xEA17AD, UChar, false, MinMaxReader<UChar, 50, 150>>>(L"SET_PLAYER_GENERIC_EYE_COLOR");

        auto ProcessEditFile = [](Path const &p) {
            if (is_regular_file(p) && (p.extension().string() == ".dbe" || p.extension().string() == ".DBE")) {
                FifamReader reader(p, 14);
                if (reader.Available()) {
                    while (!reader.IsEof()) {
                        if (!reader.EmptyLine()) {
                            auto line = reader.ReadFullLine();
                            auto parts = Utils::Split(line, L' ', true, true);
                            if (parts.size() < 3)
                                continue;
                            if (parts[0] == L"SET_PLAYER_STARHEAD_ID") {
                                GetAdditionalPlayerInfoMap()[parts[1]].mStarHeadId = Utils::SafeConvertInt<UInt>(parts[2]);
                                gPlayerStarheadEditUsed = true;
                                gNumPlayerEdits++;
                            }
                            else if (parts[0] == L"SET_PLAYER_LEVEL") {
                                GetAdditionalPlayerInfoMap()[parts[1]].mLevel = Utils::SafeConvertInt<UChar>(parts[2]);
                                gPlayerLevelEditUsed = true;
                                gNumPlayerEdits++;
                            }
                            else if (parts.size() >= 4 && parts[0] == L"SET_PLAYER_ABILITY") {
                                GetAdditionalPlayerInfoMap()[parts[1]].mAbilities.emplace_back(Utils::SafeConvertInt<UChar>(parts[2]), Utils::SafeConvertInt<UChar>(parts[3]));
                                gPlayerAbilityEditUsed = true;
                                gNumPlayerEdits++;
                            }
                            else {
                                EditorBase *editor = edManager.FindEditor(parts[0]);
                                if (editor)
                                    editor->RegisterEdit(parts);
                            }
                        }
                        else
                            reader.SkipLine();
                    }
                }
            }
        };

        path editsDir = L"edits";

        if (exists(editsDir)) {
            // KERNELBASE error: directory_iterator now throws an exception if folder does not exist
            std::error_code ec;
            for (auto const &p : recursive_directory_iterator(editsDir, ec))
                ProcessEditFile(p.path());

            if (gNumCountryEdits)
                gCountryLoaderAddr = patch::RedirectCall(0xF974A3, CountryLoader);
            if (gNumTeamEdits) {
                gTeamLoaderAddr[0] = patch::RedirectCall(0xFF2272, TeamLoader<0>);
                gTeamLoaderAddr[1] = patch::RedirectCall(0xFF24C4, TeamLoader<1>);
                gTeamLoaderAddr[2] = patch::RedirectCall(0xFF2904, TeamLoader<2>);
                gTeamLoaderAddr[3] = patch::RedirectCall(0xFF2A1A, TeamLoader<3>);
                gTeamLoaderAddr[4] = patch::RedirectCall(0xFF2AC8, TeamLoader<4>);
            }
            if (gNumPlayerEdits) {
                gPlayerLoaderAddr[0] = patch::RedirectCall(0xF33A0F, PlayerLoader<0>);
                gPlayerLoaderAddr[1] = patch::RedirectCall(0xF87E8A, PlayerLoader<1>);
                gPlayerLoaderAddr[2] = patch::RedirectCall(0xF975DF, PlayerLoader<2>);
            }
            if (gNumCompEdits) {
            
            }
        }
    }
}
