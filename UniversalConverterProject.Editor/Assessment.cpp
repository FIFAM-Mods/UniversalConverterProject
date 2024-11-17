#include "Assessment.h"
#include "DatabaseOptions.h"
#include "FifamReadWrite.h"

using AssessmentInfoAFC = Array<Float, 9>;
using AssessmentInfoCAF = Array<Float, 5>;

Map<UChar, AssessmentInfoAFC> &GetAssessmentInfoAFC() {
    static Map<UChar, AssessmentInfoAFC> assessmentInfoAFC;
    return assessmentInfoAFC;
}

Map<UChar, AssessmentInfoCAF> &GetAssessmentInfoCAF() {
    static Map<UChar, AssessmentInfoCAF> assessmentInfoCAF;
    return assessmentInfoCAF;
}

Bool METHOD OnLoadAssessmentSav(void *world, DUMMY_ARG, WideChar const *filePath) {
    CallMethod<0x4CA360>(world, filePath);
    Path databasePath = FM::GameDirPath(DatabaseFolderName);
    {
        FifamReader r(databasePath / "AssessmentAFC.sav");
        if (r.Available()) {
            r.SkipLine();
            while (!r.IsEof()) {
                if (!r.EmptyLine()) {
                    UChar countryId = 0;
                    String countryName;
                    AssessmentInfoAFC i = {};
                    r.ReadLineWithSeparator(L'\t', countryId, countryName, i[0], i[1], i[2], i[3], i[4], i[5], i[6], i[7], i[8]);
                    if (countryId >= 1 && countryId <= 207)
                        GetAssessmentInfoAFC()[countryId] = i;
                }
                else
                    r.SkipLine();
            }
        }
    }
    {
        FifamReader r(databasePath / "AssessmentCAF.sav");
        if (r.Available()) {
            r.SkipLine();
            while (!r.IsEof()) {
                if (!r.EmptyLine()) {
                    UChar countryId = 0;
                    String countryName;
                    AssessmentInfoCAF i = {};
                    r.ReadLineWithSeparator(L'\t', countryId, countryName, i[0], i[1], i[2], i[3], i[4]);
                    if (countryId >= 1 && countryId <= 207)
                        GetAssessmentInfoCAF()[countryId] = i;
                }
                else
                    r.SkipLine();
            }
        }
    }
    return true;
}

void METHOD OnWriteAssessmentToBinaryDatabase(void *t, DUMMY_ARG, void *binaryFile) {
    CallMethod<0x538CA0>(t, binaryFile);
    
    // binary database version 20130010
    auto WriteFourcc = [&binaryFile](UInt fourcc) { CallMethod<0x5511C0>(binaryFile, fourcc); };
    auto WriteUInt = [&binaryFile](UInt value) { CallMethod<0x551060>(binaryFile, value); };
    auto WriteUChar = [&binaryFile](UChar value) { CallMethod<0x550E80>(binaryFile, value); };
    auto WriteFloat = [&binaryFile](Float value) { CallMethod<0x551160>(binaryFile, value); };

    WriteFourcc('ASSM');
    WriteUInt(GetAssessmentInfoAFC().size());
    for (auto &[countryId, info] : GetAssessmentInfoAFC()) {
        WriteUChar(countryId);
        for (UInt i = 0; i < 9; i++)
            WriteFloat(info[i]);
    }
    WriteUInt(GetAssessmentInfoCAF().size());
    for (auto &[countryId, info] : GetAssessmentInfoCAF()) {
        WriteUChar(countryId);
        for (UInt i = 0; i < 5; i++)
            WriteFloat(info[i]);
    }
    WriteFourcc('ASSM');
}

void PatchAssessment(FM::Version v) {
    if (v.id() == VERSION_ED_13) {
        patch::RedirectCall(0x4D4172, OnLoadAssessmentSav);
        patch::RedirectCall(0x4D97A4, OnWriteAssessmentToBinaryDatabase);
        patch::RedirectCall(0x4D9B0D, OnWriteAssessmentToBinaryDatabase);
    }
}
