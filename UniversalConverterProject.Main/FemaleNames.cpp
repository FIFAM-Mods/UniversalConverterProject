#include "FemaleNames.h"
#include "FifamTypes.h"
#include "Log.h"
#include "license_check/license_check.h"

using namespace plugin;

//#define LOG_NAMES

enum eNameType {
    NAME_FIRST_FEMALE = 0,
    NAME_FIRST = 1,
    NAME_COMMON = 2,
    NAME_USER_NICK = 3,
    NAME_LAST = 4
};

const UInt MAX_LANGUAGES = 128;
using NamesArray = Array<Set<UShort>, MAX_LANGUAGES>;

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

Bool IsFemaleName(UChar languageId, UShort nameIndex) {
    if (languageId < MAX_LANGUAGES)
        return false;
    auto &names = GetFemaleNames()[languageId];
    return names.find(nameIndex) != names.end();
}

Bool IsFemaleSurname(UChar languageId, UShort nameIndex) {
    if (languageId < MAX_LANGUAGES)
        return false;
    auto &names = GetFemaleSurnames()[languageId];
    return names.find(nameIndex) != names.end();
}

Bool IsFemaleCommonName(UChar languageId, UShort nameIndex) {
    if (languageId < MAX_LANGUAGES)
        return false;
    auto &names = GetFemaleCommonNames()[languageId];
    return names.find(nameIndex) != names.end();
}

Bool IsFemale(UShort nameIndex, UChar languageId, UInt nameType) {
    if (nameType == NAME_FIRST)
        return IsFemaleName(languageId, nameIndex);
    else if (nameType == NAME_LAST)
        return IsFemaleSurname(languageId, nameIndex);
    else if (nameType == NAME_COMMON)
        return IsFemaleCommonName(languageId, nameIndex);
    return false;
}

UInt ReadNamesFile(String const &filename, NamesArray &names) {
    UInt numNamesRead = 0;
    FILE *f = _wfopen(filename.c_str(), Magic<'r','b'>(3009798726).c_str());
    if (f) {
        for (UInt l = 0; l < MAX_LANGUAGES; l++) {
            UInt numNames = 0;
            if (fread(&numNames, 4, 1, f) == 1 && numNames > 0) {
                for (UInt i = 0; i < numNames; i++) {
                    UShort nameIndex = 0;
                    if (fread(&nameIndex, 2, 1, f) != 1)
                        break;
                    names[l].insert(nameIndex);
                    numNamesRead++;
                }
            }
        }
        fclose(f);
    }
    return numNamesRead;
}

UShort METHOD GetRandomNameIndex(UInt namesPool) {
    return CallMethodAndReturn<UShort, 0x149970B>(namesPool);
}

WideChar const *GetNameByIndex(UInt nameType, UChar languageId, UShort index) {
    return CallMethodAndReturn<WideChar const *, 0x1499EC3>(CallAndReturn<void *, 0x1499D1D>(), languageId, nameType, index);
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
        UInt numNames = ReadNamesFile(Magic<'f','m','d','a','t','a','\\','U','C','P','_','F','e','m','a','l','e','N','a','m','e','s','.','b','i','n'>(4134089135), GetFemaleNames());
        numNames += ReadNamesFile(Magic<'f','m','d','a','t','a','\\','U','C','P','_','F','e','m','a','l','e','S','u','r','n','a','m','e','s','.','b','i','n'>(482911666), GetFemaleSurnames());
        numNames += ReadNamesFile(Magic<'f','m','d','a','t','a','\\','U','C','P','_','F','e','m','a','l','e','C','o','m','m','o','n','N','a','m','e','s','.','b','i','n'>(1755295342), GetFemaleCommonNames());
        if (numNames > 0) {
            patch::RedirectJump(0x1499E11, GetOneRandomNameIndex);
            patch::RedirectJump(0x1499E45, GetRandomFirstLast);
        }
    }
}
