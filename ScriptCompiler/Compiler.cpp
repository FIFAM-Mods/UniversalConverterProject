#include "Compiler.h"
#include "FifamDatabase.h"
#include "FifamCompPool.h"
#include "FifamCompRound.h"
#include "FifamCompLeague.h"
#include "FifamCompCup.h"
#include "FifamCompRoot.h"
#include "Utils.h"
#include <stack>
#include <algorithm>

enum eTokenType {
    TOKEN_INVALID,
    TOKEN_NAME,
    TOKEN_NUMBER,
    TOKEN_NUMBER_2D,
    TOKEN_STRING,
    TOKEN_ARRAY,
    TOKEN_REFERENCE,
    TOKEN_DIRECT_REFERENCE,
    TOKEN_PREDECESSOR,
    TOKEN_SUCCESSOR,
    TOKEN_PREDECESSOR_WEAK,
    TOKEN_SUCCESSOR_WEAK,
    TOKEN_OPEN,
    TOKEN_CLOSE,
    TOKEN_PARAMS_OPEN,
    TOKEN_PARAMS_CLOSE
};

struct TokenBase abstract {
    virtual eTokenType type() = 0;
    virtual ~TokenBase() {}
    virtual String print() = 0;
    virtual String printFull() = 0;
    virtual TokenBase *copy() = 0;
    template<typename T> T *as() {
        return reinterpret_cast<T *>(this);
    }
};
struct TokenValue : public TokenBase {};
struct TokenPredecessor : public TokenBase {
    Bool isWeak = false;
    virtual eTokenType type() { return TOKEN_PREDECESSOR; }
    String print() { return L"<-"; }
    String printFull() { return L"<- (PREDECESSOR)"; }
    TokenPredecessor(Bool weak = false) : isWeak(weak) {}
    TokenPredecessor *copy() { return new TokenPredecessor(isWeak); }
};
struct TokenSuccessor : public TokenBase {
    Bool isWeak = false;
    virtual eTokenType type() { return TOKEN_SUCCESSOR; }
    String print() { return L"->"; }
    String printFull() { return L"-> (SUCCESSOR)"; }
    TokenSuccessor(Bool weak = false) : isWeak(weak) {}
    TokenSuccessor *copy() { return new TokenSuccessor(isWeak); }
};
struct TokenOpen : public TokenBase {
    virtual eTokenType type() { return TOKEN_OPEN; }
    String print() { return L"{"; }
    String printFull() { return L"{ (OPEN)"; }
    TokenOpen *copy() { return new TokenOpen(); }
};
struct TokenClose : public TokenBase {
    virtual eTokenType type() { return TOKEN_CLOSE; }
    String print() { return L"}"; }
    String printFull() { return L"} (CLOSE)"; }
    TokenClose *copy() { return new TokenClose(); }
};
struct TokenString : public TokenValue {
    String str;
    virtual eTokenType type() { return TOKEN_STRING; }
    TokenString(String const &_str) : str(_str) {}
    String print() { return L"\"" + str + L"\""; }
    String printFull() { return L"\"" + str + L"\" (STRING)"; }
    TokenString *copy() { return new TokenString(str); }
};
struct TokenReference : public TokenString {
    virtual eTokenType type() { return TOKEN_REFERENCE; }
    String print() { return L"@" + str; }
    String printFull() { return L"@" + str + L" (REFERENCE)"; }
    TokenReference(String const &Name) : TokenString(Name) {}
    TokenReference *copy() { return new TokenReference(str); }
};
struct TokenDirectReference : public TokenString {
    virtual eTokenType type() { return TOKEN_DIRECT_REFERENCE; }
    String print() { return L"@@" + str; }
    String printFull() { return L"@@" + str + L" (DIRECT_REFERENCE)"; }
    TokenDirectReference(String const &Name) : TokenString(Name) {}
    TokenDirectReference *copy() { return new TokenDirectReference(str); }
};
struct TokenNumber : public TokenValue {
    Int number;
    virtual eTokenType type() { return TOKEN_NUMBER; }
    String print() { return Utils::Format(L"%d", number); }
    String printFull() { return Utils::Format(L"%d", number) + L" (NUMBER)"; }
    TokenNumber(Int n) : number(n) {}
    TokenNumber *copy() { return new TokenNumber(number); }
};
struct TokenNumber2D : public TokenNumber {
    Int number2;
    virtual eTokenType type() { return TOKEN_NUMBER_2D; }
    String print() { return Utils::Format(L"%d:%d", number, number2); }
    String printFull() { return Utils::Format(L"%d:%d", number, number2) + L" (NUMBER_2D)"; }
    TokenNumber2D(Int n1, Int n2) : TokenNumber(n1) { number2 = n2; }
    TokenNumber2D *copy() { return new TokenNumber2D(number, number2); }
};
struct TokenName : public TokenBase {
    String name;
    virtual eTokenType type() { return TOKEN_NAME; }
    String print() { return name; }
    String printFull() { return name + L" (NAME)"; }
    TokenName(String const &Name) : name(Name) {}
    TokenName *copy() { return new TokenName(name); }
};
struct TokenArray : public TokenValue {
    Vector<TokenBase *> values;
    virtual eTokenType type() { return TOKEN_ARRAY; }
    String print() {
        String result = L"[";
        for (UInt i = 0; i < values.size(); i++) {
            bool last = i == values.size() - 1;
            result += values[i]->print();
            if (!last)
                result += L",";
        }
        return result + L"]";
    }
    String printFull() {
        return print() + L" (ARRAY)";
    }
    TokenArray() {}
    TokenArray(Vector<TokenBase *> const &ary) {
        for (auto v : ary)
            values.push_back(v->copy());
    }
    ~TokenArray() {
        for (auto v : values)
            delete v;
    }
    TokenArray *copy() { return new TokenArray(values); }
};
struct TokenParamsOpen : public TokenBase {
    virtual eTokenType type() { return TOKEN_PARAMS_OPEN; }
    String print() { return L"("; }
    String printFull() { return L"( (PARAMS_OPEN)"; }
    TokenParamsOpen *copy() { return new TokenParamsOpen(); }
};
struct TokenParamsClose : public TokenBase {
    virtual eTokenType type() { return TOKEN_PARAMS_CLOSE; }
    String print() { return L")"; }
    String printFull() { return L") (PARAMS_CLOSE)"; }
    TokenParamsClose *copy() { return new TokenParamsClose(); }
};

FifamVersion GetFifamVersion(UInt gameId) {
    FifamVersion v;
    if (gameId <= 7 ) v.Set(0x2007, 0x0C);
    if (gameId == 8 ) v.Set(0x2007, 0x1E);
    if (gameId == 9 ) v.Set(0x2009, 0x05);
    if (gameId == 10) v.Set(0x2009, 0x0A);
    if (gameId == 11) v.Set(0x2011, 0x0A);
    if (gameId == 12) v.Set(0x2012, 0x04);
    if (gameId == 13) v.Set(0x2013, 0x0A);
    if (gameId >= 14) v.Set(0x2013, 0x0A);
    return v;
}

bool EqualWithCase(String const &a, String const &b) {
    return !wcscmp(a.c_str(), b.c_str());
}

bool Equal(String const &a, String const &b) {
    return !_wcsicmp(a.c_str(), b.c_str());
}

bool SS_IsHexadecimalLetter(wchar_t c) {
    return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool SS_IsNumber(std::wstring const &str) {
    if (str.empty())
        return false;
    bool hexadecimal = false;
    std::wstring cmpStr;
    if (Utils::StartsWith(str, L"0x") || Utils::StartsWith(str, L"0X")) {
        cmpStr = str.substr(2);
        hexadecimal = true;
    }
    else
        cmpStr = str;
    for (wchar_t c : cmpStr) {
        if (isdigit(c))
            continue;
        if (hexadecimal && SS_IsHexadecimalLetter(c))
            continue;
        return false;
    }
    return true;
}

int SS_ToNumber(std::wstring const &str) {
    std::wstring trimmed = str;
    Utils::Trim(trimmed);
    return ((Utils::StartsWith(trimmed, L"0x") || Utils::StartsWith(trimmed, L"0X")) ? wcstol(trimmed.substr(2).c_str(), nullptr, 16) : wcstol(trimmed.c_str(), nullptr, 10));
}

bool ScriptEngine::ExtractTokens(String const &in, Vector<TokenBase *> &tokens, String &error) {
    std::stack<Vector<TokenBase *> *> tokensStack;
    tokensStack.push(&tokens);
    bool insideArray = false;
    for (UInt i = 0; i < in.size(); i++) {
        bool notLast = i < (in.size() - 1);
        WideChar c = in[i];
        WideChar c2 = 0;
        if (notLast)
            c2 = in[i + 1];
        if ((c == '-' || c == '~') && c2 == '>') {
            if (insideArray) {
                error = L"Successor can't be used inside array";
                return false;
            }
            tokensStack.top()->push_back(new TokenSuccessor(c == '~'));
            i += 1;
            continue;
        }
        if (c == '<' && (c2 == '-' || c2 == '~')) {
            if (insideArray) {
                error = L"Predecessor can't be used inside array";
                return false;
            }
            tokensStack.top()->push_back(new TokenPredecessor(c2 == '~'));
            i += 1;
            continue;
        }
        if (c == '{') {
            if (insideArray) {
                error = L"Block operator can't be used inside array";
                return false;
            }
            tokensStack.top()->push_back(new TokenOpen());
            continue;
        }
        if (c == '}') {
            if (insideArray) {
                error = L"Block operator can't be used inside array";
                return false;
            }
            tokensStack.top()->push_back(new TokenClose());
            continue;
        }
        if (c == '(') {
            tokensStack.top()->push_back(new TokenParamsOpen());
            continue;
        }
        if (c == ')') {
            tokensStack.top()->push_back(new TokenParamsClose());
            continue;
        }
        if (c == '[') { // parse array
            auto ary = new TokenArray();
            tokensStack.top()->push_back(ary);
            tokensStack.push(&ary->values);
            if (!insideArray)
                insideArray = true;
            continue;
        }
        if (c == ']') {
            if (tokensStack.top() != &tokens) {
                tokensStack.pop();
                if (tokensStack.top() == &tokens)
                    insideArray = false;
            }
            else {
                error = L"']': unexpected array-close operator";
                return false;
            }
            continue;
        }
        if (c == '"') { // parse string
            String str;
            Bool finished = false;
            for (i = i + 1; i < in.size(); i++) { //-V535
                if (in[i] == '\\' && i < (in.size() - 1) && in[i + 1] == '"') {
                    str += '"';
                    i += 1;
                    continue;
                }
                if (in[i] == '\\' && i < in.size() - 1 && in[i + 1] == 'n') {
                    str += '\n';
                    i += 1;
                    continue;
                }
                if (in[i] == '"') {
                    finished = true;
                    break;
                }
                if (in[i] == '\n') {
                    error = L"unexpected string end";
                    return false;
                }
                if (in[i] == '\r' && i < in.size() - 1 && in[i + 1] == '\n') {
                    error = L"unexpected string end";
                    return false;
                }
                str += in[i];
            }
            if (!finished) {
                error = L"unexpected string end";
                return false;
            }
            tokensStack.top()->push_back(new TokenString(str));
            continue;
        }
        if (c == '/' && c2 == '/') { // parse line comment
            for (i = i + 2; i < in.size(); i++) {
                if (in[i] == '\n')
                    break;
                else if (in[i] == '\r' && i < in.size() - 1 && in[i + 1] == '\n') {
                    i += 1;
                    break;
                }
            }
            continue;
        }
        if (c == '/' && c2 == '*') { // parse block comment
            for (i = i + 2; i < in.size(); i++) {
                if (in[i] == '*' && i < in.size() - 1 && in[i + 1] == '/') {
                    i += 1;
                    break;
                }
            }
            continue;
        }
        if (c == ' ' || c == '\n' || c == '\t' || c == ',' || c == ';')
            continue;
        // parse word
        String word;
        word += c;
        for (UInt j = i + 1; j < in.size(); j++) {
            c = in[j];
            if (c == ' ' || c == '\n' || c == '\t' || c == '(' || c == ')' || c == ',' || c == ';' || c == '/' || c == '{' || c == '}' || c == '<' || c == '>' || c == '[' || c == ']')
                break;
            word += c;
        }
        if (word[0] == '@') {
            if (word.size() > 1 && word[1] == '@') {
                String refName = word.substr(2);
                if (refName.empty()) {
                    error = L"empty reference name";
                    return false;
                }
                tokensStack.top()->push_back(new TokenDirectReference(refName));
            }
            else {
                String refName = word.substr(1);
                if (refName.empty()) {
                    error = L"empty reference name";
                    return false;
                }
                tokensStack.top()->push_back(new TokenReference(refName));
            }
        }
        else if (word[0] == L'%') {
            if (insideArray) {
                if (!word.substr(1).empty()) {
                    Vector<TokenBase *> &vec = *tokensStack.top();
                    if (word.substr(1) == L"r") {
                        if (vec.size() > 1)
                            std::reverse(vec.begin(), vec.end());
                    }
                    else if (word.substr(1) == L"x" || word.substr(1) == L"rx") {
                        if (vec.size() > 1) {
                            auto copy = vec;
                            for (UInt vi = 0; vi < vec.size(); vi++) {
                                UInt newPos = vi * 2;
                                if (newPos >= vec.size())
                                    vec[newPos - vec.size() + (1 - vec.size() % 2)] = copy[vi];
                                else
                                    vec[newPos] = copy[vi];
                            }
                            if (word.substr(1) == L"rx") {
                                if (vec.size() > 2) {
                                    copy = vec;
                                    for (UInt vi = 0; vi < vec.size(); vi++) {
                                        if ((vi % 4) <= 1 && (vi + 2 < vec.size())) {
                                            vec[vi] = copy[vi + 2];
                                            vec[vi + 2] = copy[vi];
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else {
                        error = L"'%': unknown array sorter can be used only inside an array";
                        return false;
                    }
                }
                else {
                    error = L"'%': array sorter is not specified";
                    return false;
                } 
            }
            else {
                error = L"'%': array sorter can be used only inside an array";
                return false;
            }
        }
        else {
            if (SS_IsNumber(word))
                tokensStack.top()->push_back(new TokenNumber(SS_ToNumber(word)));
            else {
                auto numbers = Utils::Split(word, L':');
                if (numbers.size() == 2)
                    tokensStack.top()->push_back(new TokenNumber2D(SS_ToNumber(numbers[0]), SS_ToNumber(numbers[1])));
                else {
                    if (insideArray) {
                        auto repeat = Utils::Split(word, L'*');
                        if (repeat.size() == 2) {
                            String strRepeatTimes;
                            Int modifier = 0;
                            if (SS_IsNumber(repeat[0])) {
                                auto plusParts = Utils::Split(repeat[1], L'+');
                                if (plusParts.size() == 2) {
                                    if (SS_IsNumber(plusParts[0]) && SS_IsNumber(plusParts[1])) {
                                        strRepeatTimes = plusParts[0];
                                        modifier = SS_ToNumber(plusParts[1]);
                                    }
                                    else {
                                        error = L"'" + word + L"' : incorrect repeat syntax (only numbers are allowed)";
                                        return false;
                                    }
                                }
                                else {
                                    auto minusParts = Utils::Split(repeat[1], L'-');
                                    if (minusParts.size() == 2) {
                                        if (SS_IsNumber(minusParts[0]) && SS_IsNumber(minusParts[1])) {
                                            strRepeatTimes = minusParts[0];
                                            modifier = -SS_ToNumber(minusParts[1]);
                                        }
                                        else {
                                            error = L"'" + word + L"' : incorrect repeat syntax (only numbers are allowed)";
                                            return false;
                                        }
                                    }
                                    else {
                                        if (!SS_IsNumber(repeat[1])) {
                                            error = L"'" + word + L"' : incorrect repeat syntax (only numbers are allowed)";
                                            return false;
                                        }
                                        strRepeatTimes = repeat[1];
                                    }
                                }
                                Int repeatValue = SS_ToNumber(repeat[0]);
                                Int repeatCount = SS_ToNumber(strRepeatTimes);
                                if (repeatCount > 0) {
                                    for (Int i = 0; i < repeatCount; i++) {
                                        tokensStack.top()->push_back(new TokenNumber(repeatValue));
                                        repeatValue += modifier;
                                    }
                                }
                                else {
                                    error = L"'" + word + L"' : incorrect repeat syntax (repeat counter must be greater than 0)";
                                    return false;
                                }
                            }
                            else {
                                error = L"'" + word + L"' : incorrect repeat syntax (only numbers are allowed)";
                                return false;
                            }
                        }
                        else {
                            auto range = Utils::Split(word, L'-');
                            if (range.size() == 2) {
                                if (!SS_IsNumber(range[0])) {
                                    error = L"'" + word + L"' : incorrect range syntax (only numbers are allowed)";
                                    return false;
                                }
                                Int rangeA = SS_ToNumber(range[0]);
                                Int rangeB = 0;
                                UInt rangeStep = 1;
                                auto rangeAndStep = Utils::Split(range[1], L'^');

                                if (rangeAndStep.size() == 2) {
                                    if (SS_IsNumber(rangeAndStep[0]) && SS_IsNumber(rangeAndStep[1])) {
                                        rangeB = SS_ToNumber(rangeAndStep[0]);
                                        rangeStep = SS_ToNumber(rangeAndStep[1]);
                                        if (rangeStep < 1) {
                                            error = L"'" + word + L"' : incorrect range syntax (range step must be greater than 0)";
                                            return false;
                                        }
                                    }
                                    else {
                                        error = L"'" + word + L"' : incorrect range syntax (only numbers are allowed)";
                                        return false;
                                    }
                                }
                                else
                                    rangeB = SS_ToNumber(range[1]);
                                if (rangeB > rangeA) {
                                    for (Int i = rangeA; i <= rangeB; i += rangeStep)
                                        tokensStack.top()->push_back(new TokenNumber(i));
                                }
                                else {
                                    error = L"'" + word + L"' : incorrect range syntax (range end must be greater than range start)";
                                    return false;
                                }
                            }
                            else
                                tokensStack.top()->push_back(new TokenName(word));
                        }
                    }
                    else
                        tokensStack.top()->push_back(new TokenName(word));
                }
            }
        }
        i += word.size() - 1;
    }
    return true;
}

bool GetContinentByName(String const &str, FifamCompRegion &region) {
    region = FifamCompRegion::None;
    if (Equal(str, L"Europe"))
        region = FifamCompRegion::Europe;
    else if (Equal(str, L"SouthAmerica"))
        region = FifamCompRegion::SouthAmerica;
    else if (Equal(str, L"NorthAmerica"))
        region = FifamCompRegion::NorthAmerica;
    else if (Equal(str, L"Africa"))
        region = FifamCompRegion::Africa;
    else if (Equal(str, L"Asia"))
        region = FifamCompRegion::Asia;
    else if (Equal(str, L"Oceania"))
        region = FifamCompRegion::Oceania;
    else if (Equal(str, L"International"))
        region = FifamCompRegion::International;
    return region != FifamCompRegion::None;
}

bool GetCupSystemTypeByName(String const &str, FifamCupSystemType &systemType) {
    systemType = FifamCupSystemType::None;
    if (Equal(str, L"FA_Germany"))            systemType = FifamCupSystemType::FA_Germany;
    else if (Equal(str, L"FA_England"))            systemType = FifamCupSystemType::FA_England;
    else if (Equal(str, L"FA_France"))             systemType = FifamCupSystemType::FA_France;
    else if (Equal(str, L"FA_Spain"))              systemType = FifamCupSystemType::FA_Spain;
    else if (Equal(str, L"FA_Italy"))              systemType = FifamCupSystemType::FA_Italy;
    else if (Equal(str, L"LeagueCup_Germany"))     systemType = FifamCupSystemType::LeagueCup_Germany;
    else if (Equal(str, L"LeagueCup_England"))     systemType = FifamCupSystemType::LeagueCup_England;
    else if (Equal(str, L"LeagueCup_France"))      systemType = FifamCupSystemType::LeagueCup_France;
    else if (Equal(str, L"CC_FA_Germany_32_1"))    systemType = FifamCupSystemType::CC_FA_Germany_32_1;
    else if (Equal(str, L"CC_FA_Germany_32_2"))    systemType = FifamCupSystemType::CC_FA_Germany_32_2;
    else if (Equal(str, L"FA_Scotland"))           systemType = FifamCupSystemType::FA_Scotland;
    else if (Equal(str, L"LeagueCup_Scotland"))    systemType = FifamCupSystemType::LeagueCup_Scotland;
    else if (Equal(str, L"LeaguecupDiv2And3"))     systemType = FifamCupSystemType::LeaguecupDiv2And3;
    else if (Equal(str, L"ConferenceCup"))         systemType = FifamCupSystemType::ConferenceCup;
    else if (Equal(str, L"Supercup1Leg"))          systemType = FifamCupSystemType::Supercup1Leg;
    else if (Equal(str, L"Supercup2Leg"))          systemType = FifamCupSystemType::Supercup2Leg;
    else if (Equal(str, L"FriendlyMatch"))         systemType = FifamCupSystemType::FriendlyMatch;
    else if (Equal(str, L"Tournament4Teams"))      systemType = FifamCupSystemType::Tournament4Teams;
    else if (Equal(str, L"RelegationSwitherland")) systemType = FifamCupSystemType::RelegationSwitherland;
    else if (Equal(str, L"RelegationEngland3"))    systemType = FifamCupSystemType::RelegationEngland3;
    else if (Equal(str, L"FA_Cup_Small"))          systemType = FifamCupSystemType::FA_Cup_Small;
    else if (Equal(str, L"CC_FA_Germany_128"))     systemType = FifamCupSystemType::CC_FA_Germany_128;
    else if (Equal(str, L"CC_FA_Germany_64"))      systemType = FifamCupSystemType::CC_FA_Germany_64;
    else if (Equal(str, L"CC_FA_Germany_32_3"))    systemType = FifamCupSystemType::CC_FA_Germany_32_3;
    else if (Equal(str, L"CC_FA_Germany_64_32"))   systemType = FifamCupSystemType::CC_FA_Germany_64_32;
    return systemType != FifamCompRegion::None;
}

bool GetTeamTypeByName(String const &str, FifamClubTeamType &teamType) {
    if (Equal(str, L"First")) {
        teamType = FifamClubTeamType::First;
        return true;
    }
    else if (Equal(str, L"Reserve")) {
        teamType = FifamClubTeamType::Reserve;
        return true;
    }
    else if (Equal(str, L"YouthA")) {
        teamType = FifamClubTeamType::YouthA;
        return true;
    }
    else if (Equal(str, L"YouthB")) {
        teamType = FifamClubTeamType::YouthB;
        return true;
    }
    return false;
}

FifamCompRegion GetCountryRegion(String const &str) {
    if (Equal(str, L"Albania")) return FifamCompRegion::Albania;
    if (Equal(str, L"Andorra")) return FifamCompRegion::Andorra;
    if (Equal(str, L"Armenia")) return FifamCompRegion::Armenia;
    if (Equal(str, L"Austria")) return FifamCompRegion::Austria;
    if (Equal(str, L"Azerbaijan"))return FifamCompRegion::Azerbaijan;
    if (Equal(str, L"Belarus")) return FifamCompRegion::Belarus;
    if (Equal(str, L"Belgium")) return FifamCompRegion::Belgium;
    if (Equal(str, L"BosniaHerzegovina")) return FifamCompRegion::Bosnia_Herzegovina;
    if (Equal(str, L"Bulgaria"))return FifamCompRegion::Bulgaria;
    if (Equal(str, L"Croatia")) return FifamCompRegion::Croatia;
    if (Equal(str, L"Cyprus"))return FifamCompRegion::Cyprus;
    if (Equal(str, L"CzechRepublic")) return FifamCompRegion::Czech_Republic;
    if (Equal(str, L"Denmark")) return FifamCompRegion::Denmark;
    if (Equal(str, L"England")) return FifamCompRegion::England;
    if (Equal(str, L"Estonia")) return FifamCompRegion::Estonia;
    if (Equal(str, L"FaroeIslands")) return FifamCompRegion::Faroe_Islands;
    if (Equal(str, L"Finland")) return FifamCompRegion::Finland;
    if (Equal(str, L"France"))return FifamCompRegion::France;
    if (Equal(str, L"NorthMacedonia")) return FifamCompRegion::FYR_Macedonia;
    if (Equal(str, L"Macedonia")) return FifamCompRegion::FYR_Macedonia;
    if (Equal(str, L"Georgia")) return FifamCompRegion::Georgia;
    if (Equal(str, L"Germany")) return FifamCompRegion::Germany;
    if (Equal(str, L"Greece"))return FifamCompRegion::Greece;
    if (Equal(str, L"Hungary")) return FifamCompRegion::Hungary;
    if (Equal(str, L"Iceland")) return FifamCompRegion::Iceland;
    if (Equal(str, L"Ireland")) return FifamCompRegion::Ireland;
    if (Equal(str, L"Israel"))return FifamCompRegion::Israel;
    if (Equal(str, L"Italy")) return FifamCompRegion::Italy;
    if (Equal(str, L"Latvia"))return FifamCompRegion::Latvia;
    if (Equal(str, L"Liechtenstein")) return FifamCompRegion::Liechtenstein;
    if (Equal(str, L"Lithuania")) return FifamCompRegion::Lithuania;
    if (Equal(str, L"Luxembourg"))return FifamCompRegion::Luxembourg;
    if (Equal(str, L"Malta")) return FifamCompRegion::Malta;
    if (Equal(str, L"Moldova")) return FifamCompRegion::Moldova;
    if (Equal(str, L"Netherlands")) return FifamCompRegion::Netherlands;
    if (Equal(str, L"NorthernIreland"))return FifamCompRegion::Northern_Ireland;
    if (Equal(str, L"Norway"))return FifamCompRegion::Norway;
    if (Equal(str, L"Poland"))return FifamCompRegion::Poland;
    if (Equal(str, L"Portugal"))return FifamCompRegion::Portugal;
    if (Equal(str, L"Romania")) return FifamCompRegion::Romania;
    if (Equal(str, L"Russia"))return FifamCompRegion::Russia;
    if (Equal(str, L"SanMarino"))return FifamCompRegion::San_Marino;
    if (Equal(str, L"Scotland"))return FifamCompRegion::Scotland;
    if (Equal(str, L"Slovakia"))return FifamCompRegion::Slovakia;
    if (Equal(str, L"Slovenia"))return FifamCompRegion::Slovenia;
    if (Equal(str, L"Spain")) return FifamCompRegion::Spain;
    if (Equal(str, L"Sweden"))return FifamCompRegion::Sweden;
    if (Equal(str, L"Switzerland")) return FifamCompRegion::Switzerland;
    if (Equal(str, L"Turkey"))return FifamCompRegion::Turkey;
    if (Equal(str, L"Ukraine")) return FifamCompRegion::Ukraine;
    if (Equal(str, L"Wales")) return FifamCompRegion::Wales;
    if (Equal(str, L"Serbia"))return FifamCompRegion::Serbia;
    if (Equal(str, L"Argentina")) return FifamCompRegion::Argentina;
    if (Equal(str, L"Bolivia")) return FifamCompRegion::Bolivia;
    if (Equal(str, L"Brazil"))return FifamCompRegion::Brazil;
    if (Equal(str, L"Chile")) return FifamCompRegion::Chile;
    if (Equal(str, L"Colombia"))return FifamCompRegion::Colombia;
    if (Equal(str, L"Ecuador")) return FifamCompRegion::Ecuador;
    if (Equal(str, L"Paraguay"))return FifamCompRegion::Paraguay;
    if (Equal(str, L"Peru"))return FifamCompRegion::Peru;
    if (Equal(str, L"Uruguay")) return FifamCompRegion::Uruguay;
    if (Equal(str, L"Venezuela")) return FifamCompRegion::Venezuela;
    if (Equal(str, L"Anguilla"))return FifamCompRegion::Anguilla;
    if (Equal(str, L"NewCaledonia"))return FifamCompRegion::Anguilla;
    if (Equal(str, L"AntiguaAndBarbuda")) return FifamCompRegion::Antigua_and_Barbuda;
    if (Equal(str, L"Aruba")) return FifamCompRegion::Aruba;
    if (Equal(str, L"Bahamas")) return FifamCompRegion::Bahamas;
    if (Equal(str, L"Barbados"))return FifamCompRegion::Barbados;
    if (Equal(str, L"Belize"))return FifamCompRegion::Belize;
    if (Equal(str, L"Bermuda")) return FifamCompRegion::Bermuda;
    if (Equal(str, L"BritishVirginIs")) return FifamCompRegion::British_Virgin_Is;
    if (Equal(str, L"Canada"))return FifamCompRegion::Canada;
    if (Equal(str, L"CaymanIslands"))return FifamCompRegion::Cayman_Islands;
    if (Equal(str, L"CostaRica"))return FifamCompRegion::Costa_Rica;
    if (Equal(str, L"Cuba"))return FifamCompRegion::Cuba;
    if (Equal(str, L"Dominica"))return FifamCompRegion::Dominica;
    if (Equal(str, L"DominicanRepublic"))return FifamCompRegion::Dominican_Republic;
    if (Equal(str, L"ElSalvador")) return FifamCompRegion::El_Salvador;
    if (Equal(str, L"Grenada")) return FifamCompRegion::Grenada;
    if (Equal(str, L"Guatemala")) return FifamCompRegion::Guatemala;
    if (Equal(str, L"Guyana"))return FifamCompRegion::Guyana;
    if (Equal(str, L"Haiti")) return FifamCompRegion::Haiti;
    if (Equal(str, L"Honduras"))return FifamCompRegion::Honduras;
    if (Equal(str, L"Jamaica")) return FifamCompRegion::Jamaica;
    if (Equal(str, L"Mexico"))return FifamCompRegion::Mexico;
    if (Equal(str, L"Montserrat"))return FifamCompRegion::Montserrat;
    if (Equal(str, L"Curacao")) return FifamCompRegion::Netherlands_Antil;
    if (Equal(str, L"NetherlandsAntil")) return FifamCompRegion::Netherlands_Antil;
    if (Equal(str, L"Nicaragua")) return FifamCompRegion::Nicaragua;
    if (Equal(str, L"Panama"))return FifamCompRegion::Panama;
    if (Equal(str, L"PuertoRico")) return FifamCompRegion::Puerto_Rico;
    if (Equal(str, L"StKittsNevis"))return FifamCompRegion::St_Kitts_Nevis;
    if (Equal(str, L"StLucia"))return FifamCompRegion::St_Lucia;
    if (Equal(str, L"StVincentGren")) return FifamCompRegion::St_Vincent_Gren;
    if (Equal(str, L"Surinam")) return FifamCompRegion::Surinam;
    if (Equal(str, L"TrinidadTobago")) return FifamCompRegion::Trinidad_Tobago;
    if (Equal(str, L"TurksAndCaicos"))return FifamCompRegion::Turks_and_Caicos;
    if (Equal(str, L"UnitedStates")) return FifamCompRegion::United_States;
    if (Equal(str, L"USVirginIslands")) return FifamCompRegion::US_Virgin_Islands;
    if (Equal(str, L"Algeria")) return FifamCompRegion::Algeria;
    if (Equal(str, L"Angola"))return FifamCompRegion::Angola;
    if (Equal(str, L"Benin")) return FifamCompRegion::Benin;
    if (Equal(str, L"Botswana"))return FifamCompRegion::Botswana;
    if (Equal(str, L"BurkinaFaso"))return FifamCompRegion::Burkina_Faso;
    if (Equal(str, L"Burundi")) return FifamCompRegion::Burundi;
    if (Equal(str, L"Cameroon"))return FifamCompRegion::Cameroon;
    if (Equal(str, L"CapeVerdeIslands"))return FifamCompRegion::Cape_Verde_Islands;
    if (Equal(str, L"CentralAfricanRep")) return FifamCompRegion::Central_African_Rep;
    if (Equal(str, L"Chad"))return FifamCompRegion::Chad;
    if (Equal(str, L"Congo")) return FifamCompRegion::Congo;
    if (Equal(str, L"CoteDIvoire")) return FifamCompRegion::Cote_d_Ivoire;
    if (Equal(str, L"Djibouti"))return FifamCompRegion::Djibouti;
    if (Equal(str, L"DRCongo"))return FifamCompRegion::DR_Congo;
    if (Equal(str, L"Egypt")) return FifamCompRegion::Egypt;
    if (Equal(str, L"EquatorialGuinea")) return FifamCompRegion::Equatorial_Guinea;
    if (Equal(str, L"Eritrea")) return FifamCompRegion::Eritrea;
    if (Equal(str, L"Ethiopia"))return FifamCompRegion::Ethiopia;
    if (Equal(str, L"Gabon")) return FifamCompRegion::Gabon;
    if (Equal(str, L"Gambia"))return FifamCompRegion::Gambia;
    if (Equal(str, L"Ghana")) return FifamCompRegion::Ghana;
    if (Equal(str, L"Guinea"))return FifamCompRegion::Guinea;
    if (Equal(str, L"GuineaBissau")) return FifamCompRegion::Guinea_Bissau;
    if (Equal(str, L"Kenya")) return FifamCompRegion::Kenya;
    if (Equal(str, L"Lesotho")) return FifamCompRegion::Lesotho;
    if (Equal(str, L"Liberia")) return FifamCompRegion::Liberia;
    if (Equal(str, L"Libya")) return FifamCompRegion::Libya;
    if (Equal(str, L"Madagascar"))return FifamCompRegion::Madagascar;
    if (Equal(str, L"Malawi"))return FifamCompRegion::Malawi;
    if (Equal(str, L"Mali"))return FifamCompRegion::Mali;
    if (Equal(str, L"Mauritania"))return FifamCompRegion::Mauritania;
    if (Equal(str, L"Mauritius")) return FifamCompRegion::Mauritius;
    if (Equal(str, L"Morocco")) return FifamCompRegion::Morocco;
    if (Equal(str, L"Mozambique"))return FifamCompRegion::Mozambique;
    if (Equal(str, L"Namibia")) return FifamCompRegion::Namibia;
    if (Equal(str, L"Niger")) return FifamCompRegion::Niger;
    if (Equal(str, L"Nigeria")) return FifamCompRegion::Nigeria;
    if (Equal(str, L"Rwanda"))return FifamCompRegion::Rwanda;
    if (Equal(str, L"SaoTomeEPrincipe")) return FifamCompRegion::Sao_Tome_e_Principe;
    if (Equal(str, L"Senegal")) return FifamCompRegion::Senegal;
    if (Equal(str, L"Seychelles"))return FifamCompRegion::Seychelles;
    if (Equal(str, L"SierraLeone"))return FifamCompRegion::Sierra_Leone;
    if (Equal(str, L"Somalia")) return FifamCompRegion::Somalia;
    if (Equal(str, L"SouthAfrica"))return FifamCompRegion::South_Africa;
    if (Equal(str, L"Sudan")) return FifamCompRegion::Sudan;
    if (Equal(str, L"Eswatini")) return FifamCompRegion::Swaziland;
    if (Equal(str, L"Swaziland")) return FifamCompRegion::Swaziland;
    if (Equal(str, L"Tanzania"))return FifamCompRegion::Tanzania;
    if (Equal(str, L"Togo"))return FifamCompRegion::Togo;
    if (Equal(str, L"Tunisia")) return FifamCompRegion::Tunisia;
    if (Equal(str, L"Uganda"))return FifamCompRegion::Uganda;
    if (Equal(str, L"Zambia"))return FifamCompRegion::Zambia;
    if (Equal(str, L"Zimbabwe"))return FifamCompRegion::Zimbabwe;
    if (Equal(str, L"Afghanistan")) return FifamCompRegion::Afghanistan;
    if (Equal(str, L"Bahrain")) return FifamCompRegion::Bahrain;
    if (Equal(str, L"Bangladesh"))return FifamCompRegion::Bangladesh;
    if (Equal(str, L"Bhutan"))return FifamCompRegion::Bhutan;
    if (Equal(str, L"BruneiDarussalam")) return FifamCompRegion::Brunei_Darussalam;
    if (Equal(str, L"Cambodia"))return FifamCompRegion::Cambodia;
    if (Equal(str, L"China"))return FifamCompRegion::China_PR;
    if (Equal(str, L"Taiwan"))return FifamCompRegion::Taiwan;
    if (Equal(str, L"Guam"))return FifamCompRegion::Guam;
    if (Equal(str, L"HongKong")) return FifamCompRegion::Hong_Kong;
    if (Equal(str, L"India")) return FifamCompRegion::India;
    if (Equal(str, L"Indonesia")) return FifamCompRegion::Indonesia;
    if (Equal(str, L"Iran"))return FifamCompRegion::Iran;
    if (Equal(str, L"Iraq"))return FifamCompRegion::Iraq;
    if (Equal(str, L"Japan")) return FifamCompRegion::Japan;
    if (Equal(str, L"Jordan"))return FifamCompRegion::Jordan;
    if (Equal(str, L"Kazakhstan"))return FifamCompRegion::Kazakhstan;
    if (Equal(str, L"KoreaDPR")) return FifamCompRegion::Korea_DPR;
    if (Equal(str, L"KoreaRepublic"))return FifamCompRegion::Korea_Republic;
    if (Equal(str, L"Kuwait"))return FifamCompRegion::Kuwait;
    if (Equal(str, L"Kyrgyzstan"))return FifamCompRegion::Kyrgyzstan;
    if (Equal(str, L"Laos"))return FifamCompRegion::Laos;
    if (Equal(str, L"Lebanon")) return FifamCompRegion::Lebanon;
    if (Equal(str, L"Macao")) return FifamCompRegion::Macao;
    if (Equal(str, L"Malaysia"))return FifamCompRegion::Malaysia;
    if (Equal(str, L"Maldives"))return FifamCompRegion::Maldives;
    if (Equal(str, L"Mongolia"))return FifamCompRegion::Mongolia;
    if (Equal(str, L"Myanmar")) return FifamCompRegion::Myanmar;
    if (Equal(str, L"Nepal")) return FifamCompRegion::Nepal;
    if (Equal(str, L"Oman"))return FifamCompRegion::Oman;
    if (Equal(str, L"Pakistan"))return FifamCompRegion::Pakistan;
    if (Equal(str, L"PalestinianAuthority")) return FifamCompRegion::Palestinian_Authority;
    if (Equal(str, L"Philippines")) return FifamCompRegion::Philippines;
    if (Equal(str, L"Qatar")) return FifamCompRegion::Qatar;
    if (Equal(str, L"SaudiArabia"))return FifamCompRegion::Saudi_Arabia;
    if (Equal(str, L"Singapore")) return FifamCompRegion::Singapore;
    if (Equal(str, L"SriLanka")) return FifamCompRegion::Sri_Lanka;
    if (Equal(str, L"Syria")) return FifamCompRegion::Syria;
    if (Equal(str, L"Tajikistan"))return FifamCompRegion::Tajikistan;
    if (Equal(str, L"Thailand"))return FifamCompRegion::Thailand;
    if (Equal(str, L"Turkmenistan"))return FifamCompRegion::Turkmenistan;
    if (Equal(str, L"UnitedArabEmirates"))return FifamCompRegion::United_Arab_Emirates;
    if (Equal(str, L"Uzbekistan"))return FifamCompRegion::Uzbekistan;
    if (Equal(str, L"Vietnam")) return FifamCompRegion::Vietnam;
    if (Equal(str, L"Yemen")) return FifamCompRegion::Yemen;
    if (Equal(str, L"AmericanSamoa"))return FifamCompRegion::American_Samoa;
    if (Equal(str, L"Australia")) return FifamCompRegion::Australia;
    if (Equal(str, L"CookIslands"))return FifamCompRegion::Cook_Islands;
    if (Equal(str, L"Fiji"))return FifamCompRegion::Fiji;
    if (Equal(str, L"NewZealand")) return FifamCompRegion::New_Zealand;
    if (Equal(str, L"PapuaNewGuinea"))return FifamCompRegion::Papua_New_Guinea;
    if (Equal(str, L"Samoa")) return FifamCompRegion::Samoa;
    if (Equal(str, L"SolomonIslands")) return FifamCompRegion::Solomon_Islands;
    if (Equal(str, L"Tahiti"))return FifamCompRegion::Tahiti;
    if (Equal(str, L"Tonga")) return FifamCompRegion::Tonga;
    if (Equal(str, L"Vanuatu")) return FifamCompRegion::Vanuatu;
    if (Equal(str, L"Gibraltar")) return FifamCompRegion::Gibraltar;
    if (Equal(str, L"Montenegro"))return FifamCompRegion::Montenegro;
    if (Equal(str, L"Greenland")) return FifamCompRegion::Greenland;
    if (Equal(str, L"Kosovo")) return FifamCompRegion::Greenland;
    return FifamCompRegion::None;
}

bool GetCountryByName(String const &str, FifamCompRegion &region) {
    region = GetCountryRegion(str);
    return region != FifamCompRegion::None;
}

void GetCountriesList(FifamCompRegion &continent, Vector<FifamNation> &outCountries, UInt gameId, Bool ucpMode) {
    if (continent == FifamCompRegion::Europe) {
        for (UInt i = 1; i <= 51; i++)
            outCountries.push_back(FifamNation::MakeFromInt(i));
        outCountries.push_back(FifamNation::MakeFromInt(165));
        if (gameId >= 13 && ucpMode)
            outCountries.push_back(FifamNation::MakeFromInt(205));
        if (gameId > 7)
            outCountries.push_back(FifamNation::MakeFromInt(206));
        if (gameId >= 13 && ucpMode)
            outCountries.push_back(FifamNation::MakeFromInt(207));
    }
    else if (continent == FifamCompRegion::SouthAmerica) {
        for (UInt i = 52; i <= 61; i++)
            outCountries.push_back(FifamNation::MakeFromInt(i));
    }
    else if (continent == FifamCompRegion::NorthAmerica) {
        if (gameId < 13 || !ucpMode)
            outCountries.push_back(FifamNation::MakeFromInt(62));
        for (UInt i = 63; i <= 96; i++)
            outCountries.push_back(FifamNation::MakeFromInt(i));
    }
    else if (continent == FifamCompRegion::Africa) {
        for (UInt i = 97; i <= 148; i++)
            outCountries.push_back(FifamNation::MakeFromInt(i));
    }
    else if (continent == FifamCompRegion::Asia) {
        for (UInt i = 149; i <= 164; i++)
            outCountries.push_back(FifamNation::MakeFromInt(i));
        for (UInt i = 166; i <= 193; i++)
            outCountries.push_back(FifamNation::MakeFromInt(i));
        outCountries.push_back(FifamNation::MakeFromInt(195));
    }
    else if (continent == FifamCompRegion::Oceania) {
        if (gameId >= 13 && ucpMode)
            outCountries.push_back(FifamNation::MakeFromInt(62));
        outCountries.push_back(FifamNation::MakeFromInt(194));
        for (UInt i = 196; i <= 204; i++)
            outCountries.push_back(FifamNation::MakeFromInt(i));
    }
    else if (continent == FifamCompRegion::International) {
        for (UInt i = 1; i <= 207; i++) {
            if (gameId > 7 || i != 206)
                outCountries.push_back(FifamNation::MakeFromInt(i));
        }
    }
}

bool GetCompTypeByName(String const &str, FifamCompType &compType) {
    compType = FifamCompType::Root;
    if (Equal(str, L"ChampionsLeague"))
        compType = FifamCompType::ChampionsLeague;
    else if (Equal(str, L"UefaCup"))
        compType = FifamCompType::UefaCup;
    else if (Equal(str, L"Toyota"))
        compType = FifamCompType::Toyota;
    else if (Equal(str, L"EuroSuperCup"))
        compType = FifamCompType::EuroSuperCup;
    else if (Equal(str, L"WorldClubChamp"))
        compType = FifamCompType::WorldClubChamp;
    else if (Equal(str, L"UIC"))
        compType = FifamCompType::UIC;
    else if (Equal(str, L"QualiWC"))
        compType = FifamCompType::QualiWC;
    else if (Equal(str, L"QualiEC"))
        compType = FifamCompType::QualiEC;
    else if (Equal(str, L"WorldCup"))
        compType = FifamCompType::WorldCup;
    else if (Equal(str, L"EuroCup"))
        compType = FifamCompType::EuroCup;
    else if (Equal(str, L"U20WorldCup"))
        compType = FifamCompType::U20WorldCup;
    else if (Equal(str, L"ConfedCup"))
        compType = FifamCompType::ConfedCup;
    else if (Equal(str, L"CopaAmerica"))
        compType = FifamCompType::CopaAmerica;
    else if (Equal(str, L"ICC"))
        compType = FifamCompType::ICC;
    else if (Equal(str, L"EuroNLQ"))
        compType = FifamCompType::EuroNLQ;
    else if (Equal(str, L"EuroNL"))
        compType = FifamCompType::EuroNL;
    else if (Equal(str, L"YouthChampionsLeague"))
        compType = FifamCompType::YouthChampionsLeague;
    else if (Equal(str, L"Continental1"))
        compType = FifamCompType::Continental1;
    else if (Equal(str, L"Continental2"))
        compType = FifamCompType::Continental2;
    else if (Equal(str, L"NamCup"))
        compType = FifamCompType::NamCup;
    else if (Equal(str, L"NamNL"))
        compType = FifamCompType::NamNL;
    else if (Equal(str, L"NamNLQ"))
        compType = FifamCompType::NamNLQ;
    else if (Equal(str, L"AsiaCup"))
        compType = FifamCompType::AsiaCup;
    else if (Equal(str, L"AsiaCupQ"))
        compType = FifamCompType::AsiaCupQ;
    else if (Equal(str, L"OfcCup"))
        compType = FifamCompType::OfcCup;
    else if (Equal(str, L"OfcCupQ"))
        compType = FifamCompType::OfcCupQ;
    else if (Equal(str, L"AfricaCup"))
        compType = FifamCompType::AfricaCup;
    else if (Equal(str, L"AfricaCupQ"))
        compType = FifamCompType::AfricaCupQ;
    else if (Equal(str, L"U20WCQ"))
        compType = FifamCompType::U20WCQ;
    else if (Equal(str, L"U17WC"))
        compType = FifamCompType::U17WC;
    else if (Equal(str, L"U17WCQ"))
        compType = FifamCompType::U17WCQ;
    else if (Equal(str, L"U21EC"))
        compType = FifamCompType::U21EC;
    else if (Equal(str, L"U21ECQ"))
        compType = FifamCompType::U21ECQ;
    else if (Equal(str, L"U19EC"))
        compType = FifamCompType::U19EC;
    else if (Equal(str, L"U19ECQ"))
        compType = FifamCompType::U19ECQ;
    else if (Equal(str, L"U17EC"))
        compType = FifamCompType::U17EC;
    else if (Equal(str, L"U17ECQ"))
        compType = FifamCompType::U17ECQ;
    else if (Equal(str, L"Olympic"))
        compType = FifamCompType::Olympic;
    else if (Equal(str, L"OlympicQ"))
        compType = FifamCompType::OlympicQ;
    else if (Equal(str, L"Custom1"))
        compType = FifamCompType::Custom1;
    else if (Equal(str, L"Custom2"))
        compType = FifamCompType::Custom2;
    else if (Equal(str, L"Custom3"))
        compType = FifamCompType::Custom3;
    else if (Equal(str, L"Custom4"))
        compType = FifamCompType::Custom4;
    else if (Equal(str, L"Custom5"))
        compType = FifamCompType::Custom5;
    else if (Equal(str, L"Custom6"))
        compType = FifamCompType::Custom6;
    else if (Equal(str, L"Custom7"))
        compType = FifamCompType::Custom7;
    else if (Equal(str, L"Custom8"))
        compType = FifamCompType::Custom8;
    else if (Equal(str, L"Custom9"))
        compType = FifamCompType::Custom9;
    else if (Equal(str, L"Custom10"))
        compType = FifamCompType::Custom10;
    return compType != FifamCompType::Root;
}

bool GetCompTypeByName_DirectReference(String const &str, FifamCompType &compType) {
    compType = FifamCompType::Root;
    if (Equal(str, L"League"))
        compType = FifamCompType::League;
    else if (Equal(str, L"Pool"))
        compType = FifamCompType::Pool;
    else if (Equal(str, L"FACup"))
        compType = FifamCompType::FaCup;
    else if (Equal(str, L"LeagueCup"))
        compType = FifamCompType::LeagueCup;
    else if (Equal(str, L"SuperCup"))
        compType = FifamCompType::SuperCup;
    else if (Equal(str, L"Relegation"))
        compType = FifamCompType::Relegation;
    else if (Equal(str, L"ChampionsLeague"))
        compType = FifamCompType::ChampionsLeague;
    else if (Equal(str, L"UefaCup"))
        compType = FifamCompType::UefaCup;
    else if (Equal(str, L"Toyota"))
        compType = FifamCompType::Toyota;
    else if (Equal(str, L"EuroSuperCup"))
        compType = FifamCompType::EuroSuperCup;
    else if (Equal(str, L"WorldClubChamp"))
        compType = FifamCompType::WorldClubChamp;
    else if (Equal(str, L"UIC"))
        compType = FifamCompType::UIC;
    else if (Equal(str, L"QualiWC"))
        compType = FifamCompType::QualiWC;
    else if (Equal(str, L"QualiEC"))
        compType = FifamCompType::QualiEC;
    else if (Equal(str, L"WorldCup"))
        compType = FifamCompType::WorldCup;
    else if (Equal(str, L"EuroCup"))
        compType = FifamCompType::EuroCup;
    else if (Equal(str, L"U20WorldCup"))
        compType = FifamCompType::U20WorldCup;
    else if (Equal(str, L"ConfedCup"))
        compType = FifamCompType::ConfedCup;
    else if (Equal(str, L"CopaAmerica"))
        compType = FifamCompType::CopaAmerica;
    else if (Equal(str, L"ICC"))
        compType = FifamCompType::ICC;
    else if (Equal(str, L"EuroNLQ"))
        compType = FifamCompType::EuroNLQ;
    else if (Equal(str, L"EuroNL"))
        compType = FifamCompType::EuroNL;
    else if (Equal(str, L"YouthChampionsLeague"))
        compType = FifamCompType::YouthChampionsLeague;
    else if (Equal(str, L"Continental1"))
        compType = FifamCompType::Continental1;
    else if (Equal(str, L"Continental2"))
        compType = FifamCompType::Continental2;
    else if (Equal(str, L"NamCup"))
        compType = FifamCompType::NamCup;
    else if (Equal(str, L"NamNL"))
        compType = FifamCompType::NamNL;
    else if (Equal(str, L"NamNLQ"))
        compType = FifamCompType::NamNLQ;
    else if (Equal(str, L"AsiaCup"))
        compType = FifamCompType::AsiaCup;
    else if (Equal(str, L"AsiaCupQ"))
        compType = FifamCompType::AsiaCupQ;
    else if (Equal(str, L"OfcCup"))
        compType = FifamCompType::OfcCup;
    else if (Equal(str, L"OfcCupQ"))
        compType = FifamCompType::OfcCupQ;
    else if (Equal(str, L"AfricaCup"))
        compType = FifamCompType::AfricaCup;
    else if (Equal(str, L"AfricaCupQ"))
        compType = FifamCompType::AfricaCupQ;
    else if (Equal(str, L"U20WCQ"))
        compType = FifamCompType::U20WCQ;
    else if (Equal(str, L"U17WC"))
        compType = FifamCompType::U17WC;
    else if (Equal(str, L"U17WCQ"))
        compType = FifamCompType::U17WCQ;
    else if (Equal(str, L"U21EC"))
        compType = FifamCompType::U21EC;
    else if (Equal(str, L"U21ECQ"))
        compType = FifamCompType::U21ECQ;
    else if (Equal(str, L"U19EC"))
        compType = FifamCompType::U19EC;
    else if (Equal(str, L"U19ECQ"))
        compType = FifamCompType::U19ECQ;
    else if (Equal(str, L"U17EC"))
        compType = FifamCompType::U17EC;
    else if (Equal(str, L"U17ECQ"))
        compType = FifamCompType::U17ECQ;
    else if (Equal(str, L"Olympic"))
        compType = FifamCompType::Olympic;
    else if (Equal(str, L"OlympicQ"))
        compType = FifamCompType::OlympicQ;
    else if (Equal(str, L"Custom1"))
        compType = FifamCompType::Custom1;
    else if (Equal(str, L"Custom2"))
        compType = FifamCompType::Custom2;
    else if (Equal(str, L"Custom3"))
        compType = FifamCompType::Custom3;
    else if (Equal(str, L"Custom4"))
        compType = FifamCompType::Custom4;
    else if (Equal(str, L"Custom5"))
        compType = FifamCompType::Custom5;
    else if (Equal(str, L"Custom6"))
        compType = FifamCompType::Custom6;
    else if (Equal(str, L"Custom7"))
        compType = FifamCompType::Custom7;
    else if (Equal(str, L"Custom8"))
        compType = FifamCompType::Custom8;
    else if (Equal(str, L"Custom9"))
        compType = FifamCompType::Custom9;
    else if (Equal(str, L"Custom10"))
        compType = FifamCompType::Custom10;
    return compType != FifamCompType::Root;
}

bool GetPoolSortingByName(String const &str, FifamPoolSorting &sorting) {
    sorting = FifamPoolSorting::None;
    if (Equal(str, L"None")) sorting = FifamPoolSorting::None;
    else if (Equal(str, L"Indoor")) sorting = FifamPoolSorting::Indoor;
    else if (Equal(str, L"NatUEFA5")) sorting = FifamPoolSorting::NatUefa5;
    else if (Equal(str, L"MapNorthSouth")) sorting = FifamPoolSorting::MapNorthSouth;
    else if (Equal(str, L"MapWestEast")) sorting = FifamPoolSorting::MapWestEast;
    else if (Equal(str, L"MapLeague")) sorting = FifamPoolSorting::MapLeague;
    else if (Equal(str, L"Nationality")) sorting = FifamPoolSorting::Nationality;
    else if (Equal(str, L"BestTabPos")) sorting = FifamPoolSorting::BestTabPos;
    else if (Equal(str, L"CountryStrength")) sorting = FifamPoolSorting::CountryStrength;
    else if (Equal(str, L"UEFA5Shuffle")) sorting = FifamPoolSorting::Uefa5Shuffle;
    else if (Equal(str, L"Random")) sorting = FifamPoolSorting::Random;
    else if (Equal(str, L"Last16") || Equal(str, L"ChampionsCupLast16")) sorting = FifamPoolSorting::ChampionscupLast16;
    else if (Equal(str, L"NorthSouth")) sorting = FifamPoolSorting::NorthSouth;
    else if (Equal(str, L"ChampionsCupGroup")) sorting = FifamPoolSorting::ChampionscupGroup;
    else if (Equal(str, L"PrevPointsDiff")) sorting = FifamPoolSorting::PreviousPointsDiff;
    else if (Equal(str, L"Continent")) sorting = FifamPoolSorting::Continent;
    else if (Equal(str, L"DrawWorldCup")) sorting = FifamPoolSorting::DrawWorldCup;
    else if (Equal(str, L"DrawEuroCup")) sorting = FifamPoolSorting::DrawEuroCup;
    else if (Equal(str, L"CopaAmerica")) sorting = FifamPoolSorting::CopaAmerica;
    else return false;
    return true;
}

bool GetLeagueSortingByName(String const &str, FifamEqualPointsSorting &sorting) {
    sorting = FifamPoolSorting::None;
    if (Equal(str, L"GoalDiff") || Equal(str, L"GD")) sorting = FifamEqualPointsSorting::GoalDiff;
    else if (Equal(str, L"Goals") || Equal(str, L"G")) sorting = FifamEqualPointsSorting::Goals;
    else if (Equal(str, L"DirectMatches") || Equal(str, L"DM")) sorting = FifamEqualPointsSorting::DirectMatches;
    else return false;
    return true;
}

bool GetRoundTypeByName(String const &str, FifamRoundID &roundId) {
    roundId = FifamPoolSorting::None;
    if (Equal(str, L"None")) roundId = FifamRoundID::None;
    else if (Equal(str, L"Quali")) roundId = FifamRoundID::Quali;
    else if (Equal(str, L"Quali2")) roundId = FifamRoundID::Quali2;
    else if (Equal(str, L"Quali3")) roundId = FifamRoundID::Quali3;
    else if (Equal(str, L"PreRound1")) roundId = FifamRoundID::PreRound1;
    else if (Equal(str, L"Round1")) roundId = FifamRoundID::_1;
    else if (Equal(str, L"Round2")) roundId = FifamRoundID::_2;
    else if (Equal(str, L"Round3")) roundId = FifamRoundID::_3;
    else if (Equal(str, L"Round4")) roundId = FifamRoundID::_4;
    else if (Equal(str, L"Round5")) roundId = FifamRoundID::_5;
    else if (Equal(str, L"Group1")) roundId = FifamRoundID::Group1;
    else if (Equal(str, L"Group2")) roundId = FifamRoundID::Group2;
    else if (Equal(str, L"Last16")) roundId = FifamRoundID::Last16;
    else if (Equal(str, L"Quarterfinal")) roundId = FifamRoundID::Quarterfinal;
    else if (Equal(str, L"Semifinal")) roundId = FifamRoundID::Semifinal;
    else if (Equal(str, L"Final")) roundId = FifamRoundID::Final;
    else if (Equal(str, L"Final3rdPlace")) roundId = FifamRoundID::Final3rdPlace;
    else return false;
    return true;
}

bool ExtractCompType(String const &str, FifamCompRegion &region, FifamCompType &type) {
    region = FifamCompRegion::None;
    type = FifamCompType::Root;
    auto dotPosn = str.find(L'.');
    if (dotPosn != String::npos) {
        // comp Region.Type
        // comp Europe.ChampionsLeague
        // comp International.WorldCup
        String regionName = str.substr(0, dotPosn);
        String compTypeName = str.substr(dotPosn + 1);
        return GetContinentByName(regionName, region) && GetCompTypeByName(compTypeName, type);
    }
    // comp England
    if (!GetCompTypeByName(str, type))
        return GetContinentByName(str, region) || GetCountryByName(str, region);
    // comp WorldCup
    region = FifamCompRegion::International;
    return true;
}

bool IsCountryRegion(FifamCompRegion const &region, UInt maxCountryId) {
    return region.ToInt() > 0 && region.ToInt() <= maxCountryId;
}

bool IsContinentRegion(FifamCompRegion const &region) {
    return region.ToInt() >= 249 && region.ToInt() <= 254;
}

bool IsInternationalRegion(FifamCompRegion const &region) {
    return region.ToInt() == 255;
}

String GetBlockNameForCompetition(FifamCompID const &compId) {
    if (compId.mRegion == FifamCompRegion::Europe) {
        if (compId.mType == FifamCompType::YouthChampionsLeague)
            return L"YOUTH";
        return L"EURO";
    }
    if (compId.mRegion == FifamCompRegion::SouthAmerica) {
        if (compId.mType == FifamCompType::YouthChampionsLeague)
            return L"YOUTH";
        return L"SOUTHAM";
    }
    if (compId.mRegion == FifamCompRegion::NorthAmerica)
        return L"INTAMERICA";
    if (compId.mRegion == FifamCompRegion::Africa)
        return L"INTAFRICA";
    if (compId.mRegion == FifamCompRegion::Asia)
        return L"INTASIA";
    if (compId.mRegion == FifamCompRegion::Oceania)
        return L"INTOCEANIA";
    if (compId.mRegion == FifamCompRegion::International) {
        if (compId.mType == FifamCompType::WorldCup)
            return L"WORLD_CUP";
        if (compId.mType == FifamCompType::QualiWC)
            return L"QUALI_WC";
        if (compId.mType == FifamCompType::EuroCup)
            return L"EURO_CUP";
        if (compId.mType == FifamCompType::EuroNLQ)
            return L"Q_EURO_NL";
        if (compId.mType == FifamCompType::EuroNL)
            return L"EURO_NL";
        if (compId.mType == FifamCompType::QualiEC)
            return L"QUALI_EC";
        if (compId.mType == FifamCompType::CopaAmerica)
            return L"COPA_AMERICA";
        if (compId.mType == FifamCompType::ConfedCup)
            return L"CONFED_CUP";
        if (compId.mType == FifamCompType::U20WorldCup || compId.mType == FifamCompType::U20WCQ)
            return L"U20_WORLD_CUP";
        if (compId.mType == FifamCompType::NamCup)
            return L"NAM_CUP";
        if (compId.mType == FifamCompType::NamNL)
            return L"NAM_NL";
        if (compId.mType == FifamCompType::NamNLQ)
            return L"Q_NAM_NL";
        if (compId.mType == FifamCompType::AsiaCup)
            return L"ASIA_CUP";
        if (compId.mType == FifamCompType::AsiaCupQ)
            return L"Q_ASIA_CUP";
        if (compId.mType == FifamCompType::OfcCup)
            return L"OFC_CUP";
        if (compId.mType == FifamCompType::OfcCupQ)
            return L"Q_OFC_CUP";
        if (compId.mType == FifamCompType::AfricaCup)
            return L"AFRICA_CUP";
        if (compId.mType == FifamCompType::AfricaCupQ)
            return L"Q_AFRICA_CUP";
        if (compId.mType == FifamCompType::U17WC || compId.mType == FifamCompType::U17WCQ)
            return L"U17_WORLD_CUP";
        if (compId.mType == FifamCompType::U21EC || compId.mType == FifamCompType::U21ECQ)
            return L"U21_EC";
        if (compId.mType == FifamCompType::U19EC || compId.mType == FifamCompType::U19ECQ)
            return L"U19_EC";
        if (compId.mType == FifamCompType::U17EC || compId.mType == FifamCompType::U17ECQ)
            return L"U17_EC";
        if (compId.mType == FifamCompType::Olympic || compId.mType == FifamCompType::OlympicQ)
            return L"OLYMPIC";
        if (compId.mType == FifamCompType::Custom1)
            return L"CUSTOM_COMP_TYPE_1";
        if (compId.mType == FifamCompType::Custom2)
            return L"CUSTOM_COMP_TYPE_2";
        if (compId.mType == FifamCompType::Custom3)
            return L"CUSTOM_COMP_TYPE_3";
        if (compId.mType == FifamCompType::Custom4)
            return L"CUSTOM_COMP_TYPE_4";
        if (compId.mType == FifamCompType::Custom5)
            return L"CUSTOM_COMP_TYPE_5";
        if (compId.mType == FifamCompType::Custom6)
            return L"CUSTOM_COMP_TYPE_6";
        if (compId.mType == FifamCompType::Custom7)
            return L"CUSTOM_COMP_TYPE_7";
        if (compId.mType == FifamCompType::Custom8)
            return L"CUSTOM_COMP_TYPE_8";
        if (compId.mType == FifamCompType::Custom9)
            return L"CUSTOM_COMP_TYPE_9";
        if (compId.mType == FifamCompType::Custom10)
            return L"CUSTOM_COMP_TYPE_10";
    }
    return L"COMP";
}

struct CompContainer {
    enum Type { Root, Pool, League, Round, Cup, Level, RoundsGrid, Pots } type;
    Vector<FifamCompetition *> vec;
    Bool blockStarted = false;

    CompContainer(Type _type, Vector<FifamCompetition *> &_vec) {
        type = _type;
        vec = _vec;
        blockStarted = false;
    }

    CompContainer(Type _type, FifamCompetition *_comp) {
        type = _type;
        vec = { _comp };
        blockStarted = false;
    }

    CompContainer() {
        type = Root;
        blockStarted = false;
    }
};

bool CheckCompetitionType(FifamDatabase *db, String const &refName, FifamCompID const &compID, FifamCompDbType requiredDbType, String &error) {
    error.clear();
    FifamCompetition *comp = db->GetCompetition(compID);
    if (comp && comp->GetDbType() != requiredDbType) {
        error = L"'" + refName + L"' is not a ";
        if (requiredDbType == FifamCompDbType::Pool)
            error += L"pool";
        else if (requiredDbType == FifamCompDbType::League)
            error += L"league";
        else if (requiredDbType == FifamCompDbType::Cup)
            error += L"cup";
        else
            error += L"round";
        return false;
    }
    return true;
}

bool CheckCompetitionType(FifamDatabase *db, String const &refName, Vector<FifamCompID> const &compIDs, FifamCompDbType requiredDbType, String &error) {
    for (auto const &id : compIDs) {
        if (!CheckCompetitionType(db, refName, id, requiredDbType, error))
            return false;
    }
    return true;
}

bool ResolveReference(FifamDatabase *db, FifamCompetition *currComp, Map<String, CompContainer> const &refNames, String const &refStr,
    Vector<FifamCompID> &outIds, String &error, FifamCompDbType requiredDbType = FifamCompDbType::Root)
{
    Bool result = false;
    error.clear();
    Vector<FifamCompID> ids;
    if (currComp) {
        if (refStr == L"this") {
            ids.push_back(currComp->mID);
            result = true;
        }
        else if (refStr == L"prev") {
            for (auto const &id : currComp->mPredecessors)
                ids.push_back(id);
            result = true;
        }
        else if (refStr == L"next") {
            for (auto const &id : currComp->mSuccessors)
                ids.push_back(id);
            result = true;
        }
    }
    if (!result) {
        auto refId = refNames.find(refStr);
        if (refId != refNames.end()) {
            for (UInt i = 0; i < (*refId).second.vec.size(); i++)
                ids.push_back((*refId).second.vec[i]->mID);
            result = true;
        }
    }
    if (!result) {
        error = L"failed to resolve reference name '" + refStr + L"'";
        return false;
    }
    if (requiredDbType != FifamCompDbType::Root && !CheckCompetitionType(db, refStr, ids, requiredDbType, error))
        return false;
    for (auto const &id : ids)
        outIds.push_back(id);
    return true;
}

bool ResolveDirectReference(FifamDatabase *db, String const &refStr, FifamCompID &outId, String &error, FifamCompDbType requiredDbType = FifamCompDbType::Root) {
    error.clear();
    auto parts = Utils::Split(refStr, L'_');
    outId.SetFromInt(0);
    Bool result = false;
    if (parts.size() >= 2 && parts.size() <= 3 && SS_IsNumber(parts.back())) {
        outId.mIndex = SS_ToNumber(parts.back());
        if (parts.size() == 3) {
            if (!GetContinentByName(parts.front(), outId.mRegion)) {
                if (GetCountryByName(parts.front(), outId.mRegion) && GetCompTypeByName_DirectReference(parts[1], outId.mType))
                    result = true;
            }
            else {
                if (GetCompTypeByName_DirectReference(parts[1], outId.mType))
                    result = true;
            }
        }
        else {
            outId.mRegion = FifamCompRegion::International;
            if (GetCompTypeByName_DirectReference(parts.front(), outId.mType))
                result = true;
        }
    }
    if (!result) {
        error = L"failed to resolve reference name '" + refStr + L"'";
        return false;
    }
    if (requiredDbType != FifamCompDbType::Root && !CheckCompetitionType(db, refStr, outId, requiredDbType, error))
        return false;
    return true;
}

String GetDirectReferenceFromCompID(FifamCompID const &id, UInt gameId, Bool ucpMode) {
    if (id.mRegion == FifamCompRegion::None)
        return L"[Invalid]";
    String name;
    String typeName;
    if (id.mType == FifamCompType::League) typeName = L"League";
    else if (id.mType == FifamCompType::Pool) typeName = L"Pool";
    else if (id.mType == FifamCompType::FaCup) typeName = L"FACup";
    else if (id.mType == FifamCompType::LeagueCup) typeName = L"LeagueCup";
    else if (id.mType == FifamCompType::SuperCup) typeName = L"SuperCup";
    else if (id.mType == FifamCompType::Relegation) typeName = L"Relegation";
    else if (id.mType == FifamCompType::ChampionsLeague) typeName = L"ChampionsLeague";
    else if (id.mType == FifamCompType::UefaCup) typeName = L"UefaCup";
    else if (id.mType == FifamCompType::Toyota) typeName = L"Toyota";
    else if (id.mType == FifamCompType::EuroSuperCup) typeName = L"EuroSuperCup";
    else if (id.mType == FifamCompType::WorldClubChamp) typeName = L"WorldClubChamp";
    else if (id.mType == FifamCompType::UIC) typeName = L"UIC";
    else if (id.mType == FifamCompType::QualiWC) typeName = L"QualiWC";
    else if (id.mType == FifamCompType::QualiEC) typeName = L"QualiEC";
    else if (id.mType == FifamCompType::WorldCup) typeName = L"WorldCup";
    else if (id.mType == FifamCompType::EuroCup) typeName = L"EuroCup";
    else if (id.mType == FifamCompType::U20WorldCup) typeName = L"U20WorldCup";
    else if (id.mType == FifamCompType::ConfedCup) typeName = L"ConfedCup";
    else if (id.mType == FifamCompType::CopaAmerica) typeName = L"CopaAmerica";
    else if (id.mType == FifamCompType::ICC) typeName = L"ICC";
    else if (id.mType == FifamCompType::EuroNLQ) typeName = L"EuroNLQ";
    else if (id.mType == FifamCompType::EuroNL) typeName = L"EuroNL";
    else if (id.mType == FifamCompType::YouthChampionsLeague) typeName = L"YouthChampionsLeague";
    else if (id.mType == FifamCompType::Continental1) typeName = L"Continental1";
    else if (id.mType == FifamCompType::Continental2) typeName = L"Continental2";
    else if (id.mType == FifamCompType::NamCup) typeName = L"NamCup";
    else if (id.mType == FifamCompType::NamNL) typeName = L"NamNL";
    else if (id.mType == FifamCompType::NamNLQ) typeName = L"NamNLQ";
    else if (id.mType == FifamCompType::AsiaCup) typeName = L"AsiaCup";
    else if (id.mType == FifamCompType::AsiaCupQ) typeName = L"AsiaCupQ";
    else if (id.mType == FifamCompType::OfcCup) typeName = L"OfcCup";
    else if (id.mType == FifamCompType::OfcCupQ) typeName = L"OfcCupQ";
    else if (id.mType == FifamCompType::AfricaCup) typeName = L"AfricaCup";
    else if (id.mType == FifamCompType::AfricaCupQ) typeName = L"AfricaCupQ";
    else if (id.mType == FifamCompType::U20WCQ) typeName = L"U20WCQ";
    else if (id.mType == FifamCompType::U17WC) typeName = L"U17WC";
    else if (id.mType == FifamCompType::U17WCQ) typeName = L"U17WCQ";
    else if (id.mType == FifamCompType::U21EC) typeName = L"U21EC";
    else if (id.mType == FifamCompType::U21ECQ) typeName = L"U21ECQ";
    else if (id.mType == FifamCompType::U19EC) typeName = L"U19EC";
    else if (id.mType == FifamCompType::U19ECQ) typeName = L"U19ECQ";
    else if (id.mType == FifamCompType::U17EC) typeName = L"U17EC";
    else if (id.mType == FifamCompType::U17ECQ) typeName = L"U17ECQ";
    else if (id.mType == FifamCompType::Olympic) typeName = L"Olympic";
    else if (id.mType == FifamCompType::OlympicQ) typeName = L"OlympicQ";
    else if (id.mType == FifamCompType::Custom1) typeName = L"Custom1";
    else if (id.mType == FifamCompType::Custom2) typeName = L"Custom2";
    else if (id.mType == FifamCompType::Custom3) typeName = L"Custom3";
    else if (id.mType == FifamCompType::Custom4) typeName = L"Custom4";
    else if (id.mType == FifamCompType::Custom5) typeName = L"Custom5";
    else if (id.mType == FifamCompType::Custom6) typeName = L"Custom6";
    else if (id.mType == FifamCompType::Custom7) typeName = L"Custom7";
    else if (id.mType == FifamCompType::Custom8) typeName = L"Custom8";
    else if (id.mType == FifamCompType::Custom9) typeName = L"Custom9";
    else if (id.mType == FifamCompType::Custom10) typeName = L"Custom10";
    else return L"[Invalid]";
    String indexStr = Utils::Format(L"%u", id.mIndex);
    if (id.mRegion != FifamCompRegion::International) {
        if (id.mRegion == FifamCompRegion::Europe) name += L"Europe";
        else if (id.mRegion == FifamCompRegion::SouthAmerica) name += L"SouthAmerica";
        else if (id.mRegion == FifamCompRegion::NorthAmerica) name += L"NorthAmerica";
        else if (id.mRegion == FifamCompRegion::Africa) name += L"Africa";
        else if (id.mRegion == FifamCompRegion::Asia) name += L"Asia";
        else if (id.mRegion == FifamCompRegion::Oceania) name += L"Oceania";
        else {
            if (id.mRegion == FifamCompRegion::Albania) name += L"Albania";
            else if (id.mRegion == FifamCompRegion::Andorra) name += L"Andorra";
            else if (id.mRegion == FifamCompRegion::Armenia) name += L"Armenia";
            else if (id.mRegion == FifamCompRegion::Austria) name += L"Austria";
            else if (id.mRegion == FifamCompRegion::Azerbaijan) name += L"Azerbaijan";
            else if (id.mRegion == FifamCompRegion::Belarus) name += L"Belarus";
            else if (id.mRegion == FifamCompRegion::Belgium) name += L"Belgium";
            else if (id.mRegion == FifamCompRegion::Bosnia_Herzegovina) name += L"BosniaHerzegovina";
            else if (id.mRegion == FifamCompRegion::Bulgaria) name += L"Bulgaria";
            else if (id.mRegion == FifamCompRegion::Croatia) name += L"Croatia";
            else if (id.mRegion == FifamCompRegion::Cyprus) name += L"Cyprus";
            else if (id.mRegion == FifamCompRegion::Czech_Republic) name += L"CzechRepublic";
            else if (id.mRegion == FifamCompRegion::Denmark) name += L"Denmark";
            else if (id.mRegion == FifamCompRegion::England) name += L"England";
            else if (id.mRegion == FifamCompRegion::Estonia) name += L"Estonia";
            else if (id.mRegion == FifamCompRegion::Faroe_Islands) name += L"FaroeIslands";
            else if (id.mRegion == FifamCompRegion::Finland) name += L"Finland";
            else if (id.mRegion == FifamCompRegion::France) name += L"France";
            else if (id.mRegion == FifamCompRegion::FYR_Macedonia) name += L"NorthMacedonia";
            else if (id.mRegion == FifamCompRegion::Georgia) name += L"Georgia";
            else if (id.mRegion == FifamCompRegion::Germany) name += L"Germany";
            else if (id.mRegion == FifamCompRegion::Greece) name += L"Greece";
            else if (id.mRegion == FifamCompRegion::Hungary) name += L"Hungary";
            else if (id.mRegion == FifamCompRegion::Iceland) name += L"Iceland";
            else if (id.mRegion == FifamCompRegion::Ireland) name += L"Ireland";
            else if (id.mRegion == FifamCompRegion::Israel) name += L"Israel";
            else if (id.mRegion == FifamCompRegion::Italy) name += L"Italy";
            else if (id.mRegion == FifamCompRegion::Latvia) name += L"Latvia";
            else if (id.mRegion == FifamCompRegion::Liechtenstein) name += L"Liechtenstein";
            else if (id.mRegion == FifamCompRegion::Lithuania) name += L"Lithuania";
            else if (id.mRegion == FifamCompRegion::Luxembourg) name += L"Luxembourg";
            else if (id.mRegion == FifamCompRegion::Malta) name += L"Malta";
            else if (id.mRegion == FifamCompRegion::Moldova) name += L"Moldova";
            else if (id.mRegion == FifamCompRegion::Netherlands) name += L"Netherlands";
            else if (id.mRegion == FifamCompRegion::Northern_Ireland) name += L"NorthernIreland";
            else if (id.mRegion == FifamCompRegion::Norway) name += L"Norway";
            else if (id.mRegion == FifamCompRegion::Poland) name += L"Poland";
            else if (id.mRegion == FifamCompRegion::Portugal) name += L"Portugal";
            else if (id.mRegion == FifamCompRegion::Romania) name += L"Romania";
            else if (id.mRegion == FifamCompRegion::Russia) name += L"Russia";
            else if (id.mRegion == FifamCompRegion::San_Marino) name += L"SanMarino";
            else if (id.mRegion == FifamCompRegion::Scotland) name += L"Scotland";
            else if (id.mRegion == FifamCompRegion::Slovakia) name += L"Slovakia";
            else if (id.mRegion == FifamCompRegion::Slovenia) name += L"Slovenia";
            else if (id.mRegion == FifamCompRegion::Spain) name += L"Spain";
            else if (id.mRegion == FifamCompRegion::Sweden) name += L"Sweden";
            else if (id.mRegion == FifamCompRegion::Switzerland) name += L"Switzerland";
            else if (id.mRegion == FifamCompRegion::Turkey) name += L"Turkey";
            else if (id.mRegion == FifamCompRegion::Ukraine) name += L"Ukraine";
            else if (id.mRegion == FifamCompRegion::Wales) name += L"Wales";
            else if (id.mRegion == FifamCompRegion::Serbia) name += L"Serbia";
            else if (id.mRegion == FifamCompRegion::Argentina) name += L"Argentina";
            else if (id.mRegion == FifamCompRegion::Bolivia) name += L"Bolivia";
            else if (id.mRegion == FifamCompRegion::Brazil) name += L"Brazil";
            else if (id.mRegion == FifamCompRegion::Chile) name += L"Chile";
            else if (id.mRegion == FifamCompRegion::Colombia) name += L"Colombia";
            else if (id.mRegion == FifamCompRegion::Ecuador) name += L"Ecuador";
            else if (id.mRegion == FifamCompRegion::Paraguay) name += L"Paraguay";
            else if (id.mRegion == FifamCompRegion::Peru) name += L"Peru";
            else if (id.mRegion == FifamCompRegion::Uruguay) name += L"Uruguay";
            else if (id.mRegion == FifamCompRegion::Venezuela) name += L"Venezuela";
            else if (id.mRegion == FifamCompRegion::Anguilla) name += (gameId >= 13 && ucpMode) ? L"NewCaledonia" : L"Anguilla";
            else if (id.mRegion == FifamCompRegion::Antigua_and_Barbuda) name += L"AntiguaAndBarbuda";
            else if (id.mRegion == FifamCompRegion::Aruba) name += L"Aruba";
            else if (id.mRegion == FifamCompRegion::Bahamas) name += L"Bahamas";
            else if (id.mRegion == FifamCompRegion::Barbados) name += L"Barbados";
            else if (id.mRegion == FifamCompRegion::Belize) name += L"Belize";
            else if (id.mRegion == FifamCompRegion::Bermuda) name += L"Bermuda";
            else if (id.mRegion == FifamCompRegion::British_Virgin_Is) name += L"BritishVirginIs";
            else if (id.mRegion == FifamCompRegion::Canada) name += L"Canada";
            else if (id.mRegion == FifamCompRegion::Cayman_Islands) name += L"CaymanIslands";
            else if (id.mRegion == FifamCompRegion::Costa_Rica) name += L"CostaRica";
            else if (id.mRegion == FifamCompRegion::Cuba) name += L"Cuba";
            else if (id.mRegion == FifamCompRegion::Dominica) name += L"Dominica";
            else if (id.mRegion == FifamCompRegion::Dominican_Republic) name += L"DominicanRepublic";
            else if (id.mRegion == FifamCompRegion::El_Salvador) name += L"ElSalvador";
            else if (id.mRegion == FifamCompRegion::Grenada) name += L"Grenada";
            else if (id.mRegion == FifamCompRegion::Guatemala) name += L"Guatemala";
            else if (id.mRegion == FifamCompRegion::Guyana) name += L"Guyana";
            else if (id.mRegion == FifamCompRegion::Haiti) name += L"Haiti";
            else if (id.mRegion == FifamCompRegion::Honduras) name += L"Honduras";
            else if (id.mRegion == FifamCompRegion::Jamaica) name += L"Jamaica";
            else if (id.mRegion == FifamCompRegion::Mexico) name += L"Mexico";
            else if (id.mRegion == FifamCompRegion::Montserrat) name += L"Montserrat";
            else if (id.mRegion == FifamCompRegion::Netherlands_Antil) name += L"Curacao";
            else if (id.mRegion == FifamCompRegion::Nicaragua) name += L"Nicaragua";
            else if (id.mRegion == FifamCompRegion::Panama) name += L"Panama";
            else if (id.mRegion == FifamCompRegion::Puerto_Rico) name += L"PuertoRico";
            else if (id.mRegion == FifamCompRegion::St_Kitts_Nevis) name += L"StKittsNevis";
            else if (id.mRegion == FifamCompRegion::St_Lucia) name += L"StLucia";
            else if (id.mRegion == FifamCompRegion::St_Vincent_Gren) name += L"StVincentGren";
            else if (id.mRegion == FifamCompRegion::Surinam) name += L"Surinam";
            else if (id.mRegion == FifamCompRegion::Trinidad_Tobago) name += L"TrinidadTobago";
            else if (id.mRegion == FifamCompRegion::Turks_and_Caicos) name += L"TurksAndCaicos";
            else if (id.mRegion == FifamCompRegion::United_States) name += L"UnitedStates";
            else if (id.mRegion == FifamCompRegion::US_Virgin_Islands)
                return typeName + L"_" + L"USVirginIslands" + L"_" + indexStr;
            if (name.empty()) {
                if (id.mRegion == FifamCompRegion::Algeria) name += L"Algeria";
                else if (id.mRegion == FifamCompRegion::Angola) name += L"Angola";
                else if (id.mRegion == FifamCompRegion::Benin) name += L"Benin";
                else if (id.mRegion == FifamCompRegion::Botswana) name += L"Botswana";
                else if (id.mRegion == FifamCompRegion::Burkina_Faso) name += L"BurkinaFaso";
                else if (id.mRegion == FifamCompRegion::Burundi) name += L"Burundi";
                else if (id.mRegion == FifamCompRegion::Cameroon) name += L"Cameroon";
                else if (id.mRegion == FifamCompRegion::Cape_Verde_Islands) name += L"CapeVerdeIslands";
                else if (id.mRegion == FifamCompRegion::Central_African_Rep) name += L"CentralAfricanRep";
                else if (id.mRegion == FifamCompRegion::Chad) name += L"Chad";
                else if (id.mRegion == FifamCompRegion::Congo) name += L"Congo";
                else if (id.mRegion == FifamCompRegion::Cote_d_Ivoire) name += L"CoteDIvoire";
                else if (id.mRegion == FifamCompRegion::Djibouti) name += L"Djibouti";
                else if (id.mRegion == FifamCompRegion::DR_Congo) name += L"DRCongo";
                else if (id.mRegion == FifamCompRegion::Egypt) name += L"Egypt";
                else if (id.mRegion == FifamCompRegion::Equatorial_Guinea) name += L"EquatorialGuinea";
                else if (id.mRegion == FifamCompRegion::Eritrea) name += L"Eritrea";
                else if (id.mRegion == FifamCompRegion::Ethiopia) name += L"Ethiopia";
                else if (id.mRegion == FifamCompRegion::Gabon) name += L"Gabon";
                else if (id.mRegion == FifamCompRegion::Gambia) name += L"Gambia";
                else if (id.mRegion == FifamCompRegion::Ghana) name += L"Ghana";
                else if (id.mRegion == FifamCompRegion::Guinea) name += L"Guinea";
                else if (id.mRegion == FifamCompRegion::Guinea_Bissau) name += L"GuineaBissau";
                else if (id.mRegion == FifamCompRegion::Kenya) name += L"Kenya";
                else if (id.mRegion == FifamCompRegion::Lesotho) name += L"Lesotho";
                else if (id.mRegion == FifamCompRegion::Liberia) name += L"Liberia";
                else if (id.mRegion == FifamCompRegion::Libya) name += L"Libya";
                else if (id.mRegion == FifamCompRegion::Madagascar) name += L"Madagascar";
                else if (id.mRegion == FifamCompRegion::Malawi) name += L"Malawi";
                else if (id.mRegion == FifamCompRegion::Mali) name += L"Mali";
                else if (id.mRegion == FifamCompRegion::Mauritania) name += L"Mauritania";
                else if (id.mRegion == FifamCompRegion::Mauritius) name += L"Mauritius";
                else if (id.mRegion == FifamCompRegion::Morocco) name += L"Morocco";
                else if (id.mRegion == FifamCompRegion::Mozambique) name += L"Mozambique";
                else if (id.mRegion == FifamCompRegion::Namibia) name += L"Namibia";
                else if (id.mRegion == FifamCompRegion::Niger) name += L"Niger";
                else if (id.mRegion == FifamCompRegion::Nigeria) name += L"Nigeria";
                else if (id.mRegion == FifamCompRegion::Rwanda) name += L"Rwanda";
                else if (id.mRegion == FifamCompRegion::Sao_Tome_e_Principe) name += L"SaoTomeEPrincipe";
                else if (id.mRegion == FifamCompRegion::Senegal) name += L"Senegal";
                else if (id.mRegion == FifamCompRegion::Seychelles) name += L"Seychelles";
                else if (id.mRegion == FifamCompRegion::Sierra_Leone) name += L"SierraLeone";
                else if (id.mRegion == FifamCompRegion::Somalia) name += L"Somalia";
                else if (id.mRegion == FifamCompRegion::South_Africa) name += L"SouthAfrica";
                else if (id.mRegion == FifamCompRegion::Sudan) name += L"Sudan";
                else if (id.mRegion == FifamCompRegion::Swaziland) name += L"Eswatini";
                else if (id.mRegion == FifamCompRegion::Tanzania) name += L"Tanzania";
                else if (id.mRegion == FifamCompRegion::Togo) name += L"Togo";
                else if (id.mRegion == FifamCompRegion::Tunisia) name += L"Tunisia";
                else if (id.mRegion == FifamCompRegion::Uganda) name += L"Uganda";
                else if (id.mRegion == FifamCompRegion::Zambia) name += L"Zambia";
                else if (id.mRegion == FifamCompRegion::Zimbabwe) name += L"Zimbabwe";
                else if (id.mRegion == FifamCompRegion::Afghanistan) name += L"Afghanistan";
                else if (id.mRegion == FifamCompRegion::Bahrain) name += L"Bahrain";
                else if (id.mRegion == FifamCompRegion::Bangladesh) name += L"Bangladesh";
                else if (id.mRegion == FifamCompRegion::Bhutan) name += L"Bhutan";
                else if (id.mRegion == FifamCompRegion::Brunei_Darussalam) name += L"BruneiDarussalam";
                else if (id.mRegion == FifamCompRegion::Cambodia) name += L"Cambodia";
                else if (id.mRegion == FifamCompRegion::China_PR) name += L"China";
                else if (id.mRegion == FifamCompRegion::Taiwan) name += L"Taiwan";
                else if (id.mRegion == FifamCompRegion::Guam) name += L"Guam";
                else if (id.mRegion == FifamCompRegion::Hong_Kong) name += L"HongKong";
                else if (id.mRegion == FifamCompRegion::India) name += L"India";
                else if (id.mRegion == FifamCompRegion::Indonesia) name += L"Indonesia";
                else if (id.mRegion == FifamCompRegion::Iran) name += L"Iran";
                else if (id.mRegion == FifamCompRegion::Iraq) name += L"Iraq";
                else if (id.mRegion == FifamCompRegion::Japan) name += L"Japan";
                else if (id.mRegion == FifamCompRegion::Jordan) name += L"Jordan";
                else if (id.mRegion == FifamCompRegion::Kazakhstan) name += L"Kazakhstan";
                else if (id.mRegion == FifamCompRegion::Korea_DPR) name += L"KoreaDPR";
                else if (id.mRegion == FifamCompRegion::Korea_Republic) name += L"KoreaRepublic";
                else if (id.mRegion == FifamCompRegion::Kuwait) name += L"Kuwait";
                else if (id.mRegion == FifamCompRegion::Kyrgyzstan) name += L"Kyrgyzstan";
                else if (id.mRegion == FifamCompRegion::Laos) name += L"Laos";
                else if (id.mRegion == FifamCompRegion::Lebanon) name += L"Lebanon";
                else if (id.mRegion == FifamCompRegion::Macao) name += L"Macao";
                else if (id.mRegion == FifamCompRegion::Malaysia) name += L"Malaysia";
                else if (id.mRegion == FifamCompRegion::Maldives) name += L"Maldives";
                else if (id.mRegion == FifamCompRegion::Mongolia) name += L"Mongolia";
                else if (id.mRegion == FifamCompRegion::Myanmar) name += L"Myanmar";
                else if (id.mRegion == FifamCompRegion::Nepal) name += L"Nepal";
                else if (id.mRegion == FifamCompRegion::Oman) name += L"Oman";
                else if (id.mRegion == FifamCompRegion::Pakistan) name += L"Pakistan";
                else if (id.mRegion == FifamCompRegion::Palestinian_Authority) name += L"PalestinianAuthority";
                else if (id.mRegion == FifamCompRegion::Philippines) name += L"Philippines";
                else if (id.mRegion == FifamCompRegion::Qatar) name += L"Qatar";
                else if (id.mRegion == FifamCompRegion::Saudi_Arabia) name += L"SaudiArabia";
                else if (id.mRegion == FifamCompRegion::Singapore) name += L"Singapore";
                else if (id.mRegion == FifamCompRegion::Sri_Lanka) name += L"SriLanka";
                else if (id.mRegion == FifamCompRegion::Syria) name += L"Syria";
                else if (id.mRegion == FifamCompRegion::Tajikistan) name += L"Tajikistan";
                else if (id.mRegion == FifamCompRegion::Thailand) name += L"Thailand";
                else if (id.mRegion == FifamCompRegion::Turkmenistan) name += L"Turkmenistan";
                else if (id.mRegion == FifamCompRegion::United_Arab_Emirates) name += L"UnitedArabEmirates";
                else if (id.mRegion == FifamCompRegion::Uzbekistan) name += L"Uzbekistan";
                else if (id.mRegion == FifamCompRegion::Vietnam) name += L"Vietnam";
                else if (id.mRegion == FifamCompRegion::Yemen) name += L"Yemen";
                else if (id.mRegion == FifamCompRegion::American_Samoa) name += L"AmericanSamoa";
                else if (id.mRegion == FifamCompRegion::Australia) name += L"Australia";
                else if (id.mRegion == FifamCompRegion::Cook_Islands) name += L"CookIslands";
                else if (id.mRegion == FifamCompRegion::Fiji) name += L"Fiji";
                else if (id.mRegion == FifamCompRegion::New_Zealand) name += L"NewZealand";
                else if (id.mRegion == FifamCompRegion::Papua_New_Guinea) name += L"PapuaNewGuinea";
                else if (id.mRegion == FifamCompRegion::Samoa) name += L"Samoa";
                else if (id.mRegion == FifamCompRegion::Solomon_Islands) name += L"SolomonIslands";
                else if (id.mRegion == FifamCompRegion::Tahiti) name += L"Tahiti";
                else if (id.mRegion == FifamCompRegion::Tonga) name += L"Tonga";
                else if (id.mRegion == FifamCompRegion::Vanuatu) name += L"Vanuatu";
                else if (id.mRegion == FifamCompRegion::Gibraltar) name += L"Gibraltar";
                else if (id.mRegion == FifamCompRegion::Montenegro) name += L"Montenegro";
                else if (id.mRegion == FifamCompRegion::Greenland) name += (gameId >= 13 && ucpMode) ? L"Kosovo" : L"Greenland";
                else return L"[Invalid]";
            }
        }
        name += L"_";
    }

    name += typeName + L"_" + indexStr;
    return name;
}

bool ResolveReferenceList(FifamDatabase *db, FifamCompetition *currComp, Map<String, CompContainer> const &refNames, TokenBase *token,
    Vector<FifamCompID> &outIds, UInt maxSize, String &error, FifamCompDbType requiredDbType = FifamCompDbType::Root, bool clearIds = true)
{
    error.clear();
    if (clearIds)
        outIds.clear();
    bool failed = false;
    if (token->type() == TOKEN_REFERENCE) {
        if (!ResolveReference(db, currComp, refNames, token->as<TokenReference>()->str, outIds, error, requiredDbType))
            return false;
    }
    else if (token->type() == TOKEN_DIRECT_REFERENCE) {
        FifamCompID compId;
        if (ResolveDirectReference(db, token->as<TokenDirectReference>()->str, compId, error, requiredDbType))
            outIds.push_back(compId);
        else
            return false;
    }
    else if (token->type() == TOKEN_ARRAY) {
        TokenArray *ary = token->as<TokenArray>();
        if (ary->values.size() > maxSize) {
            error = L"too many elements in array, only " + Utils::Format(L"%d", maxSize) + L" elements are allowed";
            return false;
        }
        for (UInt c = 0; c < ary->values.size(); c++) {
            if (ary->values[c]->type() == TOKEN_REFERENCE) {
                if (!ResolveReference(db, currComp, refNames, ary->values[c]->as<TokenReference>()->str, outIds, error, requiredDbType))
                    return false;
            }
            else if (ary->values[c]->type() == TOKEN_DIRECT_REFERENCE) {
                FifamCompID compId;
                if (ResolveDirectReference(db, ary->values[c]->as<TokenDirectReference>()->str, compId, error, requiredDbType))
                    outIds.push_back(compId);
                else
                    return false;
            }
            else { // not a reference, not a direct reference
                error = L"expected reference or direct reference";
                return false;
            }
        }
    }
    else {
        error = L"expected reference, direct reference or reference array";
        return false;
    }
    return true;
}

bool TokenToBonuses(TokenArray *token, Array<UInt, 4> &outBonuses, String &error, Bool allowTwo = true) {
    if (allowTwo) {
        if (token->values.size() != 4 && token->values.size() != 2) {
            error = L"bonuses array should contain 2 or 4 numbers";
            return false;
        }
    }
    else {
        if (token->values.size() != 4) {
            error = L"bonuses array should contain 4 numbers";
            return false;
        }
    }
    if (token->values.size() == 2) {
        if (token->values[0]->type() != TOKEN_NUMBER || token->values[1]->type() != TOKEN_NUMBER) {
            error = L"bonuses array should contain only numbers";
            return false;
        }
        outBonuses[1] = token->values[0]->as<TokenNumber>()->number;
        outBonuses[3] = token->values[1]->as<TokenNumber>()->number;
    }
    else {
        for (UInt v = 0; v < token->values.size(); v++) {
            if (token->values[v]->type() != TOKEN_NUMBER) {
                error = L"bonuses array should contain only numbers";
                return false;
            }
            outBonuses[v] = token->values[v]->as<TokenNumber>()->number;
        }
    }
    return true;
}

bool TokenToMatchdays(TokenBase *token, Vector<UShort> &outMatchdays, Vector<UInt> &arraySizes, String &error, Bool &hasArrays, Bool allowArrays = false) {
    outMatchdays.clear();
    arraySizes.clear();
    hasArrays = false;
    if (token->type() == TOKEN_NUMBER)
        outMatchdays.push_back(token->as<TokenNumber>()->number);
    else if (token->type() == TOKEN_ARRAY) {
        TokenArray *ary = token->as<TokenArray>();
        for (UInt i = 0; i < ary->values.size(); i++) {
            if (ary->values[i]->type() == TOKEN_NUMBER) {
                outMatchdays.push_back(ary->values[i]->as<TokenNumber>()->number);
                if (allowArrays)
                    arraySizes.push_back(1);
            }
            else if (ary->values[i]->type() == TOKEN_ARRAY) {
                if (!allowArrays) {
                    error = L"expected number inside array";
                    outMatchdays.clear();
                    return false;
                }
                hasArrays = true;
                TokenArray *m = ary->values[i]->as<TokenArray>();
                for (UInt j = 0; j < m->values.size(); j++) {
                    if (m->values[j]->type() == TOKEN_NUMBER)
                        outMatchdays.push_back(m->values[j]->as<TokenNumber>()->number);
                    else {
                        error = L"expected number inside array";
                        outMatchdays.clear();
                        arraySizes.clear();
                        return false;
                    }
                }
                arraySizes.push_back(m->values.size());
            }
        }
    }
    else {
        error = L"expected number or array";
        return false;
    }
    for (auto i : outMatchdays) {
        if (i <= 0 || i > 730) {
            error = L"wrong match date: only numbers from 1 to 730 are allowed";
            outMatchdays.clear();
            arraySizes.clear();
            return false;
        }
    }
    return true;
}

struct Instruction {
    FifamInstructionID id;
    String specialId;
    Vector<TokenBase *> arguments;
};

FifamCompID GetAvailableCompID(FifamDatabase *db, FifamCompRegion const &region, FifamCompType const &compType) {
    FifamCompID id = { region, compType, 0u };
    for (UInt i = 0; i < 32767; i++) {
        auto it = db->mCompMap.find(id);
        if (it == db->mCompMap.end())
            return id;
        id.mIndex = i + 1;
    }
    id.SetFromInt(0);
    return id;
}

bool ScriptEngine::Compile(String const &in, String &out, String &outFixture, String &outData, Vector<CompDesc> &outComps, Int gameId, Bool universalMode, Bool ucpMode, String const &compilerVersion, CompilerOptions const &options, Vector<String> &definitions, UidResolver *uidResolver) {
    FifamDatabase db;

    Vector<TokenBase *> tokens;
    String error;

    //String readerIn = in;
    //
    //size_t currpos = 0;
    //size_t ipos = in.find(L"#include ");
    //while (ipos != String::npos) {
    //    readerIn += in.substr(currpos, ipos - currpos);
    //}

    ExtractTokens(in, tokens, error);

    outComps.clear();

    Vector<FifamCompetition *> gComps;
    String scriptName, scriptVersion, scriptAuthor, scriptDesc;
    Map<String, TokenBase *> defsTokens;
    UInt gNextCustomCompIndex = 0;
    FifamCompType gCompType;
    FifamCompRegion gCompRegion;
    String errorTitle;
    String errorDesc;
    std::stack<CompContainer> gStack;
    Map<String, CompContainer> gCompRefNames;
    UInt MaxCountryId = 207;
    Vector<Pair<String, String>> replacements;
    String blockName;
    Int blockStartIndex = 0;
    Bool writerStopped = false;

    UInt pmNumCountries = 207;
    UInt pmNumCountriesEurope = 53;
    UInt pmNumCountriesNorthAmerica = 35;
    UInt pmNumCountriesOceania = 10;

    if (gameId <= 7) {
        pmNumCountries -= 1;
        pmNumCountriesEurope -= 1;
    }
    else if (gameId >= 13 && ucpMode) {
        pmNumCountriesEurope += 2;
        pmNumCountriesNorthAmerica -= 1;
        pmNumCountriesOceania += 1;
    }

    Vector<Pair<String, UInt>> predefinedMacros = {
        { L"__MAX_LEAGUE_TEAMS__", 24 },
        { L"__MAX_POOL_TEAMS__", 9999 },
        { L"__NUM_COUNTRIES__", pmNumCountries },
        { L"__NUM_COUNTRIES_EUROPE__", pmNumCountriesEurope },
        { L"__NUM_COUNTRIES_SOUTH_AMERICA__", 10 },
        { L"__NUM_COUNTRIES_NORTH_AMERICA__", pmNumCountriesNorthAmerica },
        { L"__NUM_COUNTRIES_AFRICA__", 52 },
        { L"__NUM_COUNTRIES_ASIA__", 45 },
        { L"__NUM_COUNTRIES_OCEANIA__", pmNumCountriesOceania }
    };

    for (auto const &pm : predefinedMacros) {
        definitions.push_back(pm.first);
        for (UInt ti = 0; ti < tokens.size(); ti++) {
            if (tokens[ti]->type() == TOKEN_NAME && tokens[ti]->as<TokenName>()->name == pm.first) {
                delete tokens[ti];
                tokens[ti] = new TokenNumber(pm.second);
            }
        }
    }

    auto CreateCompetition = [&](FifamCompDbType dbType, FifamCompRegion const &region, FifamCompType const &compType) {
        FifamCompetition *comp = nullptr;
        switch (dbType.ToInt()) {
        case FifamCompDbType::Root:
            comp = new FifamCompRoot();
            break;
        case FifamCompDbType::Pool:
            comp = new FifamCompPool();
            break;
        case FifamCompDbType::League:
            comp = new FifamCompLeague();
            break;
        case FifamCompDbType::Round:
            comp = new FifamCompRound();
            break;
        case FifamCompDbType::Cup:
            comp = new FifamCompCup();
            break;
        }
        if (comp) {
            comp->mID = FifamCompID(region, compType, 65535);
            comp->SetName(L"Competition");
            if (writerStopped)
                comp->SetProperty(L"dontWrite", true);
            gComps.push_back(comp);
        }
        return comp;
    };

    for (UInt i = 0; i < tokens.size(); i++) {
        if (!error.empty()) {
            errorTitle = error;
            errorDesc = L"";
            break;
        }
        if (!errorTitle.empty())
            break;
        TokenBase *t = tokens[i];
        UInt tokensLeft = tokens.size() - 1 - i;

        if (t->type() == TOKEN_OPEN) {
            if (!gStack.empty() && !gStack.top().blockStarted)
                gStack.top().blockStarted = true;
            else {
                errorTitle = L"'{': unexpected block operator";
                errorDesc = L"The block-opening operator ('{') must follow competition description. For example: 'pool ... { ... }'";
                break;
            }
        }
        else if (t->type() == TOKEN_CLOSE) {
            if (!gStack.empty() && gStack.top().blockStarted)
                gStack.pop();
            else {
                errorTitle = L"'}': unexpected block operator";
                errorDesc = L"The block-closing operator ('}') must close current block. For example: 'pool ... { ... }'";
                break;
            }
        }
        else if (t->type() == TOKEN_NAME && (Utils::StartsWith(t->as<TokenName>()->name, L"#"))) {
            if (gStack.empty()) {
                String directiveName = t->as<TokenName>()->name.substr(1);
                if (directiveName == L"name") {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_STRING) {
                        scriptName = tokens[i + 1]->as<TokenString>()->str;
                        i += 1;
                    }
                    else {
                        errorTitle = L"#name: expected string";
                        errorDesc = L"";
                        break;
                    }
                }
                else if (directiveName == L"version") {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_STRING) {
                        scriptVersion = tokens[i + 1]->as<TokenString>()->str;
                        i += 1;
                    }
                    else {
                        errorTitle = L"#version: expected string";
                        errorDesc = L"";
                        break;
                    }
                }
                else if (directiveName == L"author") {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_STRING) {
                        scriptAuthor = tokens[i + 1]->as<TokenString>()->str;
                        i += 1;
                    }
                    else {
                        errorTitle = L"#author: expected string";
                        errorDesc = L"";
                        break;
                    }
                }
                else if (directiveName == L"desc" || directiveName == L"description") {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_STRING) {
                        scriptDesc = tokens[i + 1]->as<TokenString>()->str;
                        i += 1;
                    }
                    else {
                        errorTitle = L"#desc: expected string";
                        errorDesc = L"";
                        break;
                    }
                }
                else if (directiveName == L"define") {
                    if (tokensLeft >= 2 && tokens[i + 1]->type() == TOKEN_NAME) {
                        if (!Utils::StartsWith(tokens[i + 1]->as<TokenName>()->name, L"#") && !Utils::StartsWith(tokens[i + 1]->as<TokenName>()->name, L"__")) {
                            if (tokens[i + 2]->type() != TOKEN_NUMBER && tokens[i + 2]->type() != TOKEN_NUMBER_2D
                                && tokens[i + 2]->type() != TOKEN_STRING && tokens[i + 2]->type() != TOKEN_ARRAY
                                && tokens[i + 2]->type() != TOKEN_REFERENCE && tokens[i + 2]->type() != TOKEN_DIRECT_REFERENCE)
                            {
                                errorTitle = L"#define: only numbers, strings, references and arrays are allowed";
                                errorDesc = L"";
                                break;
                            }
                            // replace for all following tokens
                            bool constantRedefinition = false;
                            for (UInt ti = i + 3; ti < tokens.size(); ti++) {
                                if (tokens[ti]->type() == TOKEN_NAME) {
                                    if (tokens[ti]->as<TokenName>()->name == L"#define" && (ti + 1) < tokens.size() &&
                                        tokens[ti + 1]->type() == TOKEN_NAME && defsTokens.find(tokens[ti + 1]->as<TokenName>()->name) != defsTokens.end())
                                    {
                                        errorTitle = L"constant redefinition";
                                        errorDesc = L"constant '" + tokens[ti + 1]->as<TokenName>()->name + L"' was already defined";
                                        constantRedefinition = true;
                                        break;
                                    }
                                    if (tokens[ti]->as<TokenName>()->name == tokens[i + 1]->as<TokenName>()->name) {
                                        delete tokens[ti];
                                        tokens[ti] = tokens[i + 2]->copy();
                                    }
                                }
                            }
                            if (constantRedefinition)
                                break;
                            defsTokens[tokens[i + 1]->as<TokenName>()->name] = tokens[i + 2];
                            definitions.push_back(tokens[i + 1]->as<TokenName>()->name);
                            i += 2;
                        }
                        else {
                            errorTitle = L"#define: constant name can't start with '#' and '__' symbols";
                            errorDesc = L"";
                            break;
                        }
                    }

                    else {
                        errorTitle = L"#define: expected constant name and constant value";
                        errorDesc = L"";
                        break;
                    }
                }
                else if (directiveName == L"replace") {
                    if (tokensLeft >= 2 && tokens[i + 1]->type() == TOKEN_STRING && tokens[i + 2]->type() == TOKEN_STRING) {
                        replacements.emplace_back(tokens[i + 1]->as<TokenString>()->str, tokens[i + 2]->as<TokenString>()->str);
                        i += 2;
                    }
                    else {
                        errorTitle = L"#replace: expected searched string and string replacement";
                        errorDesc = L"";
                        break;
                    }
                }
                else if (directiveName == L"blockname") {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_STRING) {
                        if (tokens[i + 1]->as<TokenString>()->str.empty()) {
                            errorTitle = L"#blockname: empty string is not allowed";
                            errorDesc = L"";
                            break;
                        }
                        blockName = tokens[i + 1]->as<TokenString>()->str;
                        i += 1;
                    }
                    else {
                        errorTitle = L"#blockname: expected string";
                        errorDesc = L"";
                        break;
                    }
                }
                else if (directiveName == L"blockstartindex") {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        blockStartIndex = tokens[i + 1]->as<TokenNumber>()->number;
                        i += 1;
                    }
                    else {
                        errorTitle = L"#blockstartindex: expected number";
                        errorDesc = L"";
                        break;
                    }
                }
                else if (directiveName == L"stopwriter") {
                    writerStopped = true;
                }
                else if (directiveName == L"startwriter") {
                    writerStopped = false;
                }
                else {
                    errorTitle = L"'" + directiveName + L"': unknown directive name";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"directives can be used only in global space";
                errorDesc = L"";
                break;
            }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"comp" || t->as<TokenName>()->name == L"competition")) {
            if (gStack.empty()) {
                // parse competition Europe.ChampionsLeague
                //       comp England
                if (tokensLeft >= 1) {
                    if (tokens[i + 1]->type() == TOKEN_NAME) {
                        if (ExtractCompType(tokens[i + 1]->as<TokenName>()->name, gCompRegion, gCompType)) {
                            auto comp = CreateCompetition(FifamCompDbType::Root, gCompRegion, FifamCompType::Root);
                            gStack.emplace(CompContainer::Root, comp);
                            i += 1;
                            if (tokensLeft >= 2 && tokens[i + 1]->type() == TOKEN_STRING) {
                                String compTypeStr = tokens[i + 1]->as<TokenString>()->str;
                                if (!compTypeStr.empty()) {
                                    if (gNextCustomCompIndex < 10) {
                                        gCompType.SetFromInt(FifamCompType::Custom1 + gNextCustomCompIndex);
                                        replacements.emplace_back(gCompType.ToStr(), compTypeStr);
                                        gNextCustomCompIndex++;
                                        i += 1;
                                    }
                                    else {
                                        errorTitle = L"unable to allocate new custom competition id";
                                        errorDesc = L"reached custom competition id limit (10)";
                                        break;
                                    }
                                }
                                else {
                                    errorTitle = L"global competition name error";
                                    errorDesc = L"comp: empty names are not allowed";
                                    break;
                                }
                            }
                            continue;
                        }
                        else {
                            errorTitle = L"'" + tokens[i + 1]->as<TokenName>()->name + L"': unknown enum type";
                            errorDesc = L"'" + tokens[i + 1]->as<TokenName>()->name + L"' is unknown. Please use the correct enumeration value. For example, 'Europe.ChampionsLeague', England'";
                            break;
                        }
                    }
                    else if (tokens[i + 1]->type() == TOKEN_STRING) {
                        String compTypeStr = tokens[i + 1]->as<TokenString>()->str;
                        if (!compTypeStr.empty()) {
                            if (gNextCustomCompIndex < 10) {
                                gCompRegion = FifamCompRegion::International;
                                gCompType.SetFromInt(FifamCompType::Custom1 + gNextCustomCompIndex);
                                replacements.emplace_back(gCompType.ToStr(), compTypeStr);
                                gNextCustomCompIndex++;
                                auto comp = CreateCompetition(FifamCompDbType::Root, gCompRegion, FifamCompType::Root);
                                gStack.emplace(CompContainer::Root, comp);
                                i += 1;
                                continue;
                            }
                            else {
                                errorTitle = L"unable to allocate new custom competition id";
                                errorDesc = L"reached custom competition id limit (10)";
                                break;
                            }
                        }
                        else {
                            errorTitle = L"global competition name error";
                            errorDesc = L"comp: empty names are not allowed";
                            break;
                        }

                    }
                }
                else {
                    errorTitle = L"'" + t->as<TokenName>()->name + L"': expected enum type or string";
                    errorDesc = L"'" + t->as<TokenName>()->name + L"' must be followed with competition-type-enumeration or competition custom id (string). For example, '" +
                        t->as<TokenName>()->name + L" Europe.ChampionsLeague', '" + t->as<TokenName>()->name + L" England', '" + t->as<TokenName>()->name + L" \"CUSTOM_TYPE\"'";
                    break;
                }
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"' must placed in global space";
                errorDesc = L"'" + t->as<TokenName>()->name + L"' must be placed in global space";
                break;
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"pool") {
            if (!gStack.empty() && gStack.top().blockStarted) {
                FifamCompType compType = FifamCompType::Pool;
                if (!IsCountryRegion(gCompRegion, MaxCountryId))
                    compType = gCompType;
                auto comp = CreateCompetition(FifamCompDbType::Pool, gCompRegion, compType);
                if (!IsCountryRegion(comp->mID.mRegion, MaxCountryId))
                    comp->AsPool()->mNumPools = 1;
                if (gStack.size() > 1) {
                    Vector<FifamCompetition *> *predComps = new Vector<FifamCompetition *>;
                    for (auto p : gStack.top().vec)
                        predComps->push_back(p);
                    comp->SetProperty<Vector<FifamCompetition *> *>(L"pred_comps", predComps);
                }
                if (!gStack.top().vec.empty())
                    comp->mCompetitionLevel = gStack.top().vec.back()->mCompetitionLevel;
                gStack.emplace(CompContainer::Pool, comp);
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"': unexpected keyword";
                errorDesc = L"'" + t->as<TokenName>()->name + L"' must be placed inside global competition block";
                break;
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"round") {
            if (!gStack.empty() && gStack.top().blockStarted) {
                FifamCompType compType = FifamCompType::Relegation;
                if (!IsCountryRegion(gCompRegion, MaxCountryId))
                    compType = gCompType;
                auto comp = CreateCompetition(FifamCompDbType::Round, gCompRegion, compType);
                comp->AsRound()->mRoundType = FifamRoundID::_1;
                if (gStack.size() > 1) {
                    Vector<FifamCompetition *> *predComps = new Vector<FifamCompetition *>;
                    for (auto p : gStack.top().vec)
                        predComps->push_back(p);
                    comp->SetProperty<Vector<FifamCompetition *> *>(L"pred_comps", predComps);
                }
                if (!gStack.top().vec.empty())
                    comp->mCompetitionLevel = gStack.top().vec.back()->mCompetitionLevel;
                gStack.emplace(CompContainer::Round, comp);
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"': unexpected keyword";
                errorDesc = L"'" + t->as<TokenName>()->name + L"' must be placed inside competition block";
                break;
            }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"league" || t->as<TokenName>()->name == L"group")) {
            if (!gStack.empty() && gStack.top().blockStarted) {
                FifamCompType compType = FifamCompType::League;
                if (!IsCountryRegion(gCompRegion, MaxCountryId))
                    compType = gCompType;
                auto comp = CreateCompetition(FifamCompDbType::League, gCompRegion, compType);
                if (t->as<TokenName>()->name == L"group")
                    comp->SetProperty<Bool>(L"is_group", true);
                if (!IsCountryRegion(comp->mID.mRegion, MaxCountryId)) {
                    comp->AsLeague()->mTransferMarketMp = 100;
                    comp->AsLeague()->mAttendanceMp = 100;
                }
                comp->AsLeague()->mNumRounds = 1;
                comp->AsLeague()->mFreeAgentsCanBeTranferredAtAnyTime = true;
                comp->AsLeague()->mForeignFreeAgentsCanBeTransferredAtAnyTime = true;
                if (!IsCountryRegion(comp->mID.mRegion, MaxCountryId))
                    comp->AsLeague()->mPlayerLoansCanBeCancelledAtAnyTime = true;
                if (IsCountryRegion(comp->mID.mRegion, MaxCountryId)) {
                    comp->AsLeague()->mShortTermLoansTotalLimit = 15;
                    comp->AsLeague()->mShortTermLoansSimultaneosLimit = 15;
                    comp->AsLeague()->mLongTermLoansTotalLimit = 15;
                    comp->AsLeague()->mLongTermLoansSimultaneosLimit = 15;
                    comp->AsLeague()->mOverallTotalLoansLimit = 15;
                    comp->AsLeague()->mOverallTotalSimultaneosLoansLimit = 15;
                    comp->AsLeague()->mLoanedPlayersPerSeason = 15;
                    comp->AsLeague()->mLoanPlayerPerSquad = 15;
                }
                else {
                    comp->AsLeague()->mShortTermLoansTotalLimit = 8;
                    comp->AsLeague()->mShortTermLoansSimultaneosLimit = 4;
                    comp->AsLeague()->mLongTermLoansTotalLimit = 4;
                    comp->AsLeague()->mLongTermLoansSimultaneosLimit = 4;
                    comp->AsLeague()->mOverallTotalLoansLimit = 15;
                    comp->AsLeague()->mOverallTotalSimultaneosLoansLimit = 15;
                    comp->AsLeague()->mLoanedPlayersPerSeason = 6;
                    comp->AsLeague()->mLoanPlayerPerSquad = 5;
                }
                if (IsCountryRegion(comp->mID.mRegion, MaxCountryId)) {
                    comp->AsLeague()->mSimLoanOtherLeague = 15;
                    comp->AsLeague()->mSimLoanSameLeague = 15;
                    comp->AsLeague()->mLoanOtherLeagueCount = 15;
                    comp->AsLeague()->mLoanSameLeagueCount = 15;
                }
                else {
                    comp->AsLeague()->mSimLoanOtherLeague = 4;
                    comp->AsLeague()->mSimLoanSameLeague = 15;
                    comp->AsLeague()->mLoanOtherLeagueCount = 2;
                    comp->AsLeague()->mLoanSameLeagueCount = 0;
                }
                if (gStack.size() > 1) {
                    Vector<FifamCompetition *> *predComps = new Vector<FifamCompetition *>;
                    for (auto p : gStack.top().vec)
                        predComps->push_back(p);
                    comp->SetProperty<Vector<FifamCompetition *> *>(L"pred_comps", predComps);
                }
                if (!gStack.top().vec.empty()) {
                    comp->mCompetitionLevel = gStack.top().vec.back()->mCompetitionLevel;
                    if (gStack.top().vec.back()->GetDbType() == FifamCompDbType::League)
                        comp->AsLeague()->mLeagueLevel = gStack.top().vec.back()->AsLeague()->mLeagueLevel;
                    else
                        comp->AsLeague()->mLeagueLevel = gStack.top().vec.back()->mCompetitionLevel;
                }
                gStack.emplace(CompContainer::League, comp);
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"': unexpected keyword";
                errorDesc = L"'" + t->as<TokenName>()->name + L"' must be placed inside competition block";
                break;
            }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"cup" || t->as<TokenName>()->name == L"league_cup" || t->as<TokenName>()->name == L"super_cup")) {
            if (!gStack.empty() && gStack.top().blockStarted) {
                FifamCompType compType;
                if (t->as<TokenName>()->name == L"cup")
                    compType = FifamCompType::FaCup;
                else if (t->as<TokenName>()->name == L"league_cup")
                    compType = FifamCompType::LeagueCup;
                else
                    compType = FifamCompType::SuperCup;
                if (!IsCountryRegion(gCompRegion, MaxCountryId))
                    compType = gCompType;
                auto comp = CreateCompetition(FifamCompDbType::Cup, gCompRegion, compType);
                if (gStack.size() > 1) {
                    Vector<FifamCompetition *> *predComps = new Vector<FifamCompetition *>;
                    for (auto p : gStack.top().vec)
                        predComps->push_back(p);
                    comp->SetProperty<Vector<FifamCompetition *> *>(L"pred_comps", predComps);
                }
                if (!gStack.top().vec.empty())
                    comp->mCompetitionLevel = gStack.top().vec.back()->mCompetitionLevel;
                gStack.emplace(CompContainer::Cup, comp);
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"': unexpected keyword";
                errorDesc = L"'" + t->as<TokenName>()->name + L"' must be placed inside competition block";
                break;
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"index") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                    Int index = tokens[i + 1]->as<TokenNumber>()->number;
                    if (index >= 0 && index <= 32767) {
                        for (auto p : gStack.top().vec)
                            p->SetProperty<Int>(L"index", index);
                        i += 1;
                    }
                    else {
                        errorTitle = L"'index': incorrect number";
                        errorDesc = L"'index': only values from 0 to 32767 are allowed";
                        break;
                    }
                }
                else {
                    errorTitle = L"'index': expected number";
                    errorDesc = L"'index' must be followed with competition index. For example, 'index 22'";
                    break;
                }
            }
            else {
                errorTitle = L"'index' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"relegation") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Pool || gStack.top().type == CompContainer::League || gStack.top().type == CompContainer::Round) {
                    if (IsCountryRegion(gCompRegion, MaxCountryId)) {
                        for (auto p : gStack.top().vec)
                            p->SetProperty<Bool>(L"relegation", true);
                    }
                    else {
                        errorTitle = L"'relegation' keyword can't be used with continental or international competitions";
                        errorDesc = L"";
                    }
                }
                else {
                    errorTitle = L"'relegation' keyword works only with pool, league and round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'relegation' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"teams") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                    UInt numTeams = tokens[i + 1]->as<TokenNumber>()->number;
                    UInt minTeams = 1;
                    UInt maxTeams = 9999;
                    if (gStack.top().type == CompContainer::League) {
                        minTeams = 2;
                        maxTeams = 24;
                    }
                    if (gStack.top().type == CompContainer::Cup || gStack.top().type == CompContainer::Round) {
                        minTeams = 2;
                        maxTeams = 128;
                    }
                    if (numTeams >= minTeams && numTeams <= maxTeams) {
                        if (gStack.top().type == CompContainer::Cup) {
                            for (auto p : gStack.top().vec) {
                                if (!p->AsCup()->mRounds.empty()) {
                                    UInt actualTeamsCount = 0;
                                    for (auto const &r : p->AsCup()->mRounds)
                                        actualTeamsCount += r.mNewTeamsRound;
                                    if (numTeams != actualTeamsCount) {
                                        errorTitle = L"teams count mismatch";
                                        errorDesc = L"'teams': declared number is not same as teams count in cup rounds";
                                        break;
                                    }
                                }

                            }
                        }
                        for (auto p : gStack.top().vec)
                            p->mNumTeams = numTeams;
                        i += 1;
                        if (tokensLeft >= 2 && tokens[i + 1]->type() == TOKEN_ARRAY) {
                            TokenArray *teamsAry = tokens[i + 1]->as<TokenArray>();
                            if (teamsAry->values.size() != numTeams) {
                                errorTitle = L"'teams': array size is not the same as declared teams count";
                                errorDesc = L"";
                                break;
                            }
                            bool failed = false;
                            bool first = true;
                            String teamsLine;
                            for (UInt t = 0; t < numTeams; t++) {
                                if (first)
                                    first = false;
                                else
                                    teamsLine += L",";
                                if (teamsAry->values[t]->type() == TOKEN_NUMBER)
                                    teamsLine += Utils::Format(L"%X", teamsAry->values[t]->as<TokenNumber>()->number);
                                else if (teamsAry->values[t]->type() == TOKEN_NAME) {
                                    if (SS_IsNumber(teamsAry->values[t]->as<TokenName>()->name))
                                        teamsLine += teamsAry->values[t]->as<TokenName>()->name;
                                    else {
                                        failed = true;
                                        break;
                                    }
                                }
                                else {
                                    failed = true;
                                    break;
                                }
                            }
                            if (failed) {
                                errorTitle = L"'teams': only team IDs are allowed";
                                errorDesc = L"";
                                break;
                            }
                            for (auto p : gStack.top().vec)
                                p->SetProperty<String>(L"teams_override", teamsLine);
                            i += 1;
                        }
                        else if (tokensLeft >= 2 && tokens[i + 1]->type() == TOKEN_STRING) {
                            auto vecTeamIds = Utils::Split(tokens[i + 1]->as<TokenString>()->str, L',');
                            if (vecTeamIds.size() != numTeams) {
                                errorTitle = L"'teams': array size is not the same as declared teams count";
                                errorDesc = L"";
                                break;
                            }
                            bool failed = false;
                            bool first = true;
                            String teamsLine;
                            for (UInt t = 0; t < numTeams; t++) {
                                if (first)
                                    first = false;
                                else
                                    teamsLine += L",";
                                UInt teamIdNumber = Utils::SafeConvertInt<UInt>(vecTeamIds[t], true);
                                if (teamIdNumber != 0)
                                    teamsLine += Utils::Format(L"%X", teamIdNumber);
                                else {
                                    failed = true;
                                    break;
                                }
                            }
                            if (failed) {
                                errorTitle = L"'teams': only team IDs are allowed";
                                errorDesc = L"";
                                break;
                            }
                            for (auto p : gStack.top().vec)
                                p->SetProperty<String>(L"teams_override", teamsLine);
                            i += 1;
                        }
                    }
                    else {
                        errorTitle = L"'teams': incorrect number";
                        errorDesc = L"'teams': only values from " + Utils::Format(L"%d", minTeams) + L" to " + Utils::Format(L"%d", maxTeams) + L" are allowed";
                        break;
                    }
                }
                else {
                    errorTitle = L"'teams': expected number";
                    errorDesc = L"'teams' must be followed with teams count. For example, 'teams 20'";
                    break;
                }
            }
            else {
                errorTitle = L"'teams' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"level") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                    UInt level = tokens[i + 1]->as<TokenNumber>()->number;
                    if (level >= 1 && level <= 16) {
                        for (auto p : gStack.top().vec)
                            p->mCompetitionLevel = level - 1;
                        if (gStack.top().type == CompContainer::League) {
                            for (auto p : gStack.top().vec) {
                                p->SetProperty<Int>(L"level", p->mCompetitionLevel);
                                if (!p->HasProperty(L"league_level"))
                                    p->AsLeague()->mLeagueLevel = p->mCompetitionLevel;
                            }
                        }
                        i += 1;
                    }
                    else {
                        errorTitle = L"'level': incorrect number";
                        errorDesc = L"'level': only values from 1 to 16 are allowed";
                        break;
                    }
                }
                else {
                    errorTitle = L"'level': expected number";
                    errorDesc = L"'level' must be followed with level number. For example, 'level 2'";
                    break;
                }
            }
            else {
                errorTitle = L"'teams' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"subs") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                    if (tokens[i + 1]->as<TokenNumber>()->number >= 0 && tokens[i + 1]->as<TokenNumber>()->number <= 7) {
                        for (auto p : gStack.top().vec)
                            p->mNumSubsAllowed = tokens[i + 1]->as<TokenNumber>()->number;
                        i += 1;
                    }
                    else {
                        errorTitle = L"'subs': incorrect number";
                        errorDesc = L"'subs': only values from 0 to 7 are allowed";
                        break;
                    }
                }
                else {
                    errorTitle = L"'subs': expected number";
                    errorDesc = L"'subs' must be followed with substitutions count. For example, 'subs 4'";
                    break;
                }
            }
            else {
                errorTitle = L"'subs' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_STRING) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (!gStack.top().vec.empty() && gStack.top().vec.front()->GetProperty<Bool>(L"has_name", false)) {
                    errorTitle = L"competition already has a name";
                    errorDesc = L"'" + FifamTr(gStack.top().vec.front()->mName) + L"': competition already has a name";
                }
                String name = t->as<TokenString>()->str;
                if (Utils::StartsWith(name, L"ID_"))
                    name = L"_" + name;
                for (auto p : gStack.top().vec) {
                    if (p->GetProperty<Bool>(L"is_group")) {
                        if (name == L"A")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_A");
                        else if (name == L"B")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_B");
                        else if (name == L"C")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_C");
                        else if (name == L"D")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_D");
                        else if (name == L"E")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_E");
                        else if (name == L"F")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_F");
                        else if (name == L"G")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_G");
                        else if (name == L"H")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_H");
                        else if (name == L"I")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_I");
                        else if (name == L"J")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_J");
                        else if (name == L"K")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_K");
                        else if (name == L"L")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_L");
                        else if (name == L"1")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_1");
                        else if (name == L"2")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_2");
                        else if (name == L"3")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_3");
                        else if (name == L"4")
                            FifamTrSetAll<String>(p->mName, L"_ID_GROUP_4");
                        else
                            FifamTrSetAll<String>(p->mName, name);
                    }
                    else
                        FifamTrSetAll<String>(p->mName, name);
                    p->SetProperty<Bool>(L"has_name", true);
                }
            }
            else {
                errorTitle = L"Competition name (" + t->as<TokenString>()->str + L") must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_REFERENCE) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (!gStack.top().vec.empty() && !gStack.top().vec.front()->GetProperty<String>(L"ref", String()).empty()) {
                    errorTitle = L"competition already has a reference name";
                    errorDesc = L"'" + gStack.top().vec.front()->GetProperty<String>(L"ref", String()) + L"': competition already has a reference name";
                    break;
                }
                if (gCompRefNames.find(t->as<TokenString>()->str) != gCompRefNames.end()) {
                    errorTitle = L"reference name is already used";
                    errorDesc = L"@" + t->as<TokenString>()->str + L": reference name is already used";
                    break;
                }
                for (auto p : gStack.top().vec)
                    p->SetProperty<String>(L"ref", t->as<TokenString>()->str);
                gCompRefNames[t->as<TokenString>()->str] = gStack.top();
            }
            else {
                errorTitle = L"A reference (" + t->as<TokenString>()->str + L") must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"sorting") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Pool || gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NAME) {
                        if (gStack.top().type == CompContainer::Pool) {
                            FifamPoolSorting sorting;
                            if (GetPoolSortingByName(tokens[i + 1]->as<TokenName>()->name, sorting)) {
                                for (auto p : gStack.top().vec)
                                    p->AsPool()->mSorting = sorting;
                            }
                            else {
                                errorTitle = L"'sorting': expected pool sorting enumeration";
                                errorDesc = L"'" + tokens[i + 1]->as<TokenName>()->name + L"' is not a pool sorting enumeration";
                                break;
                            }
                        }
                        else { // League
                            FifamEqualPointsSorting sorting;
                            if (GetLeagueSortingByName(tokens[i + 1]->as<TokenName>()->name, sorting)) {
                                for (auto p : gStack.top().vec)
                                    p->AsLeague()->mEqualPointsSorting = sorting;
                            }
                            else {
                                errorTitle = L"'sorting': expected league sorting enumeration";
                                errorDesc = L"'" + tokens[i + 1]->as<TokenName>()->name + L"' is not a league sorting enumeration";
                                break;
                            }
                        }
                        i += 1;
                    }
                    else {
                        errorTitle = L"'sorting': expected sorting enumeration";
                        errorDesc = L"'sorting' must be followed with sorting enumeration";
                        break;
                    }
                }
                else {
                    errorTitle = L"'sorting' keyword works only with pool and league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'sorting' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"allow_res") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Pool) {
                    for (auto p : gStack.top().vec)
                        p->AsPool()->mReserveTeamsAllowed = true;
                }
                else {
                    errorTitle = L"'allow_res' keyword works only with pool";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'allow_res' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"constraints") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Pool) {
                    if (tokensLeft >= 1 && (tokens[i + 1]->type() == TOKEN_REFERENCE || tokens[i + 1]->type() == TOKEN_DIRECT_REFERENCE ||
                        tokens[i + 1]->type() == TOKEN_ARRAY))
                    {
                        for (auto p : gStack.top().vec)
                            p->SetProperty<TokenBase *>(L"constraints", tokens[i + 1]);
                        i += 1;
                    }
                    else {
                        errorTitle = L"'constraints': expected reference or reference array";
                        errorDesc = L"'constraints' must be followed with constraint reference or an array of constraint references";
                        break;
                    }
                }
                else {
                    errorTitle = L"'constraints' keyword works only with pool";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'constraints' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"type") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round || gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NAME) {
                        FifamRoundID roundId;
                        if (GetRoundTypeByName(tokens[i + 1]->as<TokenName>()->name, roundId)) {
                            for (auto p : gStack.top().vec) {
                                if (gStack.top().type == CompContainer::Round)
                                    p->AsRound()->mRoundType = roundId;
                                else // league
                                    p->AsLeague()->mRoundType = roundId;
                            }
                        }
                        else {
                            errorTitle = L"'type': expected round type enumeration";
                            errorDesc = L"'" + tokens[i + 1]->as<TokenName>()->name + L"' is not a round type enumeration";
                            break;
                        }
                        i += 1;
                    }
                    else {
                        errorTitle = L"'type': expected round type enumeration";
                        errorDesc = L"'type' must be followed with round type enumeration";
                        break;
                    }
                }
                else {
                    errorTitle = L"'type' keyword works only with round and league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'type' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"rel_indoor") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    for (auto p : gStack.top().vec)
                        p->AsLeague()->mIndoorRelegation = true;
                }
                else {
                    errorTitle = L"'rel_indoor' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'rel_indoor' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"rel_reserve") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    for (auto p : gStack.top().vec)
                        p->AsLeague()->mReserveRelegation = true;
                }
                else {
                    errorTitle = L"'rel_reserve' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'rel_reserve' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"take_points") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    for (auto p : gStack.top().vec)
                        p->AsLeague()->mTakePoints = true;
                }
                else {
                    errorTitle = L"'take_points' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'take_points' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"league_level") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        Int level = tokens[i + 1]->as<TokenNumber>()->number;
                        if (level >= 1 && level <= 16) {
                            for (auto p : gStack.top().vec)
                                p->AsLeague()->mLeagueLevel = level - 1;
                            i += 1;
                            if (gStack.top().type == CompContainer::League) {
                                for (auto p : gStack.top().vec) {
                                    p->SetProperty<Int>(L"league_level", p->AsLeague()->mLeagueLevel);
                                    if (!p->HasProperty(L"level"))
                                        p->mCompetitionLevel = p->AsLeague()->mLeagueLevel;
                                }
                            }
                        }
                        else {
                            errorTitle = L"'league_level': incorrect number";
                            errorDesc = L"'league_level': only values from 1 to 16 are allowed";
                            break;
                        }
                    }
                    else {
                        errorTitle = L"'league_level': expected number";
                        errorDesc = L"'league_level' must be followed with league level number. For example, 'league_level 2'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'league_level' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'league_level' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"rel") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        if (tokens[i + 1]->as<TokenNumber>()->number >= 0 && tokens[i + 1]->as<TokenNumber>()->number <= 5) {
                            for (auto p : gStack.top().vec)
                                p->AsLeague()->mNumRelegatedTeams = tokens[i + 1]->as<TokenNumber>()->number;
                            i += 1;
                        }
                        else {
                            errorTitle = L"'rel': incorrect number";
                            errorDesc = L"'rel': only values from 0 to 5 are allowed";
                            break;
                        }
                    }
                    else {
                        errorTitle = L"'rel': expected number";
                        errorDesc = L"'rel' must be followed with relegated teams count. For example, 'rel 2'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'rel' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'rel' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"rounds") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        if (tokens[i + 1]->as<TokenNumber>()->number >= 1 && tokens[i + 1]->as<TokenNumber>()->number <= 4) {
                            for (auto p : gStack.top().vec)
                                p->AsLeague()->mNumRounds = tokens[i + 1]->as<TokenNumber>()->number;
                            i += 1;
                        }
                        else {
                            errorTitle = L"'rounds': incorrect number";
                            errorDesc = L"'rounds': only values from 1 to 4 are allowed";
                            break;
                        }
                    }
                    else {
                        errorTitle = L"'rounds': expected number";
                        errorDesc = L"'rounds' must be followed with relegated teams count. For example, 'rounds 3'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'rounds' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'rounds' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"attendance" || t->as<TokenName>()->name == L"att")) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        if (tokens[i + 1]->as<TokenNumber>()->number >= 0 && tokens[i + 1]->as<TokenNumber>()->number <= 250) {
                            for (auto p : gStack.top().vec) {
                                p->AsLeague()->mAttendanceMp = tokens[i + 1]->as<TokenNumber>()->number;
                                p->SetProperty<Bool>(L"has_att", true);
                            }
                            i += 1;
                        }
                        else {
                            errorTitle = L"'" + t->as<TokenName>()->name + L"': incorrect number";
                            errorDesc = L"'" + t->as<TokenName>()->name + L"': only values from 0 to 250 are allowed";
                            break;
                        }
                    }
                    else {
                        errorTitle = L"'" + t->as<TokenName>()->name + L"': expected number";
                        errorDesc = L"'" + t->as<TokenName>()->name + L"' must be followed with attendance multiplier. For example, '" +
                            t->as<TokenName>()->name + L" 100'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"transfers" || t->as<TokenName>()->name == L"trn")) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        if (tokens[i + 1]->as<TokenNumber>()->number >= 0 && tokens[i + 1]->as<TokenNumber>()->number <= 250) {
                            for (auto p : gStack.top().vec) {
                                p->AsLeague()->mTransferMarketMp = tokens[i + 1]->as<TokenNumber>()->number;
                                p->SetProperty<Bool>(L"has_trn", true);
                            }
                            i += 1;
                        }
                        else {
                            errorTitle = L"'" + t->as<TokenName>()->name + L"': incorrect number";
                            errorDesc = L"'" + t->as<TokenName>()->name + L"': only values from 0 to 250 are allowed";
                            break;
                        }
                    }
                    else {
                        errorTitle = L"'" + t->as<TokenName>()->name + L"': expected number";
                        errorDesc = L"'" + t->as<TokenName>()->name + L"' must be followed with transfer market multiplier. For example, '" + 
                            t->as<TokenName>()->name + L" 100'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"two_legs" || t->as<TokenName>()->name == L"second_leg")) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"two_legs", true);
                }
                else {
                    errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"replay") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"replay", true);
                }
                else {
                    errorTitle = L"'replay' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'replay' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"extra_time") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"extra_time", true);
                }
                else {
                    errorTitle = L"'extra_time' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'extra_time' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"penalties" || t->as<TokenName>()->name == L"penalty")) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"penalties", true);
                }
                else {
                    errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"silver_goal") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"silver_goal", true);
                }
                else {
                    errorTitle = L"'silver_goal' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'silver_goal' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"golden_goal") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"golden_goal", true);
                }
                else {
                    errorTitle = L"'golden_goal' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'golden_goal' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        /*else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"no_shuffle") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"no_shuffle", true);
                }
                else {
                    errorTitle = L"'no_shuffle' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'no_shuffle' keyword must be associated with competition";
                errorDesc = L"";
            }
        }*/
        /*else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"no_pl_teams") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"no_pl_teams", true);
                }
                else {
                    errorTitle = L"'no_pl_teams' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'no_pl_teams' keyword must be associated with competition";
                errorDesc = L"";
            }
        }*/
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"without_away_goal") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"without_away_goal", true);
                }
                else {
                    errorTitle = L"'without_away_goal' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'without_away_goal' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"no_flags") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty(L"no_flags", true);
                }
                else {
                    errorTitle = L"'no_flags' keyword works only with round";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'no_flags' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"matchdays") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round || gStack.top().type == CompContainer::League || gStack.top().type == CompContainer::Cup) {
                    if (tokensLeft >= 1 && (tokens[i + 1]->type() == TOKEN_NUMBER || tokens[i + 1]->type() == TOKEN_ARRAY)) {
                        for (auto p : gStack.top().vec)
                            p->SetProperty<TokenBase *>(L"matchdays", tokens[i + 1]);
                        i += 1;
                    }
                    else {
                        errorTitle = L"'matchdays': expected matchdays array or matchday number";
                        errorDesc = L"";
                        break;
                    }
                }
                else {
                    errorTitle = L"'matchdays' keyword works only with round, league and cup";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'matchdays' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"matchdays2") {
        if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
            if (gStack.top().type == CompContainer::Round || gStack.top().type == CompContainer::League || gStack.top().type == CompContainer::Cup) {
                if (tokensLeft >= 1 && (tokens[i + 1]->type() == TOKEN_NUMBER || tokens[i + 1]->type() == TOKEN_ARRAY)) {
                    for (auto p : gStack.top().vec)
                        p->SetProperty<TokenBase *>(L"matchdays2", tokens[i + 1]);
                    i += 1;
                }
                else {
                    errorTitle = L"'matchdays2': expected matchdays array or matchday number";
                    errorDesc = L"";
                    break;
                }
            }
            else {
                errorTitle = L"'matchdays2' keyword works only with round, league and cup";
                errorDesc = L"";
            }
        }
        else {
            errorTitle = L"'matchdays2' keyword must be associated with competition";
            errorDesc = L"";
        }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"fixtures") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_ARRAY) {
                        for (auto p : gStack.top().vec) {
                            p->SetProperty<TokenBase *>(L"fixtures", tokens[i + 1]);
                        }
                        i += 1;
                    }
                    else {
                        errorTitle = L"'fixtures': expected fixtures array";
                        errorDesc = L"";
                        break;
                    }
                }
                else {
                    errorTitle = L"'fixtures' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'fixtures' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"eu_age_limit") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        if (tokens[i + 1]->as<TokenNumber>()->number >= 18 && tokens[i + 1]->as<TokenNumber>()->number <= 31) {
                            for (auto p : gStack.top().vec)
                                p->AsLeague()->mAgeLimitForNonEUSigns = tokens[i + 1]->as<TokenNumber>()->number;
                            i += 1;
                        }
                        else {
                            errorTitle = L"'eu_age_limit': incorrect number";
                            errorDesc = L"'eu_age_limit': only values from 18 to 31 are allowed";
                            break;
                        }
                    }
                    else {
                        errorTitle = L"'eu_age_limit': expected number";
                        errorDesc = L"'eu_age_limit' must be followed with player age. For example, 'eu_age_limit 20'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'eu_age_limit' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'eu_age_limit' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"eu_max_signs") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        if (tokens[i + 1]->as<TokenNumber>()->number >= 0 && tokens[i + 1]->as<TokenNumber>()->number <= 20) {
                            for (auto p : gStack.top().vec)
                                p->AsLeague()->mAgeLimitForNonEUSigns = tokens[i + 1]->as<TokenNumber>()->number;
                            i += 1;
                        }
                        else {
                            errorTitle = L"'eu_max_signs': incorrect number";
                            errorDesc = L"'eu_max_signs': only values from 0 to 20 are allowed";
                            break;
                        }
                    }
                    else {
                        errorTitle = L"'eu_max_signs': expected number";
                        errorDesc = L"'eu_max_signs' must be followed with signs count. For example, 'eu_max_signs 3'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'eu_max_signs' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'eu_max_signs' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"transferflags") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        UInt transferFlags = tokens[i + 1]->as<TokenNumber>()->number;
                        for (auto p : gStack.top().vec) {
                            p->AsLeague()->mFreeAgentsCanBeTranferredAtAnyTime = Utils::CheckFlag(transferFlags, 0x1);
                            p->AsLeague()->mForeignFreeAgentsCanBeTransferredAtAnyTime = Utils::CheckFlag(transferFlags, 0x2);
                            p->AsLeague()->mPlayerLoansCanBeCancelledAtAnyTime = Utils::CheckFlag(transferFlags, 0x4);
                        }
                        i += 1;
                    }
                    else {
                        errorTitle = L"'transferflags': expected number";
                        errorDesc = L"'transferflags' must be followed with flags value. For example, 'transferflags 3'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'transferflags' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'transferflags' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"loanflags") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        UInt loanFlags = tokens[i + 1]->as<TokenNumber>()->number;
                        for (auto p : gStack.top().vec) {
                            p->AsLeague()->mShortTermLoansTotalLimit = (loanFlags) & 0xF;
                            p->AsLeague()->mShortTermLoansSimultaneosLimit = (loanFlags >> 4) & 0xF;
                            p->AsLeague()->mLongTermLoansTotalLimit = (loanFlags >> 8) & 0xF;
                            p->AsLeague()->mLongTermLoansSimultaneosLimit = (loanFlags >> 12) & 0xF;
                            p->AsLeague()->mOverallTotalLoansLimit = (loanFlags >> 16) & 0xF;
                            p->AsLeague()->mOverallTotalSimultaneosLoansLimit = (loanFlags >> 20) & 0xF;
                            p->AsLeague()->mLoanedPlayersPerSeason = (loanFlags >> 24) & 0xF;
                            p->AsLeague()->mLoanPlayerPerSquad = (loanFlags >> 28) & 0xF;
                        }
                        i += 1;
                    }
                    else {
                        errorTitle = L"'loanflags': expected number";
                        errorDesc = L"'loanflags' must be followed with flags value. For example, 'loanflags 1459569736'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'loanflags' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'loanflags' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"otherflags") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::League) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        UInt otherFlags = tokens[i + 1]->as<TokenNumber>()->number;
                        for (auto p : gStack.top().vec) {
                            p->AsLeague()->mSimLoanOtherLeague = (otherFlags) & 0xF;
                            p->AsLeague()->mSimLoanSameLeague = (otherFlags >> 4) & 0xF;
                            p->AsLeague()->mLoanOtherLeagueCount = (otherFlags >> 8) & 0xF;
                            p->AsLeague()->mLoanSameLeagueCount = (otherFlags >> 12) & 0xF;
                            p->AsLeague()->mMinDomesticPlayerCount = (otherFlags >> 16) & 0x1F;
                            p->AsLeague()->mMinU21PlayerCount = (otherFlags >> 21) & 0x1F;
                            p->AsLeague()->mMinU24PlayerCount = (otherFlags >> 26) & 0x1F;
                        }
                        i += 1;
                    }
                    else {
                        errorTitle = L"'otherflags': expected number";
                        errorDesc = L"'otherflags' must be followed with flags value. For example, 'loanflags 1459569736'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'otherflags' keyword works only with league";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'otherflags' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"drawing") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Cup) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NUMBER) {
                        if (tokens[i + 1]->as<TokenNumber>()->number >= 0 && tokens[i + 1]->as<TokenNumber>()->number <= 52) {
                            for (auto p : gStack.top().vec)
                                p->AsCup()->mDrawPeriodInWeeks = tokens[i + 1]->as<TokenNumber>()->number;
                            i += 1;
                        }
                        else {
                            errorTitle = L"'drawing': incorrect number";
                            errorDesc = L"'drawing': only values from 0 to 52 are allowed";
                            break;
                        }
                    }
                    else {
                        errorTitle = L"'drawing': expected number";
                        errorDesc = L"'drawing' must be followed with a week number. For example, 'drawing 2'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'drawing' keyword works only with cup";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'drawing' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"template") {
        if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
            if (gStack.top().type == CompContainer::Cup) {
                if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_NAME) {
                    FifamCupSystemType systemType;
                    if (GetCupSystemTypeByName(tokens[i + 1]->as<TokenName>()->name, systemType)) {
                        for (auto p : gStack.top().vec)
                            p->AsCup()->mCupTemplate = systemType;
                        i += 1;
                    }
                    else {
                        errorTitle = L"'template': expected cup system type";
                        errorDesc = L"'template' must be followed with a cup system type. For example, 'template FA_England'";
                        break;
                    }
                }
                else {
                    errorTitle = L"'template': expected country id";
                    errorDesc = L"'template' must be followed with a cup system type. For example, 'template FA_England'";
                    break;
                }
            }
            else {
                errorTitle = L"'template' keyword works only with cup";
                errorDesc = L"";
            }
        }
        else {
            errorTitle = L"'template' keyword must be associated with competition";
            errorDesc = L"";
        }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"bonus" || t->as<TokenName>()->name == L"bonuses")) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Round || gStack.top().type == CompContainer::League || gStack.top().type == CompContainer::Cup) {
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_ARRAY) {
                        for (auto p : gStack.top().vec)
                            p->SetProperty<TokenBase *>(L"bonus", tokens[i + 1]);
                        i += 1;
                    }
                    else {
                        errorTitle = L"'" + t->as<TokenName>()->name + L"': expected bonuses array";
                        errorDesc = L"";
                        break;
                    }
                }
                else {
                    errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword works only with round, league and cup";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && t->as<TokenName>()->name == L"format") {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().type == CompContainer::Cup) {
                    FifamCompCup *cup = gStack.top().vec.front()->AsCup();
                    if (tokensLeft >= 1 && (tokens[i + 1]->type() == TOKEN_NUMBER || tokens[i + 1]->type() == TOKEN_NAME || tokens[i + 1]->type() == TOKEN_ARRAY)) {
                        Array<String, 8> roundDesc;
                        if (tokens[i + 1]->type() == TOKEN_NUMBER)
                            roundDesc[7] = Utils::Format(L"%d", tokens[i + 1]->as<TokenNumber>()->number);
                        else if (tokens[i + 1]->type() == TOKEN_NAME)
                            roundDesc[7] = tokens[i + 1]->as<TokenName>()->name;
                        else { // array
                            TokenArray *roundsAry = tokens[i + 1]->as<TokenArray>();
                            if (roundsAry->values.empty()) {
                                errorTitle = L"'format': expected non-empty array";
                                errorDesc = L"";
                                break;
                            }
                            else if (roundsAry->values.size() > 8) {
                                errorTitle = L"'format': too many rounds (max 8 is possible)";
                                errorDesc = L"";
                                break;
                            }
                            bool failed = false;
                            UInt descId = 8 - roundsAry->values.size();
                            for (UInt r = 0; r < roundsAry->values.size(); r++) {
                                if (roundsAry->values[r]->type() == TOKEN_NUMBER)
                                    roundDesc[descId] = Utils::Format(L"%d", roundsAry->values[r]->as<TokenNumber>()->number);
                                else if (roundsAry->values[r]->type() == TOKEN_NAME)
                                    roundDesc[descId] = roundsAry->values[r]->as<TokenName>()->name;
                                else {
                                    failed = true;
                                    break;
                                }
                                descId++;
                            }
                            if (failed) {
                                errorTitle = L"'format': expected round description";
                                errorDesc = L"";
                                break;
                            }
                        }
                        bool failed = false;
                        Int numRounds = 0;
                        Int startRoundIndex = -1;
                        for (Int i = 7; i >= 0; i--) {
                            if (roundDesc[i].empty())
                                break;
                            startRoundIndex = i;
                            numRounds++;
                        }
                        if (numRounds > 0 && startRoundIndex >= 0) {
                            cup->mRounds.resize(numRounds);
                            bool failed = false;
                            Int r = 0;
                            for (Int i = startRoundIndex; i <= 7; i++) {
                                if (roundDesc[i].empty()) {
                                    errorTitle = L"'format': expected round description (found empty value)";
                                    errorDesc = L"";
                                    failed = true;
                                    break;
                                }
                                cup->mRounds[r].mFlags.Clear();
                                for (UInt c = 0; c < roundDesc[i].length(); c++) {
                                    if (c == 0) {
                                        if (roundDesc[i][c] == '1')
                                            cup->mRounds[r].mFlags.Set(FifamBeg::_1stLeg, true);
                                        else if (roundDesc[i][c] == '2')
                                            cup->mRounds[r].mFlags.Set(FifamBeg::_2ndLeg, true);
                                        else if (roundDesc[i][c] == 'R')
                                            cup->mRounds[r].mFlags.Set(FifamBeg::WithReplay, true);
                                        else {
                                            errorTitle = L"'format': wrong round description";
                                            errorDesc = L"";
                                            failed = true;
                                            break;
                                        }
                                    }
                                    else {
                                        if (roundDesc[i][c] == 'E')
                                            cup->mRounds[r].mFlags.Set(FifamBeg::WithExtraTime, true);
                                        else if (roundDesc[i][c] == 'P')
                                            cup->mRounds[r].mFlags.Set(FifamBeg::WithPenalty, true);
                                        else if (roundDesc[i][c] == 'G')
                                            cup->mRounds[r].mFlags.Set(FifamBeg::WithGoldenGoal, true);
                                        else if (roundDesc[i][c] == 'S')
                                            cup->mRounds[r].mFlags.Set(FifamBeg::WithSilverGoal, true);
                                        else if (roundDesc[i][c] == '-' && (c + 1) < roundDesc[i].length()) {
                                            if (roundDesc[i][c + 1] == 'P')
                                                cup->mRounds[r].mFlags.Set(FifamBeg::NoPremierLeagueTeams, true);
                                            else if (roundDesc[i][c + 1] == 'S')
                                                cup->mRounds[r].mFlags.Set(FifamBeg::NoShuffle, true);
                                            else if (roundDesc[i][c + 1] == 'A')
                                                cup->mRounds[r].mFlags.Set(FifamBeg::WithoutAwayGoal, true);
                                            c++;
                                        }
                                        else if (roundDesc[i][c] == '+') {
                                            Int newTeamsInRound = SS_ToNumber(roundDesc[i].substr(c + 1));
                                            if (newTeamsInRound > 0)
                                                cup->mRounds[r].mNewTeamsRound = newTeamsInRound;
                                            break;
                                        }
                                    }
                                }
                                r++;
                            }
                            if (failed)
                                break;
                            if (!cup->mRounds.empty()) {
                                UInt teamsFromPrevRound = 0;
                                UInt currBeg = 0;
                                UInt lastRoundWithAddedTeams = 0;
                                UInt newTeamsCount = 0;
                                for (UInt r = 0; r < cup->mRounds.size(); r++) {
                                    newTeamsCount += cup->mRounds[r].mNewTeamsRound;
                                    cup->mRounds[r].mTeamsRound = cup->mRounds[r].mNewTeamsRound + teamsFromPrevRound;
                                    teamsFromPrevRound = cup->mRounds[r].mTeamsRound / 2;
                                    cup->mRounds[r].mStartBeg = currBeg;
                                    currBeg += teamsFromPrevRound;
                                    cup->mRounds[r].mEndBeg = currBeg;
                                    if (cup->mRounds[r].mNewTeamsRound)
                                        lastRoundWithAddedTeams = r;
                                }
                                if (cup->mNumTeams > 0 && cup->mNumTeams != newTeamsCount) {
                                    errorTitle = L"teams count mismatch";
                                    errorDesc = L"'teams': declared number is not same as teams count in cup rounds";
                                    break;
                                }
                                cup->mNumTeams = newTeamsCount;
                                UInt numRawRounds = cup->mRounds.size() - lastRoundWithAddedTeams;
                                // final rounds
                                Int numFinalRounds = 0;
                                for (Int r = (Int)cup->mRounds.size() - 1; r >= (Int)lastRoundWithAddedTeams; r--) {
                                    if (numFinalRounds >= 4)
                                        break;
                                    cup->mRounds[r].mRoundID.SetFromInt(15 - numFinalRounds);
                                    numFinalRounds++;
                                }
                                Int numRoundsLeft = cup->mRounds.size() - numFinalRounds;
                                // quali rounds
                                Int numQualiRounds = numRoundsLeft > 5 ? 8 - numRoundsLeft : 0;
                                for (Int r = 0; r < numQualiRounds; r++)
                                    cup->mRounds[r].mRoundID.SetFromInt(r + 1);
                                // other rounds
                                numRoundsLeft -= numQualiRounds;
                                for (Int r = 0; r < numRoundsLeft; r++)
                                    cup->mRounds[r + numQualiRounds].mRoundID.SetFromInt(r + 5);
                            }
                        }
                        else {
                            errorTitle = L"'format': parsing error";
                            errorDesc = L"";
                            break;
                        }
                        i += 1;
                    }
                    else {
                        errorTitle = L"'format': expected round format or an array of round format";
                        errorDesc = L"";
                        break;
                    }
                }
                else {
                    errorTitle = L"'format' keyword works only with cup";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'format' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_NAME && (t->as<TokenName>()->name == L"use_end_of_entry" || t->as<TokenName>()->name == L"use_eoe")) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                for (auto p : gStack.top().vec)
                    p->SetProperty<Bool>(L"use_eoe", true);
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"' keyword must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_PREDECESSOR) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (tokensLeft >= 1 && (tokens[i + 1]->type() == TOKEN_REFERENCE || tokens[i + 1]->type() == TOKEN_DIRECT_REFERENCE
                    || tokens[i + 1]->type() == TOKEN_ARRAY))
                {
                    for (auto p : gStack.top().vec)
                        p->SetProperty<TokenBase *>(t->as<TokenPredecessor>()->isWeak ? L"pred_weak" : L"pred", tokens[i + 1]);
                    i += 1;
                }
                else {
                    errorTitle = L"predecessor: expected reference or reference array";
                    errorDesc = L"predecessor must be followed with predecessor reference or an array of predecessor references";
                    break;
                }
            }
            else {
                errorTitle = L"predecessor must be associated with competition";
                errorDesc = L"";
            }
        }
        else if (t->type() == TOKEN_SUCCESSOR) {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (tokensLeft >= 1 && (tokens[i + 1]->type() == TOKEN_REFERENCE || tokens[i + 1]->type() == TOKEN_DIRECT_REFERENCE
                    || tokens[i + 1]->type() == TOKEN_ARRAY))
                {
                    for (auto p : gStack.top().vec)
                        p->SetProperty<TokenBase *>(t->as<TokenSuccessor>()->isWeak ? L"succ_weak" : L"succ", tokens[i + 1]);
                    i += 1;
                }
                else {
                    errorTitle = L"successor: expected reference or reference array";
                    errorDesc = L"successor must be followed with successor reference or an array of successor references";
                    break;
                }
            }
            else {
                errorTitle = L"successor must be associated with competition";
                errorDesc = L"";
            }
        }
        else {
        FifamInstructionID insID = FifamInstructionID::ID_END_OF_ENTRY;
        String specialID;
        if (t->type() == TOKEN_NAME) {
            String &name = t->as<TokenName>()->name;
            if (name == L"get")
                specialID = name;
            else if (name == L"reserveAssessmentTeams") insID = FifamInstructionID::ID_RESERVE_ASSESSMENT_TEAMS;
            else if (name == L"fillAssessmentReserves") insID = FifamInstructionID::ID_FILL_ASSESSMENT_RESERVES;
            else if (name == L"changeTeamTypes") insID = FifamInstructionID::ID_CHANGE_TEAM_TYPES;
            else if (name == L"copyLeagueData") insID = FifamInstructionID::ID_COPY_LEAGUE_DATA;
            else if (name == L"shuffleTeams") insID = FifamInstructionID::ID_SHUFFLE_TEAMS;
            else if (name == L"buildCounter") insID = FifamInstructionID::ID_BUILD_COUNTER;
            else if (name == L"buildUEFA5") insID = FifamInstructionID::ID_BUILD_UEFA5;
            else if (Utils::StartsWith(name, L"get")) {
                if (name == L"getChamp")                       insID = FifamInstructionID::ID_GET_CHAMP;
                else if (name == L"getEuropeanAssessmentTeams")     insID = FifamInstructionID::ID_GET_EUROPEAN_ASSESSMENT_TEAMS;
                else if (name == L"getUEFA5Tab")                    insID = FifamInstructionID::ID_GET_UEFA5_TAB;
                else if (name == L"getUEFA5SureTab")                insID = FifamInstructionID::ID_GET_UEFA5_SURE_TAB;
                else if (name == L"getChampOrRunnerUp")             insID = FifamInstructionID::ID_GET_CHAMP_OR_RUNNER_UP;
                else if (name == L"getUEFA5SureUIC")                insID = FifamInstructionID::ID_GET_UEFA5_SURE_UIC;
                else if (name == L"getTabXToY")                     insID = FifamInstructionID::ID_GET_TAB_X_TO_Y;
                else if (name == L"getTabSureXToYZ")                insID = FifamInstructionID::ID_GET_TAB_SURE_X_TO_Y_Z;
                else if (name == L"getTabLevelXToY")                insID = FifamInstructionID::ID_GET_TAB_LEVEL_X_TO_Y;
                else if (name == L"getTabSpare")                    insID = FifamInstructionID::ID_GET_TAB_SPARE;
                else if (name == L"getTabLevelStartXToY")           insID = FifamInstructionID::ID_GET_TAB_LEVEL_START_X_TO_Y;
                else if (name == L"getEuropeanAssessmentCupwinner") insID = FifamInstructionID::ID_GET_EUROPEAN_ASSESSMENT_CUPWINNER;
                else if (name == L"getUEFA5ChampOrFinalist")        insID = FifamInstructionID::ID_GET_UEFA5_CHAMP_OR_FINALIST;
                else if (name == L"getWinner")                      insID = FifamInstructionID::ID_GET_WINNER;
                else if (name == L"getLoser")                       insID = FifamInstructionID::ID_GET_LOSER;
                else if (name == L"getPool")                        insID = FifamInstructionID::ID_GET_POOL;
                else if (name == L"getNatUEFA5WithHost")            insID = FifamInstructionID::ID_GET_NAT_UEFA5_WITH_HOST;
                else if (name == L"getNatUEFA5WithoutHost")         insID = FifamInstructionID::ID_GET_NAT_UEFA5_WITHOUT_HOST;
                else if (name == L"getNatSouthAmerica")             insID = FifamInstructionID::ID_GET_NAT_SOUTH_AMERICA;
                else if (name == L"getNatAmerica")                  insID = FifamInstructionID::ID_GET_NAT_AMERICA;
                else if (name == L"getNatAfrica")                   insID = FifamInstructionID::ID_GET_NAT_AFRICA;
                else if (name == L"getNatAsia")                     insID = FifamInstructionID::ID_GET_NAT_ASIA;
                else if (name == L"getNatOceania")                  insID = FifamInstructionID::ID_GET_NAT_OCEANIA;
                else if (name == L"getHost")                        insID = FifamInstructionID::ID_GET_HOST;
                else if (name == L"getInternationalTabLevelXToY")   insID = FifamInstructionID::ID_GET_INTERNATIONAL_TAB_LEVEL_X_TO_Y;
                else if (name == L"getInternationalSpare")          insID = FifamInstructionID::ID_GET_INTERNATIONAL_SPARE;
                else if (name == L"getRunnerUp")                    insID = FifamInstructionID::ID_GET_RUNNER_UP;
                else if (name == L"getTabLevelIndoor")              insID = FifamInstructionID::ID_GET_TAB_LEVEL_INDOOR;
                else if (name == L"getRelegatedTeams")              insID = FifamInstructionID::ID_GET_RELEGATED_TEAMS;
                else if (name == L"getInternationalTeams")          insID = FifamInstructionID::ID_GET_INTERNATIONAL_TEAMS;
                else if (name == L"getCCFaWinner")                  insID = FifamInstructionID::ID_GET_CC_FA_WINNER;
                else if (name == L"getCCSpare")                     insID = FifamInstructionID::ID_GET_CC_SPARE;
                else if (name == L"getChampCountryTeam")            insID = FifamInstructionID::ID_GET_CHAMP_COUNTRY_TEAM;
                else if (name == L"getRandomNationalTeam")          insID = FifamInstructionID::ID_GET_RANDOM_NATIONAL_TEAM;
                else if (name == L"getFairnessTeam")                insID = FifamInstructionID::ID_GET_FAIRNESS_TEAM;
                else if (name == L"getNationalTeam")                insID = FifamInstructionID::ID_GET_NATIONAL_TEAM;
                else if (name == L"getNationalTeamWithoutHost")     insID = FifamInstructionID::ID_GET_NATIONAL_TEAM_WITHOUT_HOST;
                else if (name == L"getBottom")                      specialID = name;
                else if (name == L"getAll")                         specialID = name;
                else if (name == L"getBottomAll")                   specialID = name;
                else if (name == L"getFromTo")                      specialID = name;
                else if (name == L"getBottomFromTo")                specialID = name;
            }
        }
        if (insID == FifamInstructionID::ID_END_OF_ENTRY && specialID.empty()) {
            errorTitle = L"'" + t->print() + L"': unexpected value";
            errorDesc = L"";
        }
        else {
            if (!gStack.empty() && gStack.top().type != CompContainer::Root) {
                if (gStack.top().blockStarted) {
                    Bool failed = false;
                    Instruction ins;
                    ins.id = insID;
                    ins.specialId = specialID;
                    UInt counter = 1;
                    if (tokensLeft >= 1 && tokens[i + 1]->type() == TOKEN_PARAMS_OPEN) {
                        Bool finishedInstruction = false;
                        Bool unexpectedToken = false;
                        while (tokensLeft > counter) {
                            TokenBase *nextToken = tokens[i + 1 + counter];
                            if (nextToken->type() == TOKEN_PARAMS_CLOSE) {
                                finishedInstruction = true;
                                break;
                            }
                            if (nextToken->type() != TOKEN_NAME &&
                                nextToken->type() != TOKEN_NUMBER &&
                                nextToken->type() != TOKEN_STRING &&
                                nextToken->type() != TOKEN_ARRAY &&
                                nextToken->type() != TOKEN_REFERENCE &&
                                nextToken->type() != TOKEN_DIRECT_REFERENCE)
                            {
                                errorTitle = L"'" + nextToken->print() + L"' is unexpected in instruction arguments list";
                                errorDesc = L"";
                                unexpectedToken = true;
                                break;
                            }
                            ins.arguments.push_back(nextToken);
                            counter++;
                        }
                        if (!finishedInstruction) {
                            if (!unexpectedToken) {
                                errorTitle = L"'" + t->as<TokenName>()->name + L"': expected ')'";
                                errorDesc = L"";
                            }
                            failed = true;
                        }
                    }
                    else {
                        errorTitle = L"'" + t->as<TokenName>()->name + L"': expected '('";
                        errorDesc = L"";
                        failed = true;
                    }
                    if (failed)
                        break;
                    else {
                        for (auto p : gStack.top().vec) {
                            Vector<Instruction> *instructions = p->GetProperty<Vector<Instruction> *>(L"instructions", nullptr);
                            if (!instructions) {
                                instructions = new Vector<Instruction>;
                                p->SetProperty<Vector<Instruction> *>(L"instructions", instructions);
                            }
                            instructions->push_back(ins);
                        }
                        i += 1 + counter;
                    }
                }
                else {
                    errorTitle = L"'" + t->as<TokenName>()->name + L"' instruction must be placed inside block";
                    errorDesc = L"";
                }
            }
            else {
                errorTitle = L"'" + t->as<TokenName>()->name + L"' instruction must be associated with competition";
                errorDesc = L"";
            }
        }
        }
    }

    if (errorTitle.empty()) {
        // resolve competition IDs
        for (auto comp : gComps) {
            if (comp->GetDbType() != FifamCompDbType::Root) {
                Int index = comp->GetProperty<Int>(L"index", -1);
                if (index != -1) {
                    if (comp->GetProperty<Bool>(L"relegation", false))
                        comp->mID.mType = FifamCompType::Relegation;
                    comp->mID.mIndex = index;
                    if (!db.GetCompetition(comp->mID)) {
                        db.mCompMap[comp->mID] = comp;
                    }
                    else {
                        errorTitle = L"competition index " + Utils::Format(L"%d", index) + L" is already in use";
                        break;
                    }
                }
            }
        }
        if (errorTitle.empty()) {
        for (auto comp : gComps) {
            if (comp->GetDbType() != FifamCompDbType::Root) {
                Int index = comp->GetProperty<Int>(L"index", -1);
                if (index == -1) {
                    if (comp->GetProperty<Bool>(L"relegation", false))
                        comp->mID.mType = FifamCompType::Relegation;
                    comp->mID = GetAvailableCompID(&db, comp->mID.mRegion, comp->mID.mType);
                    db.mCompMap[comp->mID] = comp;
                }
            }
        }
        if (errorTitle.empty()) {
            // resolve explicit predecessors/successors
            for (auto[compId, comp] : db.mCompMap) {
                if (comp->GetDbType() != FifamCompDbType::Root) {
                    auto predComps = comp->GetProperty<Vector<FifamCompetition *> *>(L"pred_comps", nullptr);
                    if (predComps) {
                        for (auto predComp : *predComps)
                            comp->mPredecessors.push_back(predComp->mID);
                    }
                    TokenBase *pred = comp->GetProperty<TokenBase *>(L"pred", nullptr);
                    if (pred) {
                        if (!ResolveReferenceList(&db, comp, gCompRefNames, pred, comp->mPredecessors, 32, errorDesc, FifamCompDbType::Root, false)) {
                            errorTitle = L"reference resolving error";
                            break;
                        }
                    }
                    TokenBase *succ = comp->GetProperty<TokenBase *>(L"succ", nullptr);
                    if (succ) {
                        if (!ResolveReferenceList(&db, comp, gCompRefNames, succ, comp->mSuccessors, 32, errorDesc, FifamCompDbType::Root, false)) {
                            errorTitle = L"reference resolving error";
                            break;
                        }
                    }
                    for (auto const &id : comp->mPredecessors) {
                        auto c = db.GetCompetition(id);
                        if (c && !Utils::Contains(c->mSuccessors, compId))
                            c->mSuccessors.push_back(compId);
                    }
                    for (auto const &id : comp->mSuccessors) {
                        auto c = db.GetCompetition(id);
                        if (c && !Utils::Contains(c->mPredecessors, compId))
                            c->mPredecessors.push_back(compId);
                    }
                    TokenBase *predWeak = comp->GetProperty<TokenBase *>(L"pred_weak", nullptr);
                    if (predWeak) {
                        Vector<FifamCompID> weakCompIDs;
                        if (!ResolveReferenceList(&db, comp, gCompRefNames, predWeak, weakCompIDs, 32, errorDesc, FifamCompDbType::Root, false)) {
                            errorTitle = L"reference resolving error";
                            break;
                        }
                        comp->mPredecessors.insert(comp->mPredecessors.end(), weakCompIDs.begin(), weakCompIDs.end());
                    }
                    TokenBase *succWeak = comp->GetProperty<TokenBase *>(L"succ_weak", nullptr);
                    if (succWeak) {
                        Vector<FifamCompID> weakCompIDs;
                        if (!ResolveReferenceList(&db, comp, gCompRefNames, succWeak, weakCompIDs, 32, errorDesc, FifamCompDbType::Root, false)) {
                            errorTitle = L"reference resolving error";
                            break;
                        }
                        comp->mSuccessors.insert(comp->mSuccessors.end(), weakCompIDs.begin(), weakCompIDs.end());
                    }
                }
            }
            if (errorTitle.empty()) {
            // sort predecessors/successors
            for (auto[compId, comp] : db.mCompMap) {
                if (comp->GetDbType() != FifamCompDbType::Root) {
                    // now sort
                    std::sort(comp->mPredecessors.begin(), comp->mPredecessors.end());
                    comp->mPredecessors.erase(std::unique(comp->mPredecessors.begin(), comp->mPredecessors.end()), comp->mPredecessors.end());
                    std::sort(comp->mSuccessors.begin(), comp->mSuccessors.end());
                    comp->mSuccessors.erase(std::unique(comp->mSuccessors.begin(), comp->mSuccessors.end()), comp->mSuccessors.end());
                }
            }
            if (errorTitle.empty()) {
            // resolve everything else
            for (auto[compId, comp] : db.mCompMap) {
                if (comp->GetDbType() != FifamCompDbType::Root) {
                    CompetitionParams const *params = nullptr;
                    auto it = options.compOptions.find(comp->mID.ToInt());
                    if (it != options.compOptions.end())
                        params = &(*it).second;

                    if (params && !params->name.empty() && !comp->GetProperty<Bool>(L"has_name", false)) {
                        FifamTrSetAll(comp->mName, params->name);
                        comp->SetProperty<Bool>(L"has_name", true);
                    }

                    if (comp->GetProperty<Bool>(L"use_eoe", false))
                        comp->mUseEndOfEntry = true;
                    if (comp->GetDbType() == FifamCompDbType::Round) {
                        Bool noFlags = comp->GetProperty<Bool>(L"no_flags", false);
                        FifamFlags<FifamBeg> begFlags;
                        if (comp->GetProperty<Bool>(L"replay", false))
                            begFlags.Set(FifamBeg::WithReplay, true);
                        if (comp->GetProperty<Bool>(L"extra_time", false) || !noFlags)
                            begFlags.Set(FifamBeg::WithExtraTime, true);
                        if (comp->GetProperty<Bool>(L"penalties", false) || !noFlags)
                            begFlags.Set(FifamBeg::WithPenalty, true);
                        if (comp->GetProperty<Bool>(L"silver_goal", false))
                            begFlags.Set(FifamBeg::WithSilverGoal, true);
                        if (comp->GetProperty<Bool>(L"golden_goal", false))
                            begFlags.Set(FifamBeg::WithGoldenGoal, true);
                        if (comp->GetProperty<Bool>(L"without_away_goal", false))
                            begFlags.Set(FifamBeg::WithoutAwayGoal, true);
                        if (!IsCountryRegion(comp->mID.mRegion, MaxCountryId))
                            begFlags.Set(FifamBeg::End, true);
                        if (comp->GetProperty<Bool>(L"two_legs", false)) {
                            comp->AsRound()->m1stLegFlags.Set(FifamBeg::_2ndLeg, true);
                            comp->AsRound()->m2ndLegFlags = begFlags;
                            comp->AsRound()->m2ndLegFlags.Set(FifamBeg::_2ndLeg, true);
                        }
                        else {
                            comp->AsRound()->m1stLegFlags = begFlags;
                            comp->AsRound()->m1stLegFlags.Set(FifamBeg::_1stLeg, true);
                        }
                        Bool hasName = comp->GetProperty<Bool>(L"has_name", false);
                        if (!hasName) {
                            if (comp->AsRound()->mRoundType == FifamRoundID::Final) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_FINAL");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::Semifinal) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_SEMIFINAL");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::Quarterfinal) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_QUARTERFINAL");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::Last16) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_LAST_16");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::Final3rdPlace) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_3RD_PLACE");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::_1) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_ROUND1");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::_2) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_ROUND2");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::_3) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_ROUND3");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::_4) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_ROUND4");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::_5) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_ROUND5");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::Quali) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_PREQUALI1");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::Quali2) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_PREQUALI2");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::Quali3) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_PREQUALI3");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                            else if (comp->AsRound()->mRoundType == FifamRoundID::PreRound1) {
                                FifamTrSetAll<String>(comp->mName, L"_ID_QUALI");
                                comp->SetProperty<Bool>(L"has_name", true);
                            }
                        }
                    }
                    if (comp->GetDbType() == FifamCompDbType::League) {
                        if (params) {
                            if (!params->teams.empty() && !comp->HasProperty(L"teams_override")) {
                                if (params->teams.size() != comp->mNumTeams) {
                                    errorTitle = L"script auto-completion error";
                                    errorDesc = Utils::Format(L"teams count is different (script - %d, database - %d)", comp->mNumTeams, params->teams.size());
                                    break;
                                }
                                String teamsStr;
                                for (UInt i = 0; i < params->teams.size(); i++) {
                                    if (i != 0)
                                        teamsStr += L",";
                                    teamsStr += Utils::Format(L"%x", params->teams[i]);
                                }
                                comp->SetProperty<String>(L"teams_override", teamsStr);
                            }

                            if (params->attendanceMultiplier != -1 && !comp->GetProperty<Bool>(L"has_att", false))
                                comp->AsLeague()->mAttendanceMp = params->attendanceMultiplier;
                            if (params->transfersMultiplier != -1 && !comp->GetProperty<Bool>(L"has_trn", false))
                                comp->AsLeague()->mTransferMarketMp = params->transfersMultiplier;
                        }

                        TokenArray *ary = (TokenArray *)(comp->GetProperty<TokenBase *>(L"fixtures", nullptr));
                        if (ary) {
                            UInt requiredElementsCount = comp->AsLeague()->GetNumMatchdays() * comp->AsLeague()->GetNumMatchesInMatchday() * 2;
                            if (ary->values.size() != requiredElementsCount) {
                                errorTitle = L"'fixtures': fixtures count is wrong";
                                errorDesc = L"";
                                break;
                            }
                            UInt i = 0;
                            for (UInt m = 0; m < comp->AsLeague()->GetNumMatchdays(); m++) {
                                auto &fixtures = comp->AsLeague()->mFixtures.emplace_back();
                                for (UInt n = 0; n < comp->AsLeague()->GetNumMatchesInMatchday(); n++) {
                                    if (ary->values[i]->type() == TOKEN_NUMBER && ary->values[i + 1]->type() == TOKEN_NUMBER)
                                        fixtures.emplace_back(ary->values[i]->as<TokenNumber>()->number, ary->values[i + 1]->as<TokenNumber>()->number);
                                    else {
                                        errorTitle = L"'fixtures': only numbers are allowed";
                                        errorDesc = L"";
                                        break;
                                    }
                                    i += 2;
                                }
                            }
                        }
                        else
                            comp->AsLeague()->GenerateFixtures();
                    }
                    else if (comp->GetDbType() == FifamCompDbType::Pool) {
                        TokenBase *constraints = comp->GetProperty<TokenBase *>(L"constraints", nullptr);
                        if (constraints) {
                            if (!ResolveReferenceList(&db, comp, gCompRefNames, constraints, comp->AsPool()->mCompConstraints, 3, errorDesc, FifamCompDbType::Root, false)) {
                                errorTitle = L"reference resolving error";
                                break;
                            }
                        }
                    }
                    Bool failed = false;
                    if (comp->GetDbType() == FifamCompDbType::League) {
                        TokenArray *ary = (TokenArray *)comp->GetProperty<TokenBase *>(L"bonus", nullptr);
                        if (ary) {
                            if (!TokenToBonuses(ary, comp->AsLeague()->mBonuses, errorDesc, false)) {
                                errorTitle = L"bonuses array error";
                                break;
                            }
                        }
                        UInt numMatchdays = comp->AsLeague()->GetNumMatchdays();
                        TokenBase *matchdays = comp->GetProperty<TokenBase *>(L"matchdays", nullptr);
                        if (matchdays) {
                            Vector<UShort> days;
                            Vector<UInt> arraySizes;
                            Bool hasArrays = false;
                            if (!TokenToMatchdays(matchdays, days, arraySizes, errorDesc, hasArrays)) {
                                errorTitle = L"matchdays error";
                                break;
                            }
                            if (days.size() != numMatchdays) {
                                errorTitle = L"matchdays error";
                                errorDesc = L"match dates count is not same as matchdays count (" + Utils::Format(L"%u", numMatchdays) + L")";
                                break;
                            }
                            for (UInt m = 0; m < days.size(); m++)
                                comp->AsLeague()->mFirstSeasonMatchdays.push_back(days[m]);
                        }
                        TokenBase *matchdays2 = comp->GetProperty<TokenBase *>(L"matchdays2", nullptr);
                        if (matchdays2) {
                            Vector<UShort> days;
                            Vector<UInt> arraySizes;
                            Bool hasArrays = false;
                            if (!TokenToMatchdays(matchdays2, days, arraySizes, errorDesc, hasArrays)) {
                                errorTitle = L"matchdays2 error";
                                break;
                            }
                            if (days.size() != numMatchdays) {
                                errorTitle = L"matchdays2 error";
                                errorDesc = L"match dates count is not same as matchdays count (" + Utils::Format(L"%u", numMatchdays) + L")";
                                break;
                            }
                            for (UInt m = 0; m < days.size(); m++)
                                comp->AsLeague()->mSecondSeasonMatchdays.push_back(days[m]);
                        }
                        if (comp->AsLeague()->mSecondSeasonMatchdays.empty() && IsCountryRegion(comp->mID.mRegion, MaxCountryId))
                            comp->AsLeague()->mSecondSeasonMatchdays = comp->AsLeague()->mFirstSeasonMatchdays;
                    }
                    else if (comp->GetDbType() == FifamCompDbType::Round) {
                        TokenArray *ary = (TokenArray *)comp->GetProperty<TokenBase *>(L"bonus", nullptr);
                        if (ary) {
                            if (!TokenToBonuses(ary, comp->AsRound()->mBonuses, errorDesc, true)) {
                                errorTitle = L"bonuses array error";
                                break;
                            }
                        }
                        UInt numMatches = comp->AsRound()->m1stLegFlags.Check(FifamBeg::_2ndLeg) ? 2 : 1;
                        TokenBase *matchdays = comp->GetProperty<TokenBase *>(L"matchdays", nullptr);
                        if (matchdays) {
                            Vector<UShort> days;
                            Vector<UInt> arraySizes;
                            Bool hasArrays = false;
                            if (!TokenToMatchdays(matchdays, days, arraySizes, errorDesc, hasArrays)) {
                                errorTitle = L"matchdays error";
                                break;
                            }
                            if (days.size() != numMatches) {
                                errorTitle = L"matchdays error";
                                errorDesc = L"match dates count is not same as matchdays count (" + Utils::Format(L"%u", numMatches) + L")";
                                break;
                            }
                            for (UInt m = 0; m < days.size(); m++)
                                comp->AsRound()->mFirstSeasonMatchdays[m] = days[m];
                        }
                        TokenBase *matchdays2 = comp->GetProperty<TokenBase *>(L"matchdays2", nullptr);
                        if (matchdays2) {
                            Vector<UShort> days;
                            Vector<UInt> arraySizes;
                            Bool hasArrays = false;
                            if (!TokenToMatchdays(matchdays2, days, arraySizes, errorDesc, hasArrays)) {
                                errorTitle = L"matchdays2 error";
                                break;
                            }
                            if (days.size() != numMatches) {
                                errorTitle = L"matchdays2 error";
                                errorDesc = L"match dates count is not same as matchdays count (" + Utils::Format(L"%u", numMatches) + L")";
                                break;
                            }
                            for (UInt m = 0; m < days.size(); m++)
                                comp->AsRound()->mSecondSeasonMatchdays[m] = days[m];
                        }
                        if (comp->AsRound()->mSecondSeasonMatchdays[0] == 0 && comp->AsRound()->mSecondSeasonMatchdays[1] == 0)
                            comp->AsRound()->mSecondSeasonMatchdays = comp->AsRound()->mFirstSeasonMatchdays;
                    }
                    else if (comp->GetDbType() == FifamCompDbType::Cup) {
                        TokenArray *ary = (TokenArray *)comp->GetProperty<TokenBase *>(L"bonus", nullptr);
                        if (ary) {
                            if (comp->AsCup()->mRounds.empty()) {
                                errorTitle = L"no rounds in cup";
                                errorDesc = L"";
                                break;
                            }
                            if (!ary->values.empty()) {
                                if (ary->values.front()->type() == TOKEN_ARRAY) {
                                    if (comp->AsCup()->mRounds.size() != ary->values.size()) {
                                        errorTitle = L"bonuses array error";
                                        errorDesc = L"bonuses array size is not same as rounds count";
                                        break;
                                    }
                                    Bool failed = false;
                                    for (UInt r = 0; r < ary->values.size(); r++) {
                                        if (ary->values[r]->type() != TOKEN_ARRAY) {
                                            errorDesc = L"expected array";
                                            failed = true;
                                            break;
                                        }
                                        if (!TokenToBonuses(ary->values[r]->as<TokenArray>(), comp->AsCup()->mRounds[r].mBonuses, errorDesc, true)) {
                                            failed = true;
                                            break;
                                        }
                                    }
                                    if (failed) {
                                        errorTitle = L"bonuses array error";
                                        break;
                                    }
                                }
                                else {
                                    Array<UInt, 4> bonuses;
                                    if (!TokenToBonuses(ary, bonuses, errorDesc, true)) {
                                        errorTitle = L"bonuses array error";
                                        break;
                                    }
                                    for (auto &r : comp->AsCup()->mRounds)
                                        r.mBonuses = bonuses;
                                }
                            }
                            else {
                                errorTitle = L"bonuses array error";
                                errorDesc = L"bonuses array is empty";
                                break;
                            }
                        }
                        UInt numMatchdays = 0;
                        for (auto const &r : comp->AsCup()->mRounds)
                            numMatchdays += r.mFlags.Check(FifamBeg::_2ndLeg) ? 2 : 1;
                        TokenBase *matchdays = comp->GetProperty<TokenBase *>(L"matchdays", nullptr);
                        if (matchdays) {
                            Vector<UShort> days;
                            Vector<UInt> arraySizes;
                            Bool hasArrays = false;
                            if (!TokenToMatchdays(matchdays, days, arraySizes, errorDesc, hasArrays, true)) {
                                errorTitle = L"matchdays error";
                                break;
                            }
                            if (days.size() != numMatchdays) {
                                errorTitle = L"matchdays error";
                                errorDesc = L"match dates count is not same as matchdays count (" + Utils::Format(L"%u", numMatchdays) + L")";
                                break;
                            }
                            if (hasArrays) {
                                Bool failed = false;
                                for (UInt r = 0; r < comp->AsCup()->mRounds.size(); r++) {
                                    UInt numMatchesInRound = comp->AsCup()->mRounds[r].mFlags.Check(FifamBeg::_2ndLeg) ? 2 : 1;
                                    if (arraySizes[r] != numMatchesInRound) {
                                        errorTitle = L"matchdays error";
                                        errorDesc = L"match dates count is not same as matchdays count (" + Utils::Format(L"%u", numMatchdays) + L")";
                                        failed = true;
                                        break;
                                    }
                                }
                                if (failed)
                                    break;
                            }
                            for (UInt m = 0; m < days.size(); m++)
                                comp->AsCup()->mFirstSeasonMatchdays.push_back(days[m]);
                        }
                        TokenBase *matchdays2 = comp->GetProperty<TokenBase *>(L"matchdays2", nullptr);
                        if (matchdays2) {
                            Vector<UShort> days;
                            Vector<UInt> arraySizes;
                            Bool hasArrays = false;
                            if (!TokenToMatchdays(matchdays2, days, arraySizes, errorDesc, hasArrays, true)) {
                                errorTitle = L"matchdays2 error";
                                break;
                            }
                            if (days.size() != numMatchdays) {
                                errorTitle = L"matchdays2 error";
                                errorDesc = L"match dates count is not same as matchdays count (" + Utils::Format(L"%u", numMatchdays) + L")";
                                break;
                            }
                            if (hasArrays) {
                                Bool failed = false;
                                for (UInt r = 0; r < comp->AsCup()->mRounds.size(); r++) {
                                    UInt numMatchesInRound = comp->AsCup()->mRounds[r].mFlags.Check(FifamBeg::_2ndLeg) ? 2 : 1;
                                    if (arraySizes[r] != numMatchesInRound) {
                                        errorTitle = L"matchdays error";
                                        errorDesc = L"match dates count is not same as matchdays count (" + Utils::Format(L"%u", numMatchdays) + L")";
                                        failed = true;
                                        break;
                                    }
                                }
                                if (failed)
                                    break;
                            }
                            for (UInt m = 0; m < days.size(); m++)
                                comp->AsCup()->mSecondSeasonMatchdays.push_back(days[m]);
                        }
                        if (comp->AsCup()->mSecondSeasonMatchdays.empty())
                            comp->AsCup()->mSecondSeasonMatchdays = comp->AsCup()->mFirstSeasonMatchdays;
                    }
                    // validate teams count and competition name
                    String compDescriptor = comp->GetProperty<String>(L"ref", String());
                    if (compDescriptor.empty()) {
                        if (comp->GetProperty<Bool>(L"has_name", false))
                            compDescriptor = L"\"" + FifamTr(comp->mName) + L"\" (@@" + GetDirectReferenceFromCompID(comp->mID, gameId, ucpMode) + L")";
                        else
                            compDescriptor = L"@@" + GetDirectReferenceFromCompID(comp->mID, gameId, ucpMode);
                    }
                    else
                        compDescriptor = L"@" + compDescriptor;
                    if (IsCountryRegion(comp->mID.mRegion, MaxCountryId) && compId.mIndex > 31 && (comp->GetDbType() == FifamCompDbType::League || comp->GetDbType() == FifamCompDbType::Pool)) {
                        errorTitle = L"reached competition limit ( " + compDescriptor + L" )";
                        errorDesc = L"";
                        break;
                    }
                    if (options.checkTeams) {
                        if (comp->mNumTeams == 0) {
                            errorTitle = L"competition is empty";
                            errorDesc = L"no teams in " + compDescriptor;
                            break;
                        }
                    }
                    if (options.checkNames) {
                        Bool requiredName = false;
                        if (comp->GetDbType() == FifamCompDbType::Round || comp->GetDbType() == FifamCompDbType::Cup ||
                            (comp->GetDbType() == FifamCompDbType::League && !IsCountryRegion(comp->mID.mRegion, MaxCountryId)))
                        {
                            requiredName = true;
                        }
                        if (requiredName && !comp->GetProperty<Bool>(L"has_name", false)) {
                            errorTitle = L"competition has no name";
                            errorDesc = L"name is not set for " + compDescriptor;
                            break;
                        }
                    }
                    if (options.checkBonuses) {
                        if (comp->GetDbType() != FifamCompDbType::Pool) {
                            if (!comp->HasProperty(L"bonus") && !comp->HasProperty(L"option_bonus")) {
                                errorTitle = L"competition has no bonuses";
                                errorDesc = L"bonuses are not set for " + compDescriptor;
                                break;
                            }
                        }
                    }
                    if (options.checkMatchdays) {
                        if (comp->GetDbType() != FifamCompDbType::Pool) {
                            if (!comp->HasProperty(L"matchdays") && !comp->HasProperty(L"option_matchdays")) {
                                errorTitle = L"competition has no matchdays";
                                errorDesc = L"matchdays are not set for " + compDescriptor;
                                break;
                            }
                        }
                    }
                    // process instructions
                    errorTitle.clear();
                    errorDesc.clear();
                    Vector<Instruction> *instructions = comp->GetProperty<Vector<Instruction> *>(L"instructions", nullptr);
                    String fmName = L"FIFA Manager " + Utils::Format(L"%02d", gameId);
                    if (instructions) {
                        for (Instruction &ins : *instructions) {

                            auto GetArgumentRef = [&](TokenBase *t, FifamCompID &outId, FifamCompDbType requiredDbType = FifamCompDbType::Root) {
                                if (t->type() == TOKEN_REFERENCE || t->type() == TOKEN_DIRECT_REFERENCE) {
                                    Vector<FifamCompID> ids;
                                    if (!ResolveReferenceList(&db, comp, gCompRefNames, t, ids, 9999, errorTitle, requiredDbType))
                                        return false;
                                    if (ids.size() != 1) {
                                        errorTitle = L"expected reference to a single competition";
                                        return false;
                                    }
                                    outId = ids.front();
                                }
                                else {
                                    errorTitle = L"expected reference or direct reference";
                                    return false;
                                }
                                return true;
                            };
                            auto GetArgumentRefArray = [&](TokenBase *t, Vector<FifamCompID> &outIds, FifamCompDbType requiredDbType = FifamCompDbType::Root) {
                                if (!ResolveReferenceList(&db, comp, gCompRefNames, t, outIds, 9999, errorTitle, requiredDbType))
                                    return false;
                                return true;
                            };
                            auto GetArgumentInt = [&](TokenBase *t, Int &out, Int min, Int max) {
                                if (t->type() == TOKEN_NUMBER)
                                    out = t->as<TokenNumber>()->number;
                                else {
                                    errorTitle = L"'" + t->print() + L"': expected number";
                                    return false;
                                }
                                if (out < min || out > max) {
                                    errorTitle = L"'" + t->print() + Utils::Format(L"': expected number in range from %d to %d", min, max);
                                    return false;
                                }
                                return true;
                            };
                            auto GetArgumentIntArray = [&](TokenBase *t, Vector<Int> &out, Int min, Int max) {
                                if (t->type() == TOKEN_NUMBER)
                                    out.push_back(t->as<TokenNumber>()->number);
                                else if (t->type() == TOKEN_ARRAY) {
                                    for (UInt i = 0; i < t->as<TokenArray>()->values.size(); i++) {
                                        if (t->as<TokenArray>()->values[i]->type() == TOKEN_NUMBER)
                                            out.push_back(t->as<TokenArray>()->values[i]->as<TokenNumber>()->number);
                                        else {
                                            errorTitle = L"'" + t->as<TokenArray>()->values[i]->print() + L"': expected number";
                                            return false;
                                        }
                                    }
                                }
                                else {
                                    errorTitle = L"'" + t->print() + L"': expected number or an array of numbers";
                                    return false;
                                }
                                for (Int i : out) {
                                    if (i < min || i > max) {
                                        errorTitle = L"'" + t->print() + Utils::Format(L"': expected number in range from %d to %d", min, max);
                                        return false;
                                    }
                                }
                                return true;
                            };

                            auto GetArgumentCountryArray = [&](TokenBase *t, Vector<FifamNation> &out) {
                                if (t->type() == TOKEN_NUMBER) {
                                    if (t->as<TokenNumber>()->number >= 1 && t->as<TokenNumber>()->number <= MaxCountryId)
                                        out.push_back(FifamNation::MakeFromInt(t->as<TokenNumber>()->number));
                                    else {
                                        errorTitle = L"'" + t->print() + L"': expected country id or an array of country ids";
                                        return false;
                                    }
                                }
                                else if (t->type() == TOKEN_NAME) {
                                    FifamCompRegion region;
                                    if (GetContinentByName(t->as<TokenName>()->name, region))
                                        GetCountriesList(region, out, gameId, ucpMode);
                                    else if (GetCountryByName(t->as<TokenName>()->name, region)) {
                                        out.push_back(FifamNation::MakeFromInt(region.ToInt()));
                                    }
                                    else {
                                        errorTitle = L"'" + t->print() + L"': expected country id or an array of country ids";
                                        return false;
                                    }
                                }
                                else if (t->type() == TOKEN_ARRAY) {
                                    TokenArray *ary = t->as<TokenArray>();
                                    for (UInt i = 0; i < ary->values.size(); i++) {
                                        if (ary->values[i]->type() == TOKEN_NUMBER) {
                                            if (ary->values[i]->as<TokenNumber>()->number >= 1 && ary->values[i]->as<TokenNumber>()->number <= MaxCountryId)
                                                out.push_back(FifamNation::MakeFromInt(ary->values[i]->as<TokenNumber>()->number));
                                            else {
                                                errorTitle = L"'" + ary->values[i]->print() + L"': expected country id or an array of country ids";
                                                return false;
                                            }
                                        }
                                        else if (ary->values[i]->type() == TOKEN_NAME) {
                                            FifamCompRegion region;
                                            if (GetContinentByName(ary->values[i]->as<TokenName>()->name, region))
                                                GetCountriesList(region, out, gameId, ucpMode);
                                            else if (GetCountryByName(ary->values[i]->as<TokenName>()->name, region)) {
                                                out.push_back(FifamNation::MakeFromInt(region.ToInt()));
                                            }
                                            else {
                                                errorTitle = L"'" + ary->values[i]->print() + L"': expected country id or an array of country ids";
                                                return false;
                                            }
                                        }
                                        else {
                                            errorTitle = L"'" + ary->values[i]->print() + L"': expected country id or an array of country ids";
                                            return false;
                                        }
                                    }
                                }
                                else {
                                    errorTitle = L"'" + t->print() + L"': expected country id or an array of country ids";
                                    return false;
                                }
                                return true;
                            };
                            auto GetArgumentContinentArray = [&](TokenBase *t, Vector<FifamContinent> &out) {
                                if (t->type() == TOKEN_NUMBER) {
                                    if (t->as<TokenNumber>()->number >= 0 && t->as<TokenNumber>()->number <= 5)
                                        out.push_back(FifamContinent::MakeFromInt(t->as<TokenNumber>()->number));
                                    else {
                                        errorTitle = L"'" + t->print() + L"': expected coninent id or an array of coninent ids";
                                        return false;
                                    }
                                }
                                else if (t->type() == TOKEN_NAME) {
                                    FifamCompRegion region;
                                    if (GetContinentByName(t->as<TokenName>()->name, region)) {
                                        if (region == FifamCompRegion::International) {
                                            for (UInt c = 0; c <= 5; c++)
                                                out.push_back(FifamContinent::MakeFromInt(c));
                                        }
                                        else
                                            out.push_back(FifamContinent::MakeFromInt(region.ToInt() - 249));
                                    }
                                    else {
                                        errorTitle = L"'" + t->print() + L"': expected coninent id or an array of coninent ids";
                                        return false;
                                    }
                                }
                                else if (t->type() == TOKEN_ARRAY) {
                                    TokenArray *ary = t->as<TokenArray>();
                                    for (UInt i = 0; i < ary->values.size(); i++) {
                                        if (ary->values[i]->type() == TOKEN_NUMBER) {
                                            if (ary->values[i]->as<TokenNumber>()->number >= 0 && ary->values[i]->as<TokenNumber>()->number <= 5)
                                                out.push_back(FifamContinent::MakeFromInt(ary->values[i]->as<TokenNumber>()->number));
                                            else {
                                                errorTitle = L"'" + ary->values[i]->print() + L"': expected coninent id or an array of coninent ids";
                                                return false;
                                            }
                                        }
                                        else if (ary->values[i]->type() == TOKEN_NAME) {
                                            FifamCompRegion region;
                                            if (GetContinentByName(ary->values[i]->as<TokenName>()->name, region)) {
                                                if (region == FifamCompRegion::International) {
                                                    for (UInt c = 0; c <= 5; c++)
                                                        out.push_back(FifamContinent::MakeFromInt(c));
                                                }
                                                else
                                                    out.push_back(FifamContinent::MakeFromInt(region.ToInt() - 249));
                                            }
                                            else {
                                                errorTitle = L"'" + ary->values[i]->print() + L"': expected coninent id or an array of coninent ids";
                                                return false;
                                            }
                                        }
                                        else {
                                            errorTitle = L"'" + ary->values[i]->print() + L"': expected coninent id or an array of coninent ids";
                                            return false;
                                        }
                                    }
                                }
                                else {
                                    errorTitle = L"'" + t->print() + L"': expected coninent id or an array of coninent ids";
                                    return false;
                                }
                                return true;
                            };

                            auto GetArgumentTeamType = [&](TokenBase *t, FifamClubTeamType &out) {
                                if (t->type() == TOKEN_NUMBER) {
                                    if ((t->as<TokenNumber>()->number >= 0 && t->as<TokenNumber>()->number <= 2) || t->as<TokenNumber>()->number == 4)
                                        out = FifamClubTeamType::MakeFromInt(t->as<TokenNumber>()->number);
                                    else {
                                        errorTitle = L"'" + t->print() + L"': expected team type";
                                        return false;
                                    }
                                }
                                else if (t->type() == TOKEN_NAME) {
                                    if (!GetTeamTypeByName(t->as<TokenName>()->name, out)) {
                                        errorTitle = L"'" + t->print() + L"': expected number";
                                        return false;
                                    }
                                }
                                else {
                                    errorTitle = L"'" + t->print() + L"': expected team type";
                                    return false;
                                }
                                return true;
                            };

                            if (ins.id == FifamInstructionID::ID_BUILD_COUNTER) {
                                if (gameId <= 7 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'buildCounter' works only with pool";
                                    break;
                                }
                                if (!universalMode && gameId > 7) {
                                    errorTitle = L"'buildCounter' does not work with " + fmName;
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'buildCounter': expected 1 argument (assesment position)";
                                    break;
                                }
                                Int position;
                                if (!GetArgumentInt(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'buildCounter': " + errorTitle;
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::BUILD_COUNTER(position));
                            }
                            if (ins.id == FifamInstructionID::ID_FILL_ASSESSMENT_RESERVES) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'fillAssessmentReserves' works only with pool";
                                    break;
                                }
                                if (!universalMode && gameId <= 7) {
                                    errorTitle = L"'fillAssessmentReserves' does not work with " + fmName;
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'fillAssessmentReserves' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::FILL_ASSESSMENT_RESERVES());
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NAT_AMERICA) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getNatAmerica' works only with pool";
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'getNatAmerica' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::GET_NAT_AMERICA());
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NAT_AFRICA) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getNatAfrica' works only with pool";
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'getNatAfrica' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::GET_NAT_AFRICA());
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NAT_ASIA) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getNatAsia' works only with pool";
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'getNatAsia' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::GET_NAT_ASIA());
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NAT_OCEANIA) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getNatOceania' works only with pool";
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'getNatOceania' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::GET_NAT_OCEANIA());
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_HOST) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getHost' works only with pool";
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'getHost' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::GET_HOST());
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_TAB_SPARE) {
                                if (gameId <= 10 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::League && comp->GetDbType() != FifamCompDbType::Cup)) {
                                    errorTitle = L"'getTabSpare' works only with pool, league and cup";
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'getTabSpare' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::GET_TAB_SPARE());
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_CC_SPARE) {
                                if (gameId <= 10 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::League)) {
                                    errorTitle = L"'getTabSpare' works only with pool and league";
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'getCCSpare' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::GET_CC_SPARE());
                            }
                            else if (ins.id == FifamInstructionID::ID_SHUFFLE_TEAMS) {
                                if (!universalMode && gameId <= 12) {
                                    errorTitle = L"'shuffleTeams' does not work with " + fmName;
                                    break;
                                }
                                if (!ins.arguments.empty()) {
                                    errorTitle = L"'shuffleTeams' does not take arguments";
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::SHUFFLE_TEAMS());
                            }
                            else if (ins.id == FifamInstructionID::ID_RESERVE_ASSESSMENT_TEAMS) {
                                if (!universalMode && gameId <= 7) {
                                    errorTitle = L"'reserveAssessmentTeams' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'reserveAssessmentTeams' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 2) {
                                    errorTitle = L"'reserveAssessmentTeams': expected 2 arguments (assesment position, number of reserve spaces)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'reserveAssessmentTeams': " + errorTitle;
                                    break;
                                }
                                Int numSpaces = 0;
                                if (!GetArgumentInt(ins.arguments[1], numSpaces, 1, 9999)) {
                                    errorTitle = L"'reserveAssessmentTeams': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::RESERVE_ASSESSMENT_TEAMS(i, numSpaces));
                            }
                            else if (ins.id == FifamInstructionID::ID_BUILD_UEFA5) {
                                if (!universalMode && gameId > 7) {
                                    errorTitle = L"'buildUEFA5' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'buildUEFA5' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 2) {
                                    errorTitle = L"'buildUEFA5': expected 2 arguments (assesment position, number of reserve spaces)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'buildUEFA5': " + errorTitle;
                                    break;
                                }
                                Int numSpaces = 0;
                                if (!GetArgumentInt(ins.arguments[1], numSpaces, 1, 9999)) {
                                    errorTitle = L"'buildUEFA5': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::BUILD_UEFA5(i, numSpaces));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_EUROPEAN_ASSESSMENT_TEAMS) {
                                if (!universalMode && gameId <= 7) {
                                    errorTitle = L"'getEuropeanAssessmentTeams' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getEuropeanAssessmentTeams' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 3) {
                                    errorTitle = L"'getEuropeanAssessmentTeams': expected 3 arguments (assesment position, league table position, number of teams)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'getEuropeanAssessmentTeams': " + errorTitle;
                                    break;
                                }
                                Int leaguePos = 0;
                                if (!GetArgumentInt(ins.arguments[1], leaguePos, 1, 24)) {
                                    errorTitle = L"'getEuropeanAssessmentTeams': " + errorTitle;
                                    break;
                                }
                                Int numTeams = 0;
                                if (!GetArgumentInt(ins.arguments[2], numTeams, 1, 24)) {
                                    errorTitle = L"'getEuropeanAssessmentTeams': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_EUROPEAN_ASSESSMENT_TEAMS(i, leaguePos, numTeams));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_UEFA5_TAB) {
                                if (!universalMode && gameId > 7) {
                                    errorTitle = L"'getUEFA5Tab' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getUEFA5Tab' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 3) {
                                    errorTitle = L"'getUEFA5Tab': expected 3 arguments (assesment position, league table position, number of teams)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'getUEFA5Tab': " + errorTitle;
                                    break;
                                }
                                Int leaguePos = 0;
                                if (!GetArgumentInt(ins.arguments[1], leaguePos, 1, 24)) {
                                    errorTitle = L"'getUEFA5Tab': " + errorTitle;
                                    break;
                                }
                                Int numTeams = 0;
                                if (!GetArgumentInt(ins.arguments[2], numTeams, 1, 24)) {
                                    errorTitle = L"'getUEFA5Tab': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_UEFA5_TAB(i, leaguePos, numTeams));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_UEFA5_SURE_TAB) {
                                if (!universalMode && gameId > 7) {
                                    errorTitle = L"'getUEFA5SureTab' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getUEFA5SureTab' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 2) {
                                    errorTitle = L"'getUEFA5SureTab': expected 2 arguments (assesment position, unknown)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'getUEFA5SureTab': " + errorTitle;
                                    break;
                                }
                                Int unknown = 0;
                                if (!GetArgumentInt(ins.arguments[1], unknown, 1, 9999)) {
                                    errorTitle = L"'getUEFA5SureTab': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_UEFA5_SURE_TAB(i, unknown));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_UEFA5_SURE_UIC) {
                                if (!universalMode && gameId > 7) {
                                    errorTitle = L"'getUEFA5SureUIC' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 7 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getUEFA5SureUIC' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 2) {
                                    errorTitle = L"'getUEFA5SureUIC': expected 2 arguments (assesment position, unknown)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'getUEFA5SureUIC': " + errorTitle;
                                    break;
                                }
                                Int unknown = 0;
                                if (!GetArgumentInt(ins.arguments[1], unknown, 1, 9999)) {
                                    errorTitle = L"'getUEFA5SureUIC': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_UEFA5_SURE_UIC(i, unknown));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_CHAMP) {
                                if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::Round && comp->GetDbType() != FifamCompDbType::Cup)) {
                                    errorTitle = L"'getChamp' works only with pool, round and cup";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getChamp': expected 1 argument (competition reference)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs)) {
                                    errorTitle = L"'getChamp': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_CHAMP(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_CHAMP_OR_RUNNER_UP) {
                                if (!universalMode && gameId < 13) {
                                    errorTitle = L"'getChampOrRunnerUp' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::Round && comp->GetDbType() != FifamCompDbType::Cup)) {
                                    errorTitle = L"'getChampOrRunnerUp' works only with pool, round and cup";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getChampOrRunnerUp': expected 1 argument (competition reference)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs)) {
                                    errorTitle = L"'getChampOrRunnerUp': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_CHAMP_OR_RUNNER_UP(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_TAB_X_TO_Y) {
                                if (ins.arguments.size() != 3) {
                                    errorTitle = L"'getTabXToY': expected 3 arguments (league reference, table start position, number of teams)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs, FifamCompDbType::League)) {
                                    errorTitle = L"'getTabXToY': " + errorTitle;
                                    break;
                                }
                                Int tablePos = 0;
                                if (!GetArgumentInt(ins.arguments[1], tablePos, 1, 24)) {
                                    errorTitle = L"'getTabXToY': " + errorTitle;
                                    break;
                                }
                                Int teamsCount = 0;
                                if (!GetArgumentInt(ins.arguments[2], teamsCount, 1, 24)) {
                                    errorTitle = L"'getTabXToY': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_TAB_X_TO_Y(id, tablePos, teamsCount));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_TAB_SURE_X_TO_Y_Z) {
                                if (gameId >= 12) {
                                    errorTitle = L"'getTabSureXToYZ' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getTabSureXToYZ' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 4) {
                                    errorTitle = L"'getTabSureXToYZ': expected 4 arguments (league reference, table start position, table end position, number of teams)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs, FifamCompDbType::League)) {
                                    errorTitle = L"'getTabSureXToYZ': " + errorTitle;
                                    break;
                                }
                                Int tableStart = 0;
                                if (!GetArgumentInt(ins.arguments[1], tableStart, 1, 24)) {
                                    errorTitle = L"'getTabSureXToYZ': " + errorTitle;
                                    break;
                                }
                                Int tableEnd = 0;
                                if (!GetArgumentInt(ins.arguments[2], tableEnd, 1, 24)) {
                                    errorTitle = L"'getTabSureXToYZ': " + errorTitle;
                                    break;
                                }
                                Int teamsCount = 0;
                                if (!GetArgumentInt(ins.arguments[3], teamsCount, 1, 24)) {
                                    errorTitle = L"'getTabSureXToYZ': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_TAB_SURE_X_TO_Y_Z(id, tableStart, tableEnd, teamsCount));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_TAB_LEVEL_X_TO_Y) {
                                if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::Cup)) {
                                    errorTitle = L"'getTabLevelXToY' works only with pool and cup";
                                    break;
                                }
                                if (ins.arguments.size() != 3) {
                                    errorTitle = L"'getTabLevelXToY': expected 3 arguments (league level, table start position, number of teams)";
                                    break;
                                }
                                Vector<Int> levels;
                                if (!GetArgumentIntArray(ins.arguments[0], levels, 1, 16)) {
                                    errorTitle = L"'getTabLevelXToY': " + errorTitle;
                                    break;
                                }
                                Int tableStart = 0;
                                if (!GetArgumentInt(ins.arguments[1], tableStart, 1, 24)) {
                                    errorTitle = L"'getTabLevelXToY': " + errorTitle;
                                    break;
                                }
                                Int teamsCount = 0;
                                if (!GetArgumentInt(ins.arguments[2], teamsCount, 1, 24)) {
                                    errorTitle = L"'getTabLevelXToY': " + errorTitle;
                                    break;
                                }
                                for (int i : levels)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_TAB_LEVEL_X_TO_Y(i - 1, tableStart, teamsCount));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_TAB_LEVEL_START_X_TO_Y) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::League) {
                                    errorTitle = L"'getTabLevelStartXToY' works only with league";
                                    break;
                                }
                                if (ins.arguments.size() != 3) {
                                    errorTitle = L"'getTabLevelStartXToY': expected 3 arguments (league level, table start position, number of teams)";
                                    break;
                                }
                                Vector<Int> levels;
                                if (!GetArgumentIntArray(ins.arguments[0], levels, 1, 16)) {
                                    errorTitle = L"'getTabLevelStartXToY': " + errorTitle;
                                    break;
                                }
                                Int tableStart = 0;
                                if (!GetArgumentInt(ins.arguments[1], tableStart, 1, 24)) {
                                    errorTitle = L"'getTabLevelStartXToY': " + errorTitle;
                                    break;
                                }
                                Int teamsCount = 0;
                                if (!GetArgumentInt(ins.arguments[2], teamsCount, 1, 24)) {
                                    errorTitle = L"'getTabLevelStartXToY': " + errorTitle;
                                    break;
                                }
                                for (int i : levels)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_TAB_LEVEL_START_X_TO_Y(i - 1, tableStart, teamsCount));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_EUROPEAN_ASSESSMENT_CUPWINNER) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getEuropeanAssessmentCupwinner' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getEuropeanAssessmentCupwinner': expected 1 argument (assesment position)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'getEuropeanAssessmentCupwinner': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_EUROPEAN_ASSESSMENT_CUPWINNER(i));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_WINNER) {
                                if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::Round)) {
                                    errorTitle = L"'getWinner' works only with pool and round";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getWinner': expected 1 arguments (round reference)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs, FifamCompDbType::Round)) {
                                    errorTitle = L"'getWinner': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_WINNER(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_LOSER) {
                                if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::Round)) {
                                    errorTitle = L"'getLoser' works only with pool and round";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getLoser': expected 1 arguments (round reference)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs, FifamCompDbType::Round)) {
                                    errorTitle = L"'getLoser': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_LOSER(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_POOL) {
                                if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::League && comp->GetDbType() != FifamCompDbType::Round)) {
                                    errorTitle = L"'getPool' works only with pool, league and round";
                                    break;
                                }
                                if (ins.arguments.size() != 3) {
                                    errorTitle = L"'getPool': expected 3 arguments (pool reference, pool start position, number of teams)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs, FifamCompDbType::Pool)) {
                                    errorTitle = L"'getPool': " + errorTitle;
                                    break;
                                }
                                Int startPos = 0;
                                if (!GetArgumentInt(ins.arguments[1], startPos, 1, 9999)) {
                                    errorTitle = L"'getPool': " + errorTitle;
                                    break;
                                }
                                Int teamsCount = 0;
                                if (!GetArgumentInt(ins.arguments[2], teamsCount, 1, 9999)) {
                                    errorTitle = L"'getPool': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_POOL(id, startPos - 1, teamsCount));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NAT_UEFA5_WITH_HOST) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getNatUEFA5WithHost' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getNatUEFA5WithHost': expected 1 argument (assesment position)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'getNatUEFA5WithHost': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_NAT_UEFA5_WITH_HOST(i));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NAT_UEFA5_WITHOUT_HOST) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getNatUEFA5WithoutHost' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getNatUEFA5WithoutHost': expected 1 argument (assesment position)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'getNatUEFA5WithoutHost': " + errorTitle;
                                    break;
                                }
                                for (auto i : position)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_NAT_UEFA5_WITHOUT_HOST(i));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NAT_SOUTH_AMERICA) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getNatSouthAmerica' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getNatSouthAmerica': expected 1 argument (country id)";
                                    break;
                                }
                                Vector<FifamNation> countries;
                                if (!GetArgumentCountryArray(ins.arguments[0], countries)) {
                                    errorTitle = L"'getNatSouthAmerica': " + errorTitle;
                                    break;
                                }
                                for (auto i : countries)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_NAT_SOUTH_AMERICA(i));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_RUNNER_UP) {
                                if (!universalMode && gameId <= 9) {
                                    errorTitle = L"'getRunnerUp' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::Round && comp->GetDbType() != FifamCompDbType::Cup)) {
                                    errorTitle = L"'getRunnerUp' works only with pool, round and cup";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getRunnerUp': expected 1 argument (competition reference)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs)) {
                                    errorTitle = L"'getRunnerUp': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_RUNNER_UP(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_RELEGATED_TEAMS) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getRelegatedTeams' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getRelegatedTeams': expected 1 argument (league reference)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs, FifamCompDbType::League)) {
                                    errorTitle = L"'getRelegatedTeams': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_RELEGATED_TEAMS(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_UEFA5_CHAMP_OR_FINALIST) {
                                if (!universalMode && gameId > 7) {
                                    errorTitle = L"'getUEFA5ChampOrFinalist' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 7 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getUEFA5ChampOrFinalist' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 2) {
                                    errorTitle = L"'getUEFA5ChampOrFinalist': expected 2 arguments (assesment position, competition reference)";
                                    break;
                                }
                                Vector<Int> position;
                                if (!GetArgumentIntArray(ins.arguments[0], position, 1, MaxCountryId)) {
                                    errorTitle = L"'getUEFA5ChampOrFinalist': " + errorTitle;
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[1], compIDs)) {
                                    errorTitle = L"'getUEFA5ChampOrFinalist': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs) {
                                    for (auto i : position)
                                        comp->mInstructions.PushBack(new FifamInstruction::GET_UEFA5_CHAMP_OR_FINALIST(i, id));
                                }
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_INTERNATIONAL_TAB_LEVEL_X_TO_Y) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getInternationalTabLevelXToY' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 4) {
                                    errorTitle = L"'getInternationalTabLevelXToY': expected 4 arguments (country id, league level, start position, number of teams)";
                                    break;
                                }
                                Vector<FifamNation> countries;
                                if (!GetArgumentCountryArray(ins.arguments[0], countries)) {
                                    errorTitle = L"'getInternationalTabLevelXToY': " + errorTitle;
                                    break;
                                }
                                Vector<Int> level;
                                if (!GetArgumentIntArray(ins.arguments[1], level, 1, 16)) {
                                    errorTitle = L"'getInternationalTabLevelXToY': " + errorTitle;
                                    break;
                                }
                                Int startPosition = 0;
                                if (!GetArgumentInt(ins.arguments[2], startPosition, 1, 24)) {
                                    errorTitle = L"'getInternationalTabLevelXToY': " + errorTitle;
                                    break;
                                }
                                Int numTeams = 0;
                                if (!GetArgumentInt(ins.arguments[3], numTeams, 1, 24)) {
                                    errorTitle = L"'getInternationalTabLevelXToY': " + errorTitle;
                                    break;
                                }
                                for (auto i : level) {
                                    for (auto c : countries)
                                        comp->mInstructions.PushBack(new FifamInstruction::GET_INTERNATIONAL_TAB_LEVEL_X_TO_Y(c, i - 1, startPosition, numTeams));
                                }
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_INTERNATIONAL_SPARE) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getInternationalSpare' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 2) {
                                    errorTitle = L"'getInternationalSpare': expected 2 arguments (country id, number of teams)";
                                    break;
                                }
                                Vector<FifamNation> countries;
                                if (!GetArgumentCountryArray(ins.arguments[0], countries)) {
                                    errorTitle = L"'getInternationalSpare': " + errorTitle;
                                    break;
                                }
                                Int numTeams = 0;
                                if (!GetArgumentInt(ins.arguments[1], numTeams, 1, 9999)) {
                                    errorTitle = L"'getInternationalSpare': " + errorTitle;
                                    break;
                                }
                                for (auto c : countries)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_INTERNATIONAL_SPARE(c, numTeams));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_TAB_LEVEL_INDOOR) {
                                if (!universalMode && gameId > 9) {
                                    errorTitle = L"'getTabLevelIndoor' does not work with " + fmName;
                                    break;
                                }
                                if (gameId <= 9 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getTabLevelIndoor' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 3) {
                                    errorTitle = L"'getTabLevelIndoor': expected 3 arguments (league level, table start position, number of teams)";
                                    break;
                                }
                                Vector<Int> levels;
                                if (!GetArgumentIntArray(ins.arguments[0], levels, 1, 16)) {
                                    errorTitle = L"'getTabLevelIndoor': " + errorTitle;
                                    break;
                                }
                                Int tableStart = 0;
                                if (!GetArgumentInt(ins.arguments[1], tableStart, 1, 24)) {
                                    errorTitle = L"'getTabLevelIndoor': " + errorTitle;
                                    break;
                                }
                                Int teamsCount = 0;
                                if (!GetArgumentInt(ins.arguments[2], teamsCount, 1, 24)) {
                                    errorTitle = L"'getTabLevelIndoor': " + errorTitle;
                                    break;
                                }
                                for (int i : levels)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_TAB_LEVEL_INDOOR(i - 1, tableStart, teamsCount));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_INTERNATIONAL_TEAMS) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getInternationalTeams' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 2) {
                                    errorTitle = L"'getInternationalTeams': expected 2 arguments (country id, number of teams)";
                                    break;
                                }
                                Vector<FifamNation> countries;
                                if (!GetArgumentCountryArray(ins.arguments[0], countries)) {
                                    errorTitle = L"'getInternationalTeams': " + errorTitle;
                                    break;
                                }
                                Int numTeams = 0;
                                if (!GetArgumentInt(ins.arguments[1], numTeams, 1, 9999)) {
                                    errorTitle = L"'getInternationalTeams': " + errorTitle;
                                    break;
                                }
                                for (auto c : countries)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_INTERNATIONAL_TEAMS(c, numTeams));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_CC_FA_WINNER) {
                                if (gameId <= 11 && comp->GetDbType() != FifamCompDbType::Cup) {
                                    errorTitle = L"'getInternationalTeams' works only with cup";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getCCFaWinner': expected 1 argument (competition reference)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs)) {
                                    errorTitle = L"'getCCFaWinner': " + errorTitle;
                                    break;
                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_CC_FA_WINNER(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_CHAMP_COUNTRY_TEAM) {
                                if (!universalMode && gameId <= 7) {
                                    errorTitle = L"'getChampCountryTeam' does not work with " + fmName;
                                    break;
                                }
                                if ((gameId > 7 && gameId <= 11) && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getChampCountryTeam' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getChampCountryTeam': expected 1 argument (competition reference)";
                                    break;
                                }
                                Vector<FifamCompID> compIDs;
                                if (!GetArgumentRefArray(ins.arguments[0], compIDs)) {
                                    errorTitle = L"'getChampCountryTeam': " + errorTitle;
                                    break;

                                }
                                for (auto const &id : compIDs)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_CHAMP_COUNTRY_TEAM(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_RANDOM_NATIONAL_TEAM) {
                                if (!universalMode && gameId <= 7) {
                                    errorTitle = L"'getRandomNationalTeam' does not work with " + fmName;
                                    break;
                                }
                                if ((gameId > 7 && gameId <= 11) && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getRandomNationalTeam' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 2) {
                                    errorTitle = L"'getRandomNationalTeam': expected 2 arguments (continent id, number of teams)";
                                    break;
                                }
                                Vector<FifamContinent> continents;
                                if (!GetArgumentContinentArray(ins.arguments[0], continents)) {
                                    errorTitle = L"'getRandomNationalTeam': " + errorTitle;
                                    break;
                                }
                                Int numTeams = 0;
                                if (!GetArgumentInt(ins.arguments[1], numTeams, 1, 9999)) {
                                    errorTitle = L"'getRandomNationalTeam': " + errorTitle;
                                    break;
                                }
                                for (auto c : continents)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_RANDOM_NATIONAL_TEAM(c, numTeams));
                            }
                            else if (ins.id == FifamInstructionID::ID_CHANGE_TEAM_TYPES) {
                                if (!universalMode && gameId <= 7) {
                                    errorTitle = L"'changeTeamTypes' does not work with " + fmName;
                                    break;
                                }
                                if ((gameId > 7 && gameId <= 11) && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::League && comp->GetDbType() != FifamCompDbType::Round)) {
                                    errorTitle = L"'changeTeamTypes' works only with pool, league and round";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'changeTeamTypes': expected 1 arguments (team type)";
                                    break;
                                }
                                FifamClubTeamType teamType;
                                if (!GetArgumentTeamType(ins.arguments[0], teamType)) {
                                    errorTitle = L"'changeTeamTypes': " + errorTitle;
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::CHANGE_TEAM_TYPES(teamType));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_FAIRNESS_TEAM) {
                                if (!universalMode && gameId <= 7) {
                                    errorTitle = L"'getFairnessTeam' does not work with " + fmName;
                                    break;
                                }
                                if ((gameId > 7 && gameId <= 11) && comp->GetDbType() != FifamCompDbType::Pool) {
                                    errorTitle = L"'getFairnessTeam' works only with pool";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getFairnessTeam': expected 1 arguments (number of teams)";
                                    break;
                                }
                                Int numTeams;
                                if (!GetArgumentInt(ins.arguments[0], numTeams, 1, MaxCountryId)) {
                                    errorTitle = L"'getFairnessTeam': " + errorTitle;
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::GET_FAIRNESS_TEAM(numTeams));
                            }
                            else if (ins.id == FifamInstructionID::ID_COPY_LEAGUE_DATA) {
                                UInt minSupportedGame = IsCountryRegion(comp->mID.mRegion, MaxCountryId) ? 13 : 8;
                                if (!universalMode) {
                                    if (gameId < minSupportedGame) {
                                        errorTitle = L"'copyLeagueData' does not work with " + fmName;
                                        break;
                                    }
                                }
                                if ((gameId > minSupportedGame && gameId <= 11) && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::League)) {
                                    errorTitle = L"'copyLeagueData' works only with pool and league";
                                    break;
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'copyLeagueData': expected 1 arguments (league reference)";
                                    break;
                                }
                                FifamCompID id;
                                if (!GetArgumentRef(ins.arguments[0], id, FifamCompDbType::League)) {
                                    errorTitle = L"'copyLeagueData': " + errorTitle;
                                    break;
                                }
                                comp->mInstructions.PushBack(new FifamInstruction::COPY_LEAGUE_DATA(id));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NATIONAL_TEAM) {
                                if (!universalMode) {
                                    UInt minSupportedGame = IsCountryRegion(comp->mID.mRegion, MaxCountryId) ? 13 : 12;
                                    if (gameId < minSupportedGame) {
                                        errorTitle = L"'getNationalTeam' does not work with " + fmName;
                                        break;
                                    }
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getNationalTeam': expected 1 arguments (country id)";
                                    break;
                                }
                                Vector<FifamNation> countries;
                                if (!GetArgumentCountryArray(ins.arguments[0], countries)) {
                                    errorTitle = L"'getNationalTeam': " + errorTitle;
                                    break;
                                }
                                for (auto i : countries)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_NATIONAL_TEAM(i));
                            }
                            else if (ins.id == FifamInstructionID::ID_GET_NATIONAL_TEAM_WITHOUT_HOST) {
                                if (!universalMode) {
                                    UInt minSupportedGame = IsCountryRegion(comp->mID.mRegion, MaxCountryId) ? 13 : 12;
                                    if (gameId < minSupportedGame) {
                                        errorTitle = L"'getNationalTeamWithoutHost' does not work with " + fmName;
                                        break;
                                    }
                                }
                                if (ins.arguments.size() != 1) {
                                    errorTitle = L"'getNationalTeamWithoutHost': expected 1 arguments (country id)";
                                    break;
                                }
                                Vector<FifamNation> countries;
                                if (!GetArgumentCountryArray(ins.arguments[0], countries)) {
                                    errorTitle = L"'getNationalTeamWithoutHost': " + errorTitle;
                                    break;
                                }
                                for (auto i : countries)
                                    comp->mInstructions.PushBack(new FifamInstruction::GET_NATIONAL_TEAM_WITHOUT_HOST(i));
                            }
                            else if (ins.id == FifamInstructionID::ID_END_OF_ENTRY) {
                                if (ins.specialId == L"get" || ins.specialId == L"getAll" || ins.specialId == L"getBottom" || ins.specialId == L"getBottomAll" || ins.specialId == L"getFromTo" || ins.specialId == L"getBottomFromTo") {
                                    Bool fromTo = ins.specialId == L"getFromTo" || ins.specialId == L"getBottomFromTo";
                                    if (fromTo) {
                                        if (ins.arguments.size() < 1) {
                                            errorTitle = L"'" + ins.specialId + L"': expected 3 arguments (competition reference, start position, end position)";
                                            break;
                                        }
                                    }
                                    else {
                                        if (ins.arguments.size() < 1) {
                                            errorTitle = L"'" + ins.specialId + L"': expected at least 1 argument (competition reference, [position], [number of teams])";
                                            break;
                                        }
                                    }
                                    // do not allow direct references
                                    Bool failed = false;
                                    if (ins.arguments[0]->type() == TOKEN_DIRECT_REFERENCE) {
                                        errorTitle = L"'" + ins.specialId + L"': direct references are not allowed";
                                        break;
                                    }
                                    else if (ins.arguments[0]->type() == TOKEN_ARRAY) {
                                        for (auto r : ins.arguments[0]->as<TokenArray>()->values) {
                                            if (r->type() == TOKEN_DIRECT_REFERENCE) {
                                                errorTitle = L"'" + ins.specialId + L"': direct references are not allowed";
                                                failed = true;
                                                break;
                                            }
                                        }
                                    }
                                    if (failed)
                                        break;
                                    // get all comp ids
                                    Vector<FifamCompID> compIDs;
                                    if (!GetArgumentRefArray(ins.arguments[0], compIDs)) {
                                        errorTitle = L"'" + ins.specialId + L"': " + errorTitle;
                                        break;
                                    }
                                    // get competitions ptrs
                                    Vector<FifamCompetition *> comps;
                                    for (auto const &id : compIDs) {
                                        FifamCompetition *comp = db.GetCompetition(id);
                                        if (!comp) {
                                            errorTitle = L"'" + ins.specialId + L"': unable to retrieve competition";
                                            failed = true;
                                            break;
                                        }
                                        comps.push_back(comp);
                                    }
                                    if (failed)
                                        break;
                                    // check if all comps have same type
                                    FifamCompDbType dbType = FifamCompDbType::Root;
                                    for (auto comp : comps) {
                                        if (dbType == FifamCompDbType::Root)
                                            dbType = comp->GetDbType();
                                        else {
                                            if (comp->GetDbType() != dbType) {
                                                errorTitle = L"'" + ins.specialId + L"': all competitions must have the same type";
                                                failed = true;
                                                break;
                                            }
                                        }
                                    }
                                    if (failed)
                                        break;
                                    Bool all = ins.specialId == L"getAll" || ins.specialId == L"getBottomAll";
                                    Bool bottom = ins.specialId == L"getBottom" || ins.specialId == L"getBottomAll" || ins.specialId == L"getBottomFromTo";
                                    String compDbTypeStr;
                                    if (comp->GetDbType() == FifamCompDbType::League)
                                        compDbTypeStr = L"league";
                                    else if (comp->GetDbType() == FifamCompDbType::Round)
                                        compDbTypeStr = L"round";
                                    else if (comp->GetDbType() == FifamCompDbType::Cup)
                                        compDbTypeStr = L"cup";
                                    else
                                        compDbTypeStr = L"pool";
                                    if (dbType == FifamCompDbType::Pool) {
                                        if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::League && comp->GetDbType() != FifamCompDbType::Round)) {
                                            errorTitle = L"'" + ins.specialId + L"' works only with pool, league and round";
                                            break;
                                        }
                                        Int startPos = -1;
                                        if (all)
                                            startPos = 1;
                                        Int numTeams = -1;
                                        if (ins.arguments.size() != 1) {
                                            if (ins.arguments.size() == 2) {
                                                if (fromTo) {
                                                    errorTitle = L"'" + ins.specialId + L"': expected 3 arguments (competition reference, start position, end position)";
                                                    break;
                                                }
                                                numTeams = 1;
                                                if (!GetArgumentInt(ins.arguments[1], startPos, 1, 9999)) {
                                                    errorTitle = L"'" + ins.specialId + L"': " + errorTitle;
                                                    break;
                                                }
                                            }
                                            else {
                                                if (all) {
                                                    errorTitle = L"'" + ins.specialId + L"': expected 1 or 2 arguments (competition reference, [position])";
                                                    break;
                                                }
                                                else if (ins.arguments.size() == 3) {
                                                    if (!GetArgumentInt(ins.arguments[1], startPos, 1, 9999)) {
                                                        errorTitle = L"'" + ins.specialId + L"': " + errorTitle;
                                                        break;
                                                    }
                                                    if (!GetArgumentInt(ins.arguments[2], numTeams, 1, 9999)) {
                                                        errorTitle = L"'" + ins.specialId + L"': " + errorTitle;
                                                        break;
                                                    }
                                                    if (fromTo) {
                                                        if (numTeams < startPos) {
                                                            errorTitle = L"'" + ins.specialId + L"': end position must be greater or equal than start position";
                                                            break;
                                                        }
                                                        numTeams = numTeams - startPos + 1;
                                                    }
                                                }
                                                else {
                                                    if (fromTo) {
                                                        errorTitle = L"'" + ins.specialId + L"': expected 3 arguments (competition reference, start position, end position)";
                                                        break;
                                                    }
                                                    errorTitle = L"'" + ins.specialId + L"': expected 1, 2 or 3 arguments (competition reference, [position], [number of teams])";
                                                    break;
                                                }
                                            }
                                        }
                                        else {
                                            if (fromTo) {
                                                errorTitle = L"'" + ins.specialId + L"': expected 3 arguments (competition reference, start position, end position)";
                                                break;
                                            }
                                        }
                                        for (UInt c = 0; c < comps.size(); c++) {
                                            // GET_POOL
                                            Int numTeamsInPool = comps[c]->mNumTeams;
                                            Int maxTeamsAllowed = numTeamsInPool; // Utils::Min(numTeamsInPool, (Int)comp->mNumTeams);
                                            Int mod_startPos = startPos;
                                            Int mod_numTeams = numTeams;
                                            if (mod_startPos == -1) {
                                                mod_startPos = 1;
                                                mod_numTeams = maxTeamsAllowed;
                                            }
                                            else if (mod_numTeams == -1) {
                                                mod_numTeams = numTeamsInPool - (mod_startPos - 1);
                                                if (mod_numTeams < 0) {
                                                    errorTitle = L"'" + ins.specialId + L"': out of pool boundaries";
                                                    break;
                                                }
                                            }
                                            else {
                                                if ((mod_startPos - 1 + mod_numTeams) > numTeamsInPool) {
                                                    errorTitle = L"'" + ins.specialId + L"': out of pool boundaries";
                                                    break;
                                                }
                                            }
                                            if (mod_numTeams > maxTeamsAllowed) {
                                                errorTitle = L"'" + ins.specialId + L"': out of " + compDbTypeStr + L" boundaries";
                                                break;
                                            }
                                            if (bottom) {
                                                if (numTeamsInPool != 0)
                                                    mod_startPos = numTeamsInPool - mod_startPos + 1 - numTeams + 1;
                                                else
                                                    mod_startPos = 1;
                                            }
                                            comp->mInstructions.PushBack(new FifamInstruction::GET_POOL(compIDs[c], mod_startPos - 1, mod_numTeams));
                                        }
                                    }
                                    else if (dbType == FifamCompDbType::League) {
                                        Int startPos = -1;
                                        if (all)
                                            startPos = 1;
                                        Int numTeams = -1;
                                        if (ins.arguments.size() != 1) {
                                            if (ins.arguments.size() == 2) {
                                                if (fromTo) {
                                                    errorTitle = L"'" + ins.specialId + L"': expected 3 arguments (competition reference, start position, end position)";
                                                    break;
                                                }
                                                numTeams = 1;
                                                if (!GetArgumentInt(ins.arguments[1], startPos, 1, 24)) {
                                                    errorTitle = L"'" + ins.specialId + L"': " + errorTitle;
                                                    break;
                                                }
                                            }
                                            else {
                                                if (all) {
                                                    errorTitle = L"'" + ins.specialId + L"': expected 1 or 2 arguments (competition reference, [position])";
                                                    break;
                                                }
                                                else if (ins.arguments.size() == 3) {
                                                    if (!GetArgumentInt(ins.arguments[1], startPos, 1, 24)) {
                                                        errorTitle = L"'" + ins.specialId + L"': " + errorTitle;
                                                        break;
                                                    }
                                                    if (!GetArgumentInt(ins.arguments[2], numTeams, 1, 24)) {
                                                        errorTitle = L"'" + ins.specialId + L"': " + errorTitle;
                                                        break;
                                                    }
                                                    if (fromTo) {
                                                        if (numTeams < startPos) {
                                                            errorTitle = L"'" + ins.specialId + L"': end position must be greater or equal than start position";
                                                            break;
                                                        }
                                                        numTeams = numTeams - startPos + 1;
                                                    }
                                                }
                                                else {
                                                    if (fromTo) {
                                                        errorTitle = L"'" + ins.specialId + L"': expected 3 arguments (competition reference, start position, end position)";
                                                        break;
                                                    }
                                                    errorTitle = L"'" + ins.specialId + L"': expected 1, 2 or 3 arguments (competition reference, [position], [number of teams])";
                                                    break;
                                                }
                                            }
                                        }
                                        else {
                                            if (fromTo) {
                                                errorTitle = L"'" + ins.specialId + L"': expected 3 arguments (competition reference, start position, end position)";
                                                break;
                                            }
                                        }
                                        for (UInt c = 0; c < comps.size(); c++) {
                                            // GET_TAB_X_TO_Y
                                            Int numTeamsInLeague = comps[c]->mNumTeams;
                                            Int maxTeamsAllowed = numTeamsInLeague; // Utils::Min(numTeamsInLeague, (Int)comp->mNumTeams);
                                            Int mod_startPos = startPos;
                                            Int mod_numTeams = numTeams;
                                            if (mod_startPos == -1) {
                                                mod_startPos = 1;
                                                mod_numTeams = maxTeamsAllowed;
                                            }
                                            else if (mod_numTeams == -1) {
                                                mod_numTeams = numTeamsInLeague - (mod_startPos - 1);
                                                if (mod_numTeams < 0) {
                                                    errorTitle = L"'" + ins.specialId + L"': out of league boundaries";
                                                    break;
                                                }
                                            }
                                            else {
                                                if ((mod_startPos - 1 + mod_numTeams) > numTeamsInLeague) {
                                                    errorTitle = L"'" + ins.specialId + L"': out of league boundaries";
                                                    break;
                                                }
                                            }
                                            if (mod_numTeams > maxTeamsAllowed) {
                                                errorTitle = L"'" + ins.specialId + L"': out of " + compDbTypeStr + L" boundaries";
                                                break;
                                            }
                                            if (bottom) {
                                                if (numTeamsInLeague != 0)
                                                    mod_startPos = numTeamsInLeague - mod_startPos + 1 - numTeams + 1;
                                                else
                                                    mod_startPos = 1;
                                            }
                                            comp->mInstructions.PushBack(new FifamInstruction::GET_TAB_X_TO_Y(compIDs[c], mod_startPos, mod_numTeams));
                                        }
                                    }
                                    else {
                                        if (ins.specialId == L"get" || ins.specialId == L"getAll") {
                                            if (dbType == FifamCompDbType::Round) {
                                                if (gameId <= 11 && (comp->GetDbType() != FifamCompDbType::Pool && comp->GetDbType() != FifamCompDbType::Round)) {
                                                    errorTitle = L"'" + ins.specialId + L"' with round as argument works only with pool and round";
                                                    break;
                                                }
                                                if (ins.arguments.size() != 1) {
                                                    errorTitle = L"'" + ins.specialId + L"' with round as argument: expected 1 argument (round reference)";
                                                    break;
                                                }
                                                for (UInt c = 0; c < comps.size(); c++) {
                                                    if ((comps[c]->mNumTeams / 2) > comp->mNumTeams) {
                                                        errorTitle = L"'" + ins.specialId + L"': out of " + compDbTypeStr + L" boundaries";
                                                        break;
                                                    }
                                                    comp->mInstructions.PushBack(new FifamInstruction::GET_WINNER(compIDs[c]));
                                                }
                                            }
                                            else {
                                                errorTitle = L"'" + ins.specialId + L"' works only with pool, league and round";
                                                break;
                                            }
                                        }
                                        else {
                                            errorTitle = L"'" + ins.specialId + L"' works only with pool and league";
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        if (!errorTitle.empty()) {
                            errorTitle += L" (in " + compDescriptor + L")";
                            break;
                        }
                    }
                }
            }
        }
        }
        }
        }
    }

    String compsStr, fixtureStr, dataStr;
    FifamWriter writer(&compsStr, gameId, FifamVersion(0x2013, 0xA), gameId > 7);
    writer.SetVersion(GetFifamVersion(gameId));
    writer.SetReplaceQuotes(false);
    FifamWriter writerFixture(&fixtureStr, gameId, FifamVersion(0x2013, 0xA), gameId > 7);
    writerFixture.SetVersion(GetFifamVersion(gameId));
    writerFixture.SetReplaceQuotes(false);
    FifamWriter writerData(&dataStr, gameId, FifamVersion(0x2013, 0xA), gameId > 7);
    writerData.SetVersion(GetFifamVersion(gameId));
    writerData.SetReplaceQuotes(false);

    if (!errorTitle.empty()) {
        writer.WriteLine(errorTitle);
        writer.Write(errorDesc);
        writerFixture.WriteLine(errorTitle);
        writerFixture.Write(errorDesc);
    }
    else {
        Bool isCountryScript = false;
        //for (auto t : tokens)
        //    writer.WriteLine(t->printFull());
        UInt compsCount = 0;
        FifamCompetition *firstComp = nullptr;
        for (auto[compId, comp] : db.mCompMap) {
            if (comp->GetDbType() != FifamCompDbType::Root && !comp->GetProperty(L"dontWrite", false)) {
                compsCount++;
                if (!firstComp)
                    firstComp = comp;
            }
        }
        if (firstComp) {
            writer.WriteLine(L"; ----------------------------------------------------");
            writer.WriteLine(L"; Compiled with ScriptStudio " + compilerVersion);
            writer.WriteLine(L"; ----------------------------------------------------");
            if (!scriptName.empty() || !scriptVersion.empty() || !scriptAuthor.empty() || !scriptDesc.empty()) {
                auto WriteScriptInfo = [](FifamWriter &w, String const &info, String const &infoStartDesc = String()) {
                    if (!info.empty()) {
                        auto lines = Utils::Split(info, L'\n', false, false);
                        for (UInt l = 0; l < lines.size(); l++) {
                            w.Write(L"; ");
                            if (l == 0 && !infoStartDesc.empty())
                                w.Write(infoStartDesc + L" ");
                            w.WriteLine(lines[l]);
                        }
                    }
                };
                WriteScriptInfo(writer, scriptName);
                WriteScriptInfo(writer, scriptVersion, L"version");
                WriteScriptInfo(writer, scriptAuthor, L"made by");
                if (!scriptDesc.empty()) {
                    if (!scriptName.empty() && (!scriptVersion.empty() || !scriptAuthor.empty()))
                        writer.WriteLine(L";");
                    WriteScriptInfo(writer, scriptDesc);
                }
                writer.WriteLine(L"; ----------------------------------------------------");
            }
            if (IsCountryRegion(firstComp->mID.mRegion, MaxCountryId)) {
                writer.WriteVersion();
                writerFixture.WriteVersion();
                isCountryScript = true;
            }
            else {
                if (blockStartIndex == 0)
                    writer.WriteLine(compsCount);
            }
            if (isCountryScript) {
                writerData.WriteLine(compsCount + 1);
                String compQuoted = L"\"Competition\"";
                writerData.WriteLine(Utils::Format(L"%2X000000", firstComp->mID.mRegion.ToInt()),
                    compQuoted, compQuoted, compQuoted, compQuoted, compQuoted, compQuoted);
            }
        }
        Map<String, Int> blockIndex;
        UInt numCompsWritten = 0;
        for (auto[compId, comp] : db.mCompMap) {
            if (comp->GetDbType() != FifamCompDbType::Root && !comp->GetProperty(L"dontWrite", false)) {
                if (numCompsWritten == 0)
                    writer.WriteLine(L"; ----------------------------------------------------");
                writer.Write(L"; ");
                String refName = comp->GetProperty<String>(L"ref", String());
                Bool hasName = comp->GetProperty<Bool>(L"has_name", false);
                String name = FifamTr(comp->mName);
                String directRefName = GetDirectReferenceFromCompID(comp->mID, gameId, ucpMode);
                if (!refName.empty())
                    writer.Write(L"@" + refName + L" - ");
                if (!hasName)
                    writer.Write(L"[Unnamed] ( @@");
                else
                    writer.Write(L"\"" + name + L"\" ( @@");
                writer.Write(directRefName);
                writer.WriteLine(L" )");
                writer.WriteLine(L"; ----------------------------------------------------");
                String compBlockName;
                UInt compIndex = 0;
                if (IsCountryRegion(comp->mID.mRegion, MaxCountryId)) {
                    if (blockName.empty())
                        writer.WriteStartIndex(L"COMPETITION");
                    else
                        writer.WriteStartIndex(blockName);
                }
                else {
                    if (blockName.empty())
                        compBlockName = GetBlockNameForCompetition(comp->mID);
                    else
                        compBlockName = blockName;
                    compIndex = blockStartIndex + blockIndex[compBlockName]++;
                    writer.WriteStartIndex(Utils::Format(L"%s%d", compBlockName, compIndex));
                    writer.SetVersion(0, 0);
                }
                db.WriteCompetition(writer, comp, FifamNation::None);
                if (IsCountryRegion(comp->mID.mRegion, MaxCountryId))
                    writer.WriteEndIndex(L"COMPETITION");
                else
                    writer.WriteEndIndex(Utils::Format(L"%s%d", compBlockName, compIndex));
                writer.WriteLine(L"; ----------------------------------------------------");

                if (isCountryScript) {
                    writerData.WriteLine(comp->mID.ToHexStr(), Quoted(comp->mName[0]), Quoted(comp->mName[1]), Quoted(comp->mName[2]),
                        Quoted(comp->mName[3]), Quoted(comp->mName[4]), Quoted(comp->mName[5]));

                    writerFixture.WriteLine(L"; ----------------------------------------------------");
                    writerFixture.WriteStartIndex(L"COMPETITION");
                    writerFixture.WriteLine(comp->mID.ToStr());
                    writerFixture.WriteLine(comp->GetDbType().ToStr());
                    UInt numTeams = 0;
                    UInt numRegisteredTeams = 0;
                    String strTeams;
                    Vector<UShort> matchdaysFirstSeason;
                    Vector<UShort> matchdaysSecondSeason;
                    Vector<Vector<Pair<UChar, UChar>>> fixtures;
                    Vector<Array<UInt, 4>> bonuses;
                    if (comp->GetDbType() == FifamCompDbType::League) {
                        auto league = comp->AsLeague();
                        numTeams = league->mNumTeams;
                        strTeams = comp->GetProperty<String>(L"teams_override", String());
                        if (!strTeams.empty()) {
                            auto teamIDs = Utils::Split(strTeams, L',');
                            numRegisteredTeams = Utils::Min(numTeams, teamIDs.size());
                        }
                        else
                            numRegisteredTeams = 0;
                        matchdaysFirstSeason = league->mFirstSeasonMatchdays;
                        matchdaysSecondSeason = league->mSecondSeasonMatchdays;
                        fixtures = league->mFixtures;
                        bonuses = { league->mBonuses };
                    }
                    else if (comp->GetDbType() == FifamCompDbType::Round) {
                        auto round = comp->AsRound();
                        numTeams = round->mNumTeams;
                        matchdaysFirstSeason = { round->mFirstSeasonMatchdays[0], round->mFirstSeasonMatchdays[1] };
                        matchdaysSecondSeason = { round->mSecondSeasonMatchdays[0], round->mSecondSeasonMatchdays[1] };
                        bonuses = { round->mBonuses };
                    }
                    else if (comp->GetDbType() == FifamCompDbType::Pool) {
                        auto pool = comp->AsPool();
                        numTeams = pool->mNumTeams;
                        bonuses = { pool->mBonuses };
                    }
                    else if (comp->GetDbType() == FifamCompDbType::Cup) {
                        auto cup = comp->AsCup();
                        numTeams = cup->mNumTeams;
                        matchdaysFirstSeason = cup->mFirstSeasonMatchdays;
                        matchdaysSecondSeason = cup->mSecondSeasonMatchdays;
                        bonuses.resize(cup->mRounds.size());
                        for (UInt i = 0; i < cup->mRounds.size(); i++)
                            bonuses[i] = cup->mRounds[i].mBonuses;
                    }
                    writerFixture.WriteLine(numTeams);
                    writerFixture.WriteLine(numRegisteredTeams);
                    writerFixture.WriteStartIndex(L"TEAMS");
                    if (numRegisteredTeams > 0)
                        writerFixture.WriteLine(strTeams);
                    writerFixture.WriteEndIndex(L"TEAMS");
                    if (comp->GetDbType() == FifamCompDbType::League || comp->GetDbType() == FifamCompDbType::Cup || comp->GetDbType() == FifamCompDbType::Round) {
                        writerFixture.WriteStartIndex(L"MATCHDAYS");
                        Utils::Remove(matchdaysFirstSeason, 0);
                        writerFixture.WriteLineArray(matchdaysFirstSeason);
                        Utils::Remove(matchdaysSecondSeason, 0);
                        writerFixture.WriteLineArray(matchdaysSecondSeason);
                        writerFixture.WriteEndIndex(L"MATCHDAYS");
                    }
                    if (comp->GetDbType() == FifamCompDbType::League) {
                        writerFixture.WriteStartIndex(L"FIXTURES");
                        if (fixtures.size() > 0) {
                            writerFixture.WriteLine(fixtures.size(), fixtures[0].size());
                            for (UInt i = 0; i < fixtures.size(); i++) {
                                for (UInt f = 0; f < fixtures[i].size(); f++) {
                                    if (f != 0)
                                        writerFixture.Write(L",");
                                    writerFixture.Write(fixtures[i][f].first, fixtures[i][f].second);
                                }
                                writerFixture.WriteNewLine();
                            }
                        }
                        else {
                            writerFixture.WriteLine(0, 0);
                        }
                        writerFixture.WriteEndIndex(L"FIXTURES");
                    }
                    writerFixture.WriteStartIndex(L"BONUS");
                    for (UInt i = 0; i < 4; i++) {
                        Vector<UInt> roundBonuses(bonuses.size());
                        for (UInt b = 0; b < bonuses.size(); b++)
                            roundBonuses[b] = bonuses[b][i];
                        writerFixture.WriteLineArray(roundBonuses);
                    }
                    writerFixture.WriteEndIndex(L"BONUS");
                    writerFixture.WriteEndIndex(L"COMPETITION");
                }
                CompDesc compDesc;
                compDesc.id = comp->mID;
                compDesc.type = comp->GetDbType();
                if (comp->GetProperty<Bool>(L"has_name", false))
                    compDesc.name = FifamTr(comp->mName);
                compDesc.refName = comp->GetProperty<String>(L"ref", String());
                if (!compDesc.refName.empty())
                    compDesc.refName = L"@" + refName;
                compDesc.dirRefName = L"@@" + GetDirectReferenceFromCompID(comp->mID, gameId, ucpMode);
                compDesc.teams = comp->mNumTeams;
                compDesc.level = comp->mCompetitionLevel;
                outComps.push_back(compDesc);
                numCompsWritten++;
            }
        }
    }

    out = compsStr;
    outFixture = fixtureStr;
    outData = dataStr;

    for (auto &[from, to] : replacements) {
        Utils::Replace(out, from, to);
        Utils::Replace(outFixture, from, to);
        Utils::Replace(outData, from, to);
    }

    for (auto t : tokens)
        delete t;

    db.mCompMap.clear();

    for (auto comp : gComps) {
        Vector<Instruction> *instructions = comp->GetProperty<Vector<Instruction> *>(L"instructions", nullptr);
        delete instructions;
        Vector<FifamCompetition *> *predComps = comp->GetProperty<Vector<FifamCompetition *> *>(L"pred_comps", nullptr);
        delete predComps;
        delete comp;
    }

    return errorTitle.empty();
}

bool ScriptEngine::Decompile(const Path &filename, String &out, Int gameId, Bool universalMode) {
    out.clear();
    FifamReader reader(filename, gameId);
    if (!reader.Available())
        return false;
    FifamDatabase db;
    Vector<FifamCompetition *> comps;
    if (filename.extension() == L".sav") {
        
    }
    else {
        String compKeyName;
        if (filename.filename() == L"Continental - Europe.txt")
            compKeyName = L"EURO";
        else if (filename.filename() == L"Continental - South America.txt")
            compKeyName = L"SOUTHAM";
        else if (filename.filename() == L"Continental - Africa.txt")
            compKeyName = L"INTAFRICA";
        else if (filename.filename() == L"Continental - Asia.txt")
            compKeyName = L"INTASIA";
        else if (filename.filename() == L"Continental - North America.txt")
            compKeyName = L"INTAMERICA";
        else if (filename.filename() == L"Continental - Oceania.txt")
            compKeyName = L"INTOCEANIA";
        else if (filename.filename() == L"WorldCupQualification.txt")
            compKeyName = L"QUALI_WC";
        else if (filename.filename() == L"WorldCup.txt")
            compKeyName = L"WORLD_CUP";
        else if (filename.filename() == L"EuropeanChampionshipQualification.txt")
            compKeyName = L"QUALI_EC";
        else if (filename.filename() == L"EuropeanChampionship.txt")
            compKeyName = L"EURO_CUP";
        else if (filename.filename() == L"WorldCupU20.txt")
            compKeyName = L"U20_WORLD_CUP";
        else if (filename.filename() == L"ConfedCup.txt")
            compKeyName = L"CONFED_CUP";
        else if (filename.filename() == L"CopaAmerica.txt")
            compKeyName = L"COPA_AMERICA";
        else if (filename.filename() == L"EuropeanCup.txt")
            compKeyName = L"EURO";
        else if (filename.filename() == L"SouthAmCup")
            compKeyName = L"SOUTHAM";
        else if (filename.filename() == L"IntAfrica")
            compKeyName = L"INTAFRICA";
        else if (filename.filename() == L"IntAsia")
            compKeyName = L"INTASIA";
        else if (filename.filename() == L"IntAmerica")
            compKeyName = L"INTAMERICA";
        else if (filename.filename() == L"IntOceania")
            compKeyName = L"INTOCEANIA";
        else if (filename.filename() == L"QualiWC.txt")
            compKeyName = L"QUALI_WC";
        else if (filename.filename() == L"WC.txt")
            compKeyName = L"WORLD_CUP";
        else if (filename.filename() == L"QualiEC.txt")
            compKeyName = L"QUALI_EC";
        else if (filename.filename() == L"EC.txt")
            compKeyName = L"EURO_CUP";
        if (compKeyName.empty())
            return false;
        auto numComps = reader.ReadLine<UInt>();
        for (UInt i = 0; i < numComps; i++) {
            if (reader.ReadStartIndex(Utils::Format(L"%s%d", compKeyName.c_str(), i))) {
                auto comp = db.ReadCompetition(reader, FifamNation::None);
                if (comp)
                    comps.push_back(comp);
                else
                    return false;
                reader.ReadEndIndex(Utils::Format(L"%s%d", compKeyName.c_str(), i));
            }
        }
    }
    UInt numPools = 0, numRounds = 0, numLeagues = 0, numCups = 0;
    for (auto comp : comps) {
        if (comp->GetDbType() == FifamCompDbType::Pool)
            comp->SetProperty(L"ref", Utils::Format(L"pool_%u", ++numPools));
        else if (comp->GetDbType() == FifamCompDbType::Round)
            comp->SetProperty(L"ref", Utils::Format(L"round_%u", ++numRounds));
        else if (comp->GetDbType() == FifamCompDbType::League)
            comp->SetProperty(L"ref", Utils::Format(L"league_%u", ++numLeagues));
        else if (comp->GetDbType() == FifamCompDbType::Cup)
            comp->SetProperty(L"ref", Utils::Format(L"cup_%u", ++numLeagues));
        bool hasRef = false;
        for (auto refcomp : comps) {
            if (refcomp != comp) {
                for (auto const &ref : refcomp->mPredecessors) {
                    if (ref == comp->mID) {
                        hasRef = true;
                        break;
                    }
                }
                if (!hasRef) {
                    for (auto const &ref : refcomp->mSuccessors) {
                        if (ref == comp->mID) {
                            hasRef = true;
                            break;
                        }
                    }
                    if (!hasRef) {
                        if (refcomp->GetDbType() == FifamCompDbType::Pool) {
                            for (auto const &ref : refcomp->AsPool()->mCompConstraints) {
                                if (ref == comp->mID) {
                                    hasRef = true;
                                    break;
                                }
                            }
                        }
                        if (!hasRef) {
                            refcomp->mInstructions.ForAllCompetitionLinks([&](FifamCompID &ref, UInt, FifamAbstractInstruction *) {
                                if (!hasRef && ref == comp->mID)
                                    hasRef = true;
                            });
                        }
                    }
                }
            }
            if (hasRef)
                break;
        }
        comp->SetProperty(L"has_ref", hasRef);
    }

    for (auto comp : comps) {
        if (comp->GetDbType() == FifamCompDbType::Pool)
            out += L"pool";
        else if (comp->GetDbType() == FifamCompDbType::Round)
            out += L"round";
        else if (comp->GetDbType() == FifamCompDbType::League)
            out += L"league";
        else if (comp->GetDbType() == FifamCompDbType::Cup)
            out += L"cup";
        else
            continue;
        if (comp->GetProperty<Bool>(L"has_ref", false))
            out += L" @" + comp->GetProperty<String>(L"ref");
        if (!FifamTr(comp->mName).empty() && FifamTr(comp->mName) != L"Competition")
            out += L" \"" + FifamTr(comp->mName) + L"\"";
        if (comp->mCompetitionLevel > 0)
            out += L" level " + Utils::Format(L"%d", comp->mCompetitionLevel - 1);
        out += L" teams " + Utils::Format(L"%d", comp->mNumTeams);
        out += L" {\n";
        out += L"}\n\n";
    }
    return true;
}
