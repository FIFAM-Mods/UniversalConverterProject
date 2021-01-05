#include "FemaleNames.h"
#include "FifamTypes.h"
#include "Log.h"
#include "license_check/license_check.h"

using namespace plugin;

#define LOG_NAMES

enum eNameType {
    NAME_FIRST_FEMALE = 0,
    NAME_FIRST = 1,
    NAME_COMMON = 2,
    NAME_USER_NICK = 3,
    NAME_LAST = 4
};

using NamesArray = Set<String>;

NamesArray &GetFemaleNames() {
    static NamesArray femaleNames;
    return femaleNames;
}

NamesArray &GetFemaleSurnames() {
    static NamesArray femaleSurnames;
    return femaleSurnames;
}

NamesArray &GetFemaleCommonNames() {
    static NamesArray femaleCommonNames;
    return femaleCommonNames;
}

WideChar const *GetNameByIndex(UInt nameType, UChar languageId, UShort index) {
    return CallMethodAndReturn<WideChar const *, 0x1499EC3>(CallAndReturn<void *, 0x1499D1D>(), languageId, nameType, index);
}

Bool IsFemale(UShort nameIndex, UChar languageId, UInt nameType) {
    if (nameType == NAME_FIRST)
        return GetFemaleNames().contains(GetNameByIndex(nameType, languageId, nameIndex));
    else if (nameType == NAME_LAST)
        return GetFemaleSurnames().contains(GetNameByIndex(nameType, languageId, nameIndex));
    else if (nameType == NAME_COMMON)
        return GetFemaleCommonNames().contains(GetNameByIndex(nameType, languageId, nameIndex));
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

UShort GetRandomNonFemaleNameIndex(UInt pools, UChar languageId, UInt nameType) {
    UShort index = GetRandomNameIndex(pools + 180 * languageId + nameType * 36);
    UInt counter = 0;
#ifdef LOG_NAMES
    Bool infoWritten = false;
#endif
    while (index != 0 && IsFemale(index, languageId, nameType)) {
    #ifdef LOG_NAMES
        if (!infoWritten) {
            Log::Write(Utils::Format(L"GetRandomNonFemaleNameIndex (%d,%d):", languageId, nameType));
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
        return GetRandomNonFemaleNameIndex(pools, languageId, nameType);
    return 0;
}

void * METHOD GetRandomFirstLast(UInt pools, DUMMY_ARG, void *desc, UChar languageId, Bool bMale) {
    if (languageId < 128) {
        UShort firstNameIndex = GetRandomNonFemaleNameIndex(pools, languageId, bMale ? 1 : 0);
        UShort lastNameIndex = GetRandomNonFemaleNameIndex(pools, languageId, 4);
        CallMethod<0x14991DD>(desc, languageId, firstNameIndex, lastNameIndex, bMale);
    }
    else
        CallMethod<0x149917F>(desc);
    return desc;
}

void PatchFemaleNames(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        UInt numNames = ReadNamesFile(L"fmdata\\UCP_FemaleNames.txt", GetFemaleNames());
        numNames += ReadNamesFile(L"fmdata\\UCP_FemaleSurnames.txt", GetFemaleSurnames());
        numNames += ReadNamesFile(L"fmdata\\UCP_FemaleCommonNames.txt", GetFemaleCommonNames());
        if (numNames > 0) {
            patch::RedirectJump(0x1499E11, GetOneRandomNameIndex);
            patch::RedirectJump(0x1499E45, GetRandomFirstLast);
        }
    }
}
