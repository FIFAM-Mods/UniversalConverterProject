#include "..\WinHeader.h"
#include "Settings.h"
#include <io.h>
#include <fcntl.h>

std::string SettingsHelper::ToUTF8(std::wstring const &wstr) {
    if (wstr.empty())
        return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring SettingsHelper::ToUTF16(std::string const &str) {
    if (str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring strTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &strTo[0], size_needed);
    return strTo;
}

void SettingsAbstract::Read(std::filesystem::path const &filename) {
    Reset();
    FILE *file = nullptr;
    _wfopen_s(&file, filename.c_str(), L"rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long fileSizeWithBom = ftell(file);
        fseek(file, 0, SEEK_SET);
        enum class encoding { utf8, utf16le, utf16be } enc = encoding::utf8;
        long numBytesToSkip = 0;
        if (fileSizeWithBom >= 2) {
            unsigned char bom[3];
            bom[0] = 0;
            fread(&bom, 1, 2, file);
            fseek(file, 0, SEEK_SET);
            if (bom[0] == 0xFE && bom[1] == 0xFF) {
                enc = encoding::utf16be;
                numBytesToSkip = 2;
            }
            else if (bom[0] == 0xFF && bom[1] == 0xFE) {
                enc = encoding::utf16le;
                numBytesToSkip = 2;
            }
            else if (fileSizeWithBom >= 3) {
                bom[0] = 0;
                fread(&bom, 1, 3, file);
                fseek(file, 0, SEEK_SET);
                if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
                    numBytesToSkip = 3;
            }
        }
        long totalSize = fileSizeWithBom - numBytesToSkip;
        if (totalSize > 0) {
            char *fileData = new char[totalSize];
            fseek(file, numBytesToSkip, SEEK_SET);
            fread(fileData, 1, totalSize, file);
            fclose(file);
            long numWideChars = 0;
            switch (enc) {
            case encoding::utf8:
                numWideChars = MultiByteToWideChar(CP_UTF8, 0, fileData, totalSize, 0, 0);
                break;
            case encoding::utf16le:
            case encoding::utf16be:
                numWideChars = totalSize / 2;
                break;
            }
            wchar_t *data = new wchar_t[numWideChars];
            memset(data, 0, numWideChars * 2);
            switch (enc) {
            case encoding::utf8:
                MultiByteToWideChar(CP_UTF8, 0, fileData, totalSize, data, numWideChars);
                break;
            case encoding::utf16le:
            case encoding::utf16be:
                memcpy(data, fileData, totalSize);
                break;
            }
            delete[] fileData;
            if (enc == encoding::utf16be) {
                for (long i = 0; i < numWideChars; i++)
                    data[i] = (data[i] >> 8) | (data[i] << 8);
            }
            std::wstring sectionName;
            std::wstring paramName;
            std::wstring paramValue;
            std::wstring currentSectionName;
            bool foundES = false;
            enum class parsing_state { before_name, name, before_parameter, parameter, section, ignore } state = parsing_state::before_name;
            auto lineEnd = [&]() {
                if (!sectionName.empty()) {
                    if (sectionName.starts_with(L'[') && sectionName.ends_with(L']')) {
                        currentSectionName = sectionName.substr(1, sectionName.size() - 2);
                        SettingsHelper::Trim(currentSectionName);
                    }
                    sectionName.clear();
                }
                else {
                    SettingsHelper::Trim(paramValue);
                    if (paramValue.size() >= 2 && paramValue.front() == L'"' && paramValue.back() == L'"')
                        paramValue = paramValue.substr(1, paramValue.size() - 2);
                    if (!paramName.empty() && !paramValue.empty()) {
                        if (!currentSectionName.empty())
                            paramName = currentSectionName + L"/" + paramName;
                        auto paramKey = SettingsHelper::ToUTF8(SettingsHelper::ToLower(paramName));
                        if (mParametersMap.contains(paramKey)) {
                            auto &parameter = mParametersMap[paramKey];
                            parameter.mFromString((void *)((unsigned int)this + parameter.mOffset), paramValue);
                        }
                    }
                    paramName.clear();
                    paramValue.clear();
                    foundES = false;
                }
                state = parsing_state::before_name;
            };
            for (long i = 0; i < numWideChars; i++) {
                if (data[i] == L'\n')
                    lineEnd();
                else if (data[i] == L'\r') {
                    if ((i + 1) < numWideChars && data[i + 1] == L'\n')
                        i++;
                    lineEnd();
                }
                else if (state != parsing_state::ignore) {
                    if (data[i] == L';')
                        state = parsing_state::ignore;
                    else {
                        if (state == parsing_state::before_name) {
                            if (data[i] == L'=')
                                state = parsing_state::ignore;
                            else if (data[i] == '[') {
                                sectionName += data[i];
                                state = parsing_state::section;
                            }
                            else if (data[i] != ' ') {
                                paramName += data[i];
                                state = parsing_state::name;
                            }
                        }
                        else if (state == parsing_state::name) {
                            if (data[i] == L' ' || data[i] == L'=')
                                state = parsing_state::before_parameter;
                            else
                                paramName += data[i];
                        }
                        else if (state == parsing_state::before_parameter) {
                            if (data[i] == '=' && !foundES)
                                foundES = true;
                            else if (data[i] != ' ') {
                                paramValue += data[i];
                                state = parsing_state::parameter;
                            }
                        }
                        else if (state == parsing_state::parameter)
                            paramValue += data[i];
                        else if (state == parsing_state::section) {
                            sectionName += data[i];
                            if (data[i] == '[')
                                state = parsing_state::ignore;
                        }
                    }
                }
            }
            lineEnd();
            delete[] data;
        }
        fclose(file);
    }
}

void SettingsAbstract::Write(std::filesystem::path const &filename, bool onlyNonDefault) {
    if (filename.has_parent_path() && !exists(filename.parent_path()))
        std::filesystem::create_directories(filename.parent_path());
    FILE *file = nullptr;
    _wfopen_s(&file, filename.c_str(), L"wb");
    if (file) {
        if (!mParametersMap.empty()) {
            _setmode(_fileno(file), _O_U8TEXT);
            std::vector<SettingsParameter *> vecParameters(mParametersMap.size());
            for (auto &[key, parameter] : mParametersMap)
                vecParameters[parameter.mIndex] = &parameter;
            std::wstring currentSection;
            bool firstLine = true;
            for (auto &parameter : vecParameters) {
                if (!onlyNonDefault || !parameter->mCompare((void const *)((unsigned int)this + parameter->mOffset), mDefaultValues.data() + parameter->mOffset)) {
                    std::wstring thisName = SettingsHelper::ToUTF16(parameter->mName);
                    auto slashPos = thisName.rfind(L'/');
                    std::wstring thisSection;
                    if (slashPos != std::wstring::npos) {
                        thisSection = thisName.substr(0, slashPos);
                        thisName = thisName.substr(slashPos + 1);
                    }
                    if (!thisName.empty()) {
                        std::wstring thisSectionL = SettingsHelper::ToLower(thisSection);
                        if (thisSectionL != currentSection) {
                            if (!firstLine)
                                fputws(L"\n", file);
                            fwprintf(file, L"[%s]\n", thisSection.c_str());
                            currentSection = thisSectionL;
                        }
                        std::wstring thisValue;
                        parameter->mToString((void *)((unsigned int)this + parameter->mOffset), thisValue);
                        if (thisValue.starts_with(L' ') || thisValue.ends_with(L' '))
                            thisValue = L"\"" + thisValue + L"\"";
                        fwprintf(file, L"%s = %s\n", thisName.c_str(), thisValue.c_str());
                        firstLine = false;
                    }
                }
            }
        }
        fclose(file);
    }
}

void SettingsAbstract::Reset() {
    for (auto &[name, parameter] : mParametersMap)
        parameter.mAssign((void *)((unsigned int)this + parameter.mOffset), mDefaultValues.data() + parameter.mOffset);
}

SettingsAbstract::SettingsAbstract() {}

SettingsAbstract::SettingsAbstract(std::filesystem::path const &filename) {
    Read(filename);
}
