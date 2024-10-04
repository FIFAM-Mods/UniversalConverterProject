#pragma once
#include <map>
#include <string>
#include <vector>
#include <filesystem>

class SettingsHelper {
public:
    template<typename SrcT>
    static SrcT ToLower(SrcT const &str) {
        SrcT result;
        for (size_t i = 0; i < str.size(); i++)
            result += tolower(static_cast<SrcT::value_type>(str[i]));
        return result;
    }

    template<typename SrcT>
    static SrcT ToUpper(SrcT const &str) {
        SrcT result;
        for (size_t i = 0; i < str.length(); i++)
            result += toupper(static_cast<SrcT::value_type>(str[i]));
        return result;
    }

    template<typename SrcT>
    static void Trim(SrcT &str) {
        size_t start = str.find_first_not_of(' ');
        if (start != SrcT::npos)
            str = str.substr(start);
        size_t end = str.find_last_not_of(' ');
        if (end != SrcT::npos)
            str = str.substr(0, end + 1);
    }

    template<typename SrcT>
    static std::vector<SrcT> Split(SrcT const &line, typename SrcT::value_type delim, bool trim = true, bool skipEmpty = false) {
        std::vector<SrcT> result;
        SrcT currStr;
        auto AddStr = [&, trim, skipEmpty]() {
            if (trim)
                Trim(currStr);
            if (!skipEmpty || !currStr.empty())
                result.push_back(currStr);
            currStr.clear();
        };
        for (size_t i = 0; i < line.length(); i++) {
            auto c = line[i];
            if (c == '\r' || (delim != '\n' && c == '\n'))
                break;
            if (c == delim)
                AddStr();
            else
                currStr += c;
        }
        AddStr();
        return result;
    }

    template<typename DstT, typename SrcT>
    static DstT StrToInt(SrcT const &str, bool isHex = false) {
        DstT result = 0;
        try {
            result = static_cast<DstT>(std::stoull(str, nullptr, isHex ? 16 : 10));
        }
        catch (...) {}
        return result;
    }

    template<typename SrcT>
    static float StrToFloat(SrcT const &str) {
        float result = 0.0f;
        try {
            result = std::stof(str);
        }
        catch (...) {}
        return result;
    }

    template<typename SrcT>
    static double StrToDouble(SrcT const &str) {
        double result = 0.0;
        try {
            result = std::stod(str);
        }
        catch (...) {}
        return result;
    }

    template<typename DstT, typename SrcT>
    static bool StrToInt(SrcT const &str, DstT &dst, bool isHex) {
        bool result = false;
        try {
            dst = static_cast<DstT>(std::stoull(str, nullptr, isHex ? 16 : 10));
            result = true;
        }
        catch (...) {}
        return result;
    }

    template<typename SrcT>
    static float StrToFloat(SrcT const &str, float &dst) {
        bool result = false;
        try {
            dst = std::stof(str);
            result = true;
        }
        catch (...) {}
        return result;
    }

    template<typename SrcT>
    static double StrToDouble(SrcT const &str, double &dst) {
        bool result = false;
        try {
            dst = std::stod(str);
            result = true;
        }
        catch (...) {}
        return result;
    }

    class FormatterUtilsUnicode {
    public:
        template<typename T> static T const &Arg(T const &arg) { return arg; }
        static wchar_t const *Arg(std::wstring const &arg) { return arg.c_str(); }
    };

    class FormatterUtilsAscii {
    public:
        template<typename T> static T const &Arg(T const &arg) { return arg; }
        static char const *Arg(std::string const &arg) { return arg.c_str(); }
    };

    template<typename ...ArgTypes>
    static std::wstring Format(const std::wstring &format, ArgTypes... args) {
        static wchar_t buf[4096];
        swprintf(buf, 4096, format.c_str(), FormatterUtilsUnicode::Arg(args)...);
        return buf;
    }

    template<typename ...ArgTypes>
    static std::string Format(const std::string &format, ArgTypes... args) {
        static char buf[4096];
        snprintf(buf, 4096, format.c_str(), FormatterUtilsAscii::Arg(args)...);
        return buf;
    }

    static std::string ToUTF8(std::wstring const &wstr);
    static std::wstring ToUTF16(std::string const &str);
};

template<typename T, typename = void> struct SettingsParameterCallbacks;

template<typename T> struct SettingsParameterDefaultAssignCompare {
    static void Assign(T &dst, T const &src) {
        dst = src;
    }
    static bool Compare(T const &a, T const &b) {
        return a == b;
    }
};

template<typename T> struct SettingsParameterCallbacks<T, std::enable_if_t<std::is_integral_v<T>>> : public SettingsParameterDefaultAssignCompare<T> {
    static void FromString(T &dst, std::wstring const &src) {
        bool hexadecimal = src.size() >= 2 && src[0] == L'0' && (src[1] == L'x' || src[1] == L'X');
        if (!hexadecimal || src.size() >= 3) {
            try { dst = static_cast<T>(std::stoull(src, 0, hexadecimal ? 16 : 10)); }
            catch (...) {}
        }
    }
    static void ToString(T const &src, std::wstring &dst) {
        dst = std::to_wstring(src);
    }
};

template<> struct SettingsParameterCallbacks<float> : public SettingsParameterDefaultAssignCompare<float> {
    static void FromString(float &dst, std::wstring const &src) {
        try { dst = std::stof(src); }
        catch (...) {}
    }
    static void ToString(float const &src, std::wstring &dst) {
        dst = std::to_wstring(src);
    }
};

template<> struct SettingsParameterCallbacks<double> : public SettingsParameterDefaultAssignCompare<double> {
    static void FromString(double &dst, std::wstring const &src) {
        try { dst = std::stod(src); }
        catch (...) {}
    }
    static void ToString(double const &src, std::wstring &dst) {
        dst = std::to_wstring(src);
    }
};

template<> struct SettingsParameterCallbacks<long double> : public SettingsParameterDefaultAssignCompare<long double> {
    static void FromString(long double &dst, std::wstring const &src) {
        try { dst = std::stold(src); }
        catch (...) {}
    }
    static void ToString(long double const &src, std::wstring &dst) {
        dst = std::to_wstring(src);
    }
};

template<> struct SettingsParameterCallbacks<bool> : public SettingsParameterDefaultAssignCompare<bool> {
    static void FromString(bool &dst, std::wstring const &src) {
        std::wstring l = SettingsHelper::ToLower(src);
        if (l == L"1" || l == L"true" || l == L"yes" || l == L"on")
            dst = true;
        else if (l == L"0" || l == L"false" || l == L"no" || l == L"off")
            dst = false;
    }
    static void ToString(bool const &src, std::wstring &dst) {
        dst = src ? L"TRUE" : L"FALSE";
    }
};

template<> struct SettingsParameterCallbacks<std::string> : public SettingsParameterDefaultAssignCompare<std::string> {
    static void FromString(std::string &dst, std::wstring const &src) {
        dst = SettingsHelper::ToUTF8(src);
    }
    static void ToString(std::string const &src, std::wstring &dst) {
        dst = SettingsHelper::ToUTF16(src);
    }
};

template<> struct SettingsParameterCallbacks<std::wstring> : public SettingsParameterDefaultAssignCompare<std::wstring> {
    static void FromString(std::wstring &dst, std::wstring const &src) {
        dst = src;
    }
    static void ToString(std::wstring const &src, std::wstring &dst) {
        dst = src;
    }
};

template<> struct SettingsParameterCallbacks<std::filesystem::path> : public SettingsParameterDefaultAssignCompare<std::filesystem::path> {
    static void FromString(std::filesystem::path &dst, std::wstring const &src) {
        dst = src;
    }
    static void ToString(std::filesystem::path const &src, std::wstring &dst) {
        dst = src.c_str();
    }
};

class SettingsAbstract abstract {
protected:
    class SettingsParameter {
    public:
        std::string mName;
        unsigned int mOffset = 0;
        unsigned int mIndex = 0;
        void(*mAssign)(void *, void const *) = nullptr;
        bool(*mCompare)(void const *, void const *) = nullptr;
        void(*mFromString)(void *, std::wstring const &) = nullptr;
        void(*mToString)(void const *, std::wstring &) = nullptr;
    };

    std::vector<unsigned char> mDefaultValues;
    std::map<std::string, SettingsParameter> mParametersMap;

    template<typename T>
    T SettingsInitializer(SettingsAbstract *base, void *member, char const *name, T defaultVal, void(*customRead)(void *, std::wstring const &) = nullptr, void(*customWrite)(void const *, std::wstring &) = nullptr) {
        SettingsParameter parameter;
        parameter.mName = name;
        parameter.mOffset = (unsigned int)member - (unsigned int)base;
        parameter.mIndex = base->mParametersMap.size();
        parameter.mAssign = (void(*)(void *, void const *)) & SettingsParameterCallbacks<T>::Assign;
        parameter.mCompare = (bool(*)(void const *, void const *)) & SettingsParameterCallbacks<T>::Compare;
        if (customRead)
            parameter.mFromString = customRead;
        else
            parameter.mFromString = (void(*)(void *, std::wstring const &)) & SettingsParameterCallbacks<T>::FromString;
        if (customWrite)
            parameter.mToString = customWrite;
        else
            parameter.mToString = (void(*)(void const *, std::wstring &)) & SettingsParameterCallbacks<T>::ToString;
        base->mParametersMap[SettingsHelper::ToLower(std::string(name))] = parameter;
        unsigned int newSize = parameter.mOffset + sizeof(T);
        if (base->mDefaultValues.size() < newSize)
            base->mDefaultValues.resize(newSize);
        new (base->mDefaultValues.data() + parameter.mOffset) T();
        SettingsParameterCallbacks<T>::Assign(*(T *)(base->mDefaultValues.data() + parameter.mOffset), *(T const *)&defaultVal);
        return defaultVal;
    }
public:
    void Read(std::filesystem::path const &filename);
    void Write(std::filesystem::path const &filename, bool onlyNonDefault = false);
    void Reset();
    SettingsAbstract();
    SettingsAbstract(std::filesystem::path const &filename);
};

#define SETTINGS_PARAMETER(type, variable, name, defaultValue) type variable { SettingsInitializer<type>(this, &variable, name, defaultValue) }
#define SETTINGS_PARAMETER_CUSTOM_READ_WRITE(type, variable, name, defaultValue, customRead, customWrite) type variable { SettingsInitializer<type>(this, &variable, name, defaultValue, (void(*)(void *, std::wstring const &))customRead, (void(*)(void const *, std::wstring &))customWrite) }
