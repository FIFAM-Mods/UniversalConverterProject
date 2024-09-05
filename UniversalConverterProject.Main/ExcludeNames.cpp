#include "ExcludeNames.h"
#include "FifamTypes.h"
#include "Log.h"

using namespace plugin;

//#define LOG_NAMES

enum eNameType {
    NAME_FIRST_EXCLUDE = 0,
    NAME_FIRST = 1,
    NAME_COMMON = 2,
    NAME_USER_NICK = 3,
    NAME_LAST = 4
};

using NamesArray = Set<String>;

NamesArray &GetExcludeNames() {
    static NamesArray excludeNames;
    return excludeNames;
}

NamesArray &GetExcludeSurnames() {
    static NamesArray excludeSurnames;
    return excludeSurnames;
}

NamesArray &GetExcludeCommonNames() {
    static NamesArray excludeCommonNames;
    return excludeCommonNames;
}

String GetNameByIndex(UInt nameType, UChar languageId, UShort index) {
    return CallMethodAndReturn<WideChar const *, 0x1499EC3>(CallAndReturn<void *, 0x1499D1D>(), languageId, nameType, index);
}

Bool IsExcluded(UShort nameIndex, UChar languageId, UInt nameType) {
    if (nameType == NAME_FIRST)
        return Utils::Contains(GetExcludeNames(), GetNameByIndex(nameType, languageId, nameIndex));
    else if (nameType == NAME_LAST)
        return Utils::Contains(GetExcludeSurnames(), GetNameByIndex(nameType, languageId, nameIndex));
    else if (nameType == NAME_COMMON)
        return Utils::Contains(GetExcludeCommonNames(), GetNameByIndex(nameType, languageId, nameIndex));
    return false;
}

UInt ReadNamesFile(String const &filename, NamesArray &names) {
    UInt numNames = names.size();
	FifamReader reader(filename, 14, FifamVersion());
	if (reader.Available()) {
		while (!reader.IsEof()) {
			String name;
			reader.ReadLine(name);
			Utils::Trim(name);
			if (!name.empty())
			    names.insert(name);
        }
    }
    return names.size() - numNames;
}

UShort METHOD GetRandomNameIndex(UInt namesPool) {
    return CallMethodAndReturn<UShort, 0x149970B>(namesPool);
}

UShort GetRandomNonExcludedNameIndex(UInt pools, UChar languageId, UInt nameType) {
    UShort index = GetRandomNameIndex(pools + 180 * languageId + nameType * 36);
    UInt counter = 0;
#ifdef LOG_NAMES
    Bool infoWritten = false;
#endif
    while (index != 0 && IsExcluded(index, languageId, nameType)) {
    #ifdef LOG_NAMES
        if (!infoWritten) {
            Log::Write(Utils::Format(L"GetRandomNonExcludedNameIndex (%d,%d):", languageId, nameType));
            infoWritten = true;
        }
        Log::Write(Utils::Format(L" %d: %s (%d);", counter + 1, GetNameByIndex(nameType, languageId, index), index));
    #endif
        index = GetRandomNameIndex(pools + 180 * languageId + nameType * 36);
        if (++counter >= 10) {
        #ifdef LOG_NAMES
            Log::Write(L"\n");
        #endif
            return index;
        }
    }
#ifdef LOG_NAMES
    if (infoWritten)
        Log::WriteLine(Utils::Format(L" Final: %s Attempts: %d", GetNameByIndex(nameType, languageId, index), counter));
#endif
    return index;
}

UShort METHOD GetOneRandomNameIndex(UInt pools, DUMMY_ARG, UChar languageId, UInt nameType) {
    if (languageId < 128)
        return GetRandomNonExcludedNameIndex(pools, languageId, nameType);
    return 0;
}

void * METHOD GetRandomFirstLast(UInt pools, DUMMY_ARG, void *desc, UChar languageId, Bool bMale) {
    if (languageId < 128) {
        UShort firstNameIndex = GetRandomNonExcludedNameIndex(pools, languageId, bMale ? 1 : 0);
        UShort lastNameIndex = GetRandomNonExcludedNameIndex(pools, languageId, 4);
        CallMethod<0x14991DD>(desc, languageId, firstNameIndex, lastNameIndex, bMale);
    }
    else
        CallMethod<0x149917F>(desc);
    return desc;
}

void PatchExcludeNames(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        //ReadNamesFile(L"database\\ExcludeNames.txt", GetExcludeNames());
        //ReadNamesFile(L"database\\ExcludeSurnames.txt", GetExcludeSurnames());
        //ReadNamesFile(L"database\\ExcludeCommonNames.txt", GetExcludeCommonNames());
        //patch::RedirectJump(0x1499E11, GetOneRandomNameIndex);
        //patch::RedirectJump(0x1499E45, GetRandomFirstLast);
        UInt numNames = ReadNamesFile(L"fmdata\\UCP_FemaleNames.txt", GetExcludeNames());
        numNames += ReadNamesFile(L"fmdata\\UCP_FemaleSurnames.txt", GetExcludeSurnames());
        numNames += ReadNamesFile(L"fmdata\\UCP_FemaleCommonNames.txt", GetExcludeCommonNames());
        if (numNames > 0) {
            patch::RedirectJump(0x1499E11, GetOneRandomNameIndex);
            patch::RedirectJump(0x1499E45, GetRandomFirstLast);
        }
    }
}
