#include "FemaleNames.h"
#include "FifamTypes.h"
#include "Utils.h"
#include "FifamReadWrite.h"

using namespace plugin;

enum eNameType {
    NAME_FIRST_FEMALE = 0,
    NAME_FIRST = 1,
    NAME_COMMON = 2,
    NAME_USER_NICK = 3,
    NAME_LAST = 4
};

enum eNameFlag {
    USED_BY_MALE = 1,
    USED_BY_FEMALE = 2
};

const UInt MAX_LANGUAGES = 128;
using NamesArray = Array<Map<UShort, UChar>, MAX_LANGUAGES>;
using DebugNamesArray = Array<Map<UShort, String>, MAX_LANGUAGES>;

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

DebugNamesArray &GetDebugFemaleNames() {
    static DebugNamesArray debugFemaleNames;
    return debugFemaleNames;
}

DebugNamesArray &GetDebugFemaleSurnames() {
    static DebugNamesArray debugFemaleSurnames;
    return debugFemaleSurnames;
}

DebugNamesArray &GetDebugFemaleCommonNames() {
    static DebugNamesArray debugFemaleCommonNames;
    return debugFemaleCommonNames;
}

Set<String> &GetDatabaseFemaleStaff() {
    static Set<String> dbFemaleStaff;
    return dbFemaleStaff;
}

Set<String> &GetDatabaseFemaleReferees() {
    static Set<String> dbFemaleReferess;
    return dbFemaleReferess;
}

String MakeUniqueId(String const &firstName, String const &lastName, String const &pseudonym, Date const &birthdate) {
    return firstName + lastName + pseudonym + birthdate.ToString();
}

Bool IsFemaleStaff(String const &firstName, String const &lastName, String const &pseudonym, Date const &birthdate) {
    return GetDatabaseFemaleStaff().find(MakeUniqueId(firstName, lastName, pseudonym, birthdate)) != GetDatabaseFemaleStaff().end();
}

Bool IsFemaleReferee(String const &firstName, String const &lastName) {
    return GetDatabaseFemaleReferees().find(MakeUniqueId(firstName, lastName, String(), Date())) != GetDatabaseFemaleReferees().end();
}

void ReadDatabaseEntities(Path const &filename, Set<String> &entries) {
    FifamReader reader(filename, 13, 0, 0);
    if (reader.Available()) {
        reader.SkipLine();
        while (!reader.IsEof()) {
            if (!reader.EmptyLine()) {
                String firstName, lastName, pseudonym;
                Date birthdate;
                reader.ReadLine(firstName, lastName, pseudonym, birthdate);
                entries.insert(MakeUniqueId(firstName, lastName, pseudonym, birthdate));
            }
            else
                reader.SkipLine();
        }
    }
    else
        Error(Utils::Format(L"failed to open '%s'", filename));
}

void WriteNamesFile(WideChar const *filename, NamesArray &names) {
    FILE *f = _wfopen(filename, L"wb");
    if (f) {
        for (UInt l = 0; l < MAX_LANGUAGES; l++) {
            UInt numNames = 0;
            for (auto[nameIndex, flags] : names[l]) {
                if (flags == USED_BY_FEMALE)
                    numNames++;
            }
            if (fwrite(&numNames, 4, 1, f) != 1) {
                Error(Utils::Format(L"failed to write to '%s'", filename));
                break;
            }
            if (numNames > 0) {
                for (auto[nameIndex, flags] : names[l]) {
                    if (flags == USED_BY_FEMALE) {
                        if (fwrite(&nameIndex, 2, 1, f) != 1) {
                            Error(Utils::Format(L"failed to write to '%s'", filename));
                            break;
                        }
                    }
                }
            }
        }
        fclose(f);
    }
    else
        Error(Utils::Format(L"failed to open '%s'", filename));
}

void WriteDebugNamesFile(WideChar const *filename, NamesArray &names, DebugNamesArray &debugNames) {
    FifamWriter writer(filename, 14, FifamVersion());
    if (writer.Available()) {
        for (UInt l = 0; l < MAX_LANGUAGES; l++) {
            UInt numNames = 0;
            for (auto[nameIndex, flags] : names[l]) {
                if (flags == USED_BY_FEMALE)
                    numNames++;
            }
            writer.WriteLine(Utils::Format(L"%d - %d names", l, numNames));
            if (numNames > 0) {
                for (auto[nameIndex, flags] : names[l]) {
                    if (flags == USED_BY_FEMALE)
                        writer.WriteLine(debugNames[l][nameIndex]);
                    else if (flags == (USED_BY_MALE | USED_BY_FEMALE))
                        writer.WriteLine(L"; " + debugNames[l][nameIndex] + L" - used also by male");
                }
            }
        }
    }
    else
        Error(Utils::Format(L"failed to open '%s'", filename));
}

void WriteNames() {
    WriteNamesFile(L"fmdata\\UCP_FemaleNames.bin", GetFemaleNames());
    WriteNamesFile(L"fmdata\\UCP_FemaleSurnames.bin", GetFemaleSurnames());
    WriteNamesFile(L"fmdata\\UCP_FemaleCommonNames.bin", GetFemaleCommonNames());
}

void WriteDebugNames() {
    WriteDebugNamesFile(L"fmdata\\UCP_FemaleNames.txt", GetFemaleNames(), GetDebugFemaleNames());
    WriteDebugNamesFile(L"fmdata\\UCP_FemaleSurnames.txt", GetFemaleSurnames(), GetDebugFemaleSurnames());
    WriteDebugNamesFile(L"fmdata\\UCP_FemaleCommonNames.txt", GetFemaleCommonNames(), GetDebugFemaleCommonNames());
}

void AddName(NamesArray &names, WideChar const *name, UShort nameIndex, UChar languageId, UInt nameType, Bool isFemale) {
    if (languageId < 128) {
        names[languageId][nameIndex] |= isFemale ? USED_BY_FEMALE : USED_BY_MALE;
        if (nameType == NAME_FIRST)
            GetDebugFemaleNames()[languageId][nameIndex] = name;
        else if (nameType == NAME_LAST)
            GetDebugFemaleSurnames()[languageId][nameIndex] = name;
        else if (nameType == NAME_COMMON)
            GetDebugFemaleCommonNames()[languageId][nameIndex] = name;
    }
    else
        Error(Utils::Format(L"failed to add name for language '%d'", languageId));
}

UInt gOriginalDbWriteFunc = 0;

Bool32 METHOD OnWriteDb(void *db, DUMMY_ARG, Int a, Int b) {
    Bool32 result = CallMethodAndReturnDynGlobal<Bool32>(gOriginalDbWriteFunc, db, a, b);
    WriteNames();
    WriteDebugNames();
    return result;
}

template<UInt addr1, UInt addr2, Bool playerHook>
class EntityWriterHook {
    inline static UInt gOriginalWriteEmployee[2] = {};
    inline static void *gEntity = nullptr;
    using Callback = Function<void(void *, UShort, UChar, UInt, WideChar const *)>;
    inline static Callback gCallback;

    static void METHOD Hook1(void *entity, DUMMY_ARG, void *db) {
        gEntity = entity;
        CallMethodDynGlobal(gOriginalWriteEmployee[0], entity, db);
        gEntity = nullptr;
    }
    static void METHOD Hook2(void *entity, DUMMY_ARG, void *db) {
        gEntity = entity;
        CallMethodDynGlobal(gOriginalWriteEmployee[1], entity, db);
        gEntity = nullptr;
    }
    static void METHOD PlayerHook1(void *entity, DUMMY_ARG, void *db, Int clubId) {
        gEntity = entity;
        CallMethodDynGlobal(gOriginalWriteEmployee[0], entity, db, clubId);
        gEntity = nullptr;
    }
    static void METHOD PlayerHook2(void *entity, DUMMY_ARG, void *db, Int clubId) {
        gEntity = entity;
        CallMethodDynGlobal(gOriginalWriteEmployee[1], entity, db, clubId);
        gEntity = nullptr;
    }
    static UShort METHOD GetNameIndexHook(void *pools, DUMMY_ARG, UChar languageId, UInt nameType, WideChar const *name, void *db, Bool increaseRefCounter) {
        UShort index = CallMethodAndReturn<UShort, 0x51AD50>(pools, languageId, nameType, name, db, increaseRefCounter);
        if (gEntity && *raw_ptr<void *>(db, 8))
            gCallback(gEntity, index, languageId, nameType, name);
        return index;
    }
public:
    EntityWriterHook(UInt nameHookAddr, UInt surnameHookAddr, UInt commonNameHookAddr, Callback cb) {
        gCallback = cb;
        if constexpr (!playerHook) {
            if constexpr (addr1 > 0)
                gOriginalWriteEmployee[0] = patch::RedirectCall(addr1, Hook1);
            if constexpr (addr2 > 0)
                gOriginalWriteEmployee[1] = patch::RedirectCall(addr2, Hook2);
        }
        else {
            if constexpr (addr1 > 0)
                gOriginalWriteEmployee[0] = patch::RedirectCall(addr1, PlayerHook1);
            if constexpr (addr2 > 0)
                gOriginalWriteEmployee[1] = patch::RedirectCall(addr2, PlayerHook2);
        }
        if (nameHookAddr > 0)
            patch::RedirectCall(nameHookAddr, GetNameIndexHook);
        if (surnameHookAddr > 0)
            patch::RedirectCall(surnameHookAddr, GetNameIndexHook);
        if (commonNameHookAddr > 0)
            patch::RedirectCall(commonNameHookAddr, GetNameIndexHook);
    }
};

Bool METHOD OnlineDatabaseEnabled(void *) {
    return false;
}

void PatchFemaleNames(FM::Version v) {
    if (v.id() == ID_ED_13_1000) {

        patch::RedirectCall(0x4D9B89, OnlineDatabaseEnabled);

        ReadDatabaseEntities(L"database\\FemaleStaff.sav", GetDatabaseFemaleStaff());
        ReadDatabaseEntities(L"database\\FemaleReferee.sav", GetDatabaseFemaleReferees());

        gOriginalDbWriteFunc = patch::RedirectCall(0x5156BF, OnWriteDb);

        static EntityWriterHook<0x4C4F5F, 0x4E8CEF, false> employeeHook(0x51F5C3, 0x51F5DF, 0x51F5FB,
        [](void *entity, UShort index, UChar languageId, UInt nameType, WideChar const *name) {
            Bool isFemaleStaff = IsFemaleStaff(
                raw_ptr<WideChar const>(entity, 0x10),
                raw_ptr<WideChar const>(entity, 0x30),
                raw_ptr<WideChar const>(entity, 0x80),
                FifamDate(*raw_ptr<UInt>(entity, 0x174))
            );
            if (nameType == NAME_FIRST)
                AddName(GetFemaleNames(), name, index, languageId, nameType, isFemaleStaff);
            else if (nameType == NAME_LAST)
                AddName(GetFemaleSurnames(), name, index, languageId, nameType, isFemaleStaff);
            else if (nameType == NAME_COMMON)
                AddName(GetFemaleCommonNames(), name, index, languageId, nameType, isFemaleStaff);
        });
        static EntityWriterHook<0x4E8BE0, 0x51F737, false> staffHook(0x573C3A, 0x573C5A, 0x573C9A,
        [](void *entity, UShort index, UChar languageId, UInt nameType, WideChar const *name) {
            Bool isFemaleStaff = IsFemaleStaff(
                raw_ptr<WideChar const>(*raw_ptr<void *>(entity), 0x10),
                raw_ptr<WideChar const>(*raw_ptr<void *>(entity), 0x30),
                raw_ptr<WideChar const>(*raw_ptr<void *>(entity), 0x80),
                FifamDate(*raw_ptr<UInt>(*raw_ptr<void *>(entity), 0x174))
            );
            if (nameType == NAME_FIRST)
                AddName(GetFemaleNames(), name, index, languageId, nameType, isFemaleStaff);
            else if (nameType == NAME_LAST)
                AddName(GetFemaleSurnames(), name, index, languageId, nameType, isFemaleStaff);
            else if (nameType == NAME_COMMON)
                AddName(GetFemaleCommonNames(), name, index, languageId, nameType, isFemaleStaff);
        });
        static EntityWriterHook<0x4C4EF2, 0x4D4316, true> playerHook(0x521978, 0x521993, 0x5219E5,
        [](void *entity, UShort index, UChar languageId, UInt nameType, WideChar const *name) {
            if (nameType == NAME_FIRST)
                AddName(GetFemaleNames(), name, index, languageId, nameType, false);
            else if (nameType == NAME_LAST)
                AddName(GetFemaleSurnames(), name, index, languageId, nameType, false);
            else if (nameType == NAME_COMMON)
                AddName(GetFemaleCommonNames(), name, index, languageId, nameType, false);
        });
        static EntityWriterHook<0x4E3058, 0, false> refereeHook(0x53FE2E, 0x53FE4B, 0,
        [](void *entity, UShort index, UChar languageId, UInt nameType, WideChar const *name) {
            Bool isFemaleReferee = IsFemaleReferee(
                raw_ptr<WideChar const>(raw_ptr<void *>(entity)),
                raw_ptr<WideChar const>(raw_ptr<void *>(entity), 0x20)
            );
            if (nameType == NAME_FIRST)
                AddName(GetFemaleNames(), name, index, languageId, nameType, isFemaleReferee);
            else if (nameType == NAME_LAST)
                AddName(GetFemaleSurnames(), name, index, languageId, nameType, isFemaleReferee);
        });
    }
}
