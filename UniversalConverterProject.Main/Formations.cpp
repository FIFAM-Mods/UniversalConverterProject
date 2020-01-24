#include "Formations.h"
#include "license_check/license_check.h"
#include "tinyxml/tinyxml.h"

using namespace plugin;

const bool USER_FORMATIONS_XML = false;

bool gReadingFormationsXML = USER_FORMATIONS_XML;

unsigned char METHOD OnReadFormationsFile(void *t, DUMMY_ARG, wchar_t const *filename) {
    gReadingFormationsXML = true;
    auto result = CallMethodAndReturn<unsigned char, 0x1175B00>(t, filename);
    gReadingFormationsXML = USER_FORMATIONS_XML;
    return result;
}

void OnReadFormationsFileData(wchar_t *buf, unsigned int size, unsigned int count, void *file) {
    Call<0x15772CF>(buf, size, count, file);
    if (!gReadingFormationsXML) {
        wchar_t enc_key[16];
        memcpy(enc_key, (void *)0x24CF9DC, 32);
        if (count) {
            wchar_t *pBuf = buf;
            unsigned int counter = count;
            do {
                wchar_t val = *pBuf;
                wchar_t *pKey = enc_key;
                unsigned int i = 16;
                do
                {
                    val ^= *pKey;
                    ++pKey;
                    --i;
                } while (i);
                *pBuf = val;
                ++pBuf;
                --counter;
            } while (counter);
        }
    }
    else {
        if (count > 0) {
            wchar_t *oldData = new wchar_t[count];
            memcpy(oldData, buf, count * 2);
            memset(buf, 0, count * 2);
            unsigned int counter = 0;
            for (unsigned int i = 0; i < count; i++) {
                if (oldData[i] != L'\t' && oldData[i] != L'\r' && oldData[i] != L'\n')
                    buf[counter++] = oldData[i];
            }
            delete[] oldData;
        }
    }
}

void WriteUserFormationsFile(const wchar_t *data, unsigned int elementSize, unsigned int count, FILE *f) {
    unsigned char *utf8data = new unsigned char[count];
    for (unsigned int i = 0; i < (count - 1); i++)
        utf8data[i] = (unsigned char)data[i + 1];
    utf8data[count - 1] = '\0';
    FILE *uf = fopen("utf8data.txt", "wb");
    if (uf) {
        fwrite(utf8data, 1, count, uf);
        fclose(uf);
    }
    TiXmlDocument doc;
    doc.Parse((const char *)utf8data, 0, TIXML_ENCODING_UTF8);
    TiXmlPrinter printer;
    printer.SetIndent("\t");
    doc.Accept(&printer);
    const char *formattedData = printer.CStr();
    FILE *ff = fopen("formattedData.txt", "wb");
    if (ff) {
        fwrite(formattedData, 1, printer.Size(), ff);
        fclose(ff);
    }
    unsigned int newDataSize = printer.Size() + 1;
    wchar_t *newData = new wchar_t[newDataSize];
    newData[0] = 0xFEFF;
    for (unsigned int i = 0; i < (newDataSize - 1); i++)
        newData[i + 1] = (wchar_t)formattedData[i];
    Call<0x1574A5C>(newData, elementSize, newDataSize, f);
    FILE *tf = fopen("test.txt", "wb");
    if (tf) {
        fwrite(newData, elementSize, newDataSize, tf);
        fclose(tf);
    }
    delete[] utf8data;
    delete[] newData;
}

void PatchFormations(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        static std::wstring formationsFileName = Magic<'U','C','P','F','o','r','m','a','t','i','o','n','s','.','x','m','l'>(2409944510);
        patch::SetPointer(0x1176C2F + 1, (void *)formationsFileName.c_str());
        patch::RedirectCall(0x1176C50, OnReadFormationsFile);
        patch::RedirectJump(0x1175B9C, (void *)0x1175BD6);
        patch::RedirectCall(0x1175B86, OnReadFormationsFileData);

        // manager preferred formation multiplier (default = 1.01)
        //static float fManagerPreferredFormationMultiplier = 1.06f;
        //patch::SetPointer(0x13DA06B + 2, &fManagerPreferredFormationMultiplier);
        
        // 4 defenders limit
        patch::Nop(0x11770F0, 2);

        if (USER_FORMATIONS_XML) {
            static std::wstring userFormationsFileName = Magic<'U', 'C', 'P', 'U', 's', 'e', 'r', 'F', 'o', 'r', 'm', 'a', 't', 'i', 'o', 'n', 's', '.', 'x', 'm', 'l'>(1449696081);
            patch::SetPointer(0x1172847 + 1, (void *)userFormationsFileName.c_str());
            patch::SetPointer(0x1176C6B + 1, (void *)userFormationsFileName.c_str());
            // temporary
            patch::RedirectJump(0x1172A10, (void *)0x1172A46);
            // xml ident
            patch::RedirectCall(0x1172A59, WriteUserFormationsFile);
        }

        // AI rotation
        
        const unsigned char maxPlayersToRotate = 12; // 18
        patch::SetUChar(0x13D7498 + 2, maxPlayersToRotate);
        patch::SetUInt(0x13D749B + 4, maxPlayersToRotate);
        /*
        patch::SetUInt(0x309B46C, 2);
        patch::SetUInt(0x309B474, 4);
        patch::SetUInt(0x309B47C, 6);
        patch::SetUInt(0x309B484, 8);
        patch::SetUInt(0x309B48C, 10);

        static int matchImportanceTeamLevelTable[] = {
            700, 11,
            600, 10,
            500, 9,
            400, 8,
            300, 7,
            250, 6,
            200, 5,
            150, 4,
            100, 3,
            75, 2,
            40, 1,
            20, 0,
            0, -2,
            -1000, -3
        };

        patch::SetPointer(0x13D735C + 1, matchImportanceTeamLevelTable);
        patch::SetUChar(0x13D736F + 2, std::size(matchImportanceTeamLevelTable) / 2);
        */
    }
}
