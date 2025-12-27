#pragma once
#include <Windows.h>
#include "Error.h"

#define METHOD __fastcall
#define DUMMY_ARG int
#define NAKED __declspec(naked)
#define VALIDATE_SIZE(struc, size) static_assert(sizeof(struc) == size, "Invalid struct size of " #struc)

namespace plugin
{
    using AddrType = uintptr_t;
    using OffsetType = intptr_t;

    namespace injector
    {
        inline bool ProtectMemory(AddrType addr, size_t size, DWORD protection) {
            return VirtualProtect((LPVOID)addr, size, protection, &protection) != 0;
        }

        inline bool UnprotectMemory(AddrType addr, size_t size, DWORD &out_oldprotect) {
            return VirtualProtect((LPVOID)addr, size, PAGE_EXECUTE_READWRITE, &out_oldprotect) != 0;
        }

        inline bool UnprotectMemory(AddrType addr, size_t size) {
            DWORD out_oldprotect = 0;
            return VirtualProtect((LPVOID)addr, size, PAGE_EXECUTE_READWRITE, &out_oldprotect) != 0;
        }

        struct scoped_unprotect {
            AddrType addr = 0;
            size_t size = 0;
            DWORD dwOldProtect = 0;
            bool bUnprotected = false;

            scoped_unprotect(AddrType addr, size_t size) {
                if (size == 0)
                    bUnprotected = false;
                else
                    bUnprotected = UnprotectMemory(this->addr = addr, this->size = size, dwOldProtect);
            }

            ~scoped_unprotect() {
                if (bUnprotected)
                    ProtectMemory(this->addr, this->size, this->dwOldProtect);
            }
        };

        inline static std::vector<uint8_t> HexStringToBytes(std::string const &str) {
            std::vector<uint8_t> result;
            uint8_t tempDigit = 0;
            bool tempFlag = false;
            auto hexVal = [](char c) -> uint8_t {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
                if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
                return 0;
            };
            for (auto ch : str) {
                if (ch == ' ')
                    continue;
                else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
                    uint8_t thisDigit = hexVal(ch);
                    if (!tempFlag) {
                        tempDigit = thisDigit << 4;
                        tempFlag = true;
                    }
                    else {
                        tempDigit |= thisDigit;
                        tempFlag = false;
                        result.push_back(tempDigit);
                    }
                }
            }
            return result;
        }

        inline std::string BytesToHexString(void const *data, size_t size) {
            if (size == 0)
                return std::string{};
            unsigned char const *bytes = (unsigned char const *)data;
            std::string out;
            out.reserve(size * 3 - 1);
            static constexpr char HEX[] = "0123456789ABCDEF";
            for (size_t i = 0; i < size; ++i) {
                unsigned char b = bytes[i];
                out.push_back(HEX[b >> 4]);
                out.push_back(HEX[b & 0x0F]);
                if (i + 1 != size)
                    out.push_back(' ');
            }
            return out;
        }

        inline std::string BytesToHexString(std::vector<uint8_t> const &vecBytes) {
            return BytesToHexString(vecBytes.data(), vecBytes.size());
        }

        inline void WriteMemoryRaw(AddrType addr, void *value, size_t size, bool vp) {
            scoped_unprotect xprotect(addr, vp ? size : 0);
            memcpy((void *)addr, value, size);
        }

        inline void ReadMemoryRaw(AddrType addr, void *ret, size_t size, bool vp) {
            scoped_unprotect xprotect(addr, vp ? size : 0);
            memcpy(ret, (void *)addr, size);
        }

        inline void WriteHexBytes(AddrType addr, std::string const &hexBytes, bool vp) {
            auto vecBytes = HexStringToBytes(hexBytes);
            if (!vecBytes.empty())
                WriteMemoryRaw(addr, vecBytes.data(), vecBytes.size(), vp);
        }

        inline std::string ReadHexBytes(AddrType addr, size_t size, bool vp) {
            if (size == 0)
                return std::string();
            std::vector<uint8_t> vecBytes(size, 0);
            ReadMemoryRaw(addr, vecBytes.data(), size, vp);
            return BytesToHexString(vecBytes);
        }

        inline void MemoryFill(AddrType addr, uint8_t value, size_t size, bool vp) {
            scoped_unprotect xprotect(addr, vp ? size : 0);
            memset((void *)addr, value, size);
        }

        template<class T>
        inline T &WriteObject(AddrType addr, const T &value, bool vp = false) {
            scoped_unprotect xprotect(addr, vp ? sizeof(value) : 0);
            return (*(T *)addr = value);
        }

        template<class T>
        inline T &ReadObject(AddrType addr, T &value, bool vp = false) {
            scoped_unprotect xprotect(addr, vp ? sizeof(value) : 0);
            return (value = *(T *)addr);
        }

        template<class T>
        inline void WriteMemory(AddrType addr, T value, bool vp = false) {
            WriteObject(addr, value, vp);
        }

        template<class T>
        inline T ReadMemory(AddrType addr, bool vp = false) {
            T value;
            return ReadObject(addr, value, vp);
        }

        inline AddrType GetAbsoluteOffset(OffsetType rel_value, AddrType end_of_instruction) {
            return end_of_instruction + rel_value;
        }

        inline OffsetType GetRelativeOffset(AddrType abs_value, AddrType end_of_instruction) {
            return (OffsetType)abs_value - (OffsetType)end_of_instruction;
        }

        inline AddrType ReadRelativeOffset(AddrType at, size_t sizeof_addr = 4, bool vp = true) {
            switch (sizeof_addr) {
            case 1: return (GetAbsoluteOffset(ReadMemory<int8_t>(at, vp), at + sizeof_addr));
            case 2: return (GetAbsoluteOffset(ReadMemory<int16_t>(at, vp), at + sizeof_addr));
            case 4: return (GetAbsoluteOffset(ReadMemory<int32_t>(at, vp), at + sizeof_addr));
            }
            return 0;
        }

        inline void MakeRelativeOffset(AddrType at, AddrType dest, size_t sizeof_addr = 4, bool vp = true) {
            switch (sizeof_addr) {
            case 1:
                WriteMemory<int8_t>(at, static_cast<int8_t> (GetRelativeOffset(dest, at + sizeof_addr)), vp);
                break;
            case 2:
                WriteMemory<int16_t>(at, static_cast<int16_t>(GetRelativeOffset(dest, at + sizeof_addr)), vp);
                break;
            case 4:
                WriteMemory<int32_t>(at, static_cast<int32_t>(GetRelativeOffset(dest, at + sizeof_addr)), vp);
                break;
            }
        }

        inline AddrType GetBranchDestination(AddrType at, bool vp = true) {
            switch (ReadMemory<uint8_t>(at, vp)) {
            case 0xE8:
            case 0xE9:
                return ReadRelativeOffset(at + 1, 4, vp);
            case 0xFF:
                switch (ReadMemory<uint8_t>(at + 1, vp)) {
                case 0x15:
                case 0x25:
                    return *(ReadMemory<uintptr_t *>(at + 2, vp));
                }
                break;
            }
            return 0;
        }

        inline AddrType MakeJMP(AddrType at, AddrType dest, bool vp = true) {
            auto p = GetBranchDestination(at, vp);
            WriteMemory<uint8_t>(at, 0xE9, vp);
            MakeRelativeOffset(at + 1, dest, 4, vp);
            return p;
        }

        inline AddrType MakeCALL(AddrType at, AddrType dest, bool vp = true) {
            auto p = GetBranchDestination(at, vp);
            WriteMemory<uint8_t>(at, 0xE8, vp);
            MakeRelativeOffset(at + 1, dest, 4, vp);
            return p;
        }

        inline void MakeJA(AddrType at, AddrType dest, bool vp = true) {
            WriteMemory<uint16_t>(at, 0x0F87, vp);
            MakeRelativeOffset(at + 2, dest, 4, vp);
        }

        inline void MakeNOP(AddrType at, size_t count = 1, bool vp = true) {
            MemoryFill(at, 0x90, count, vp);
        }

        inline void MakeRangedNOP(AddrType at, AddrType until, bool vp = true) {
            return MakeNOP(at, until - at, vp);
        }

        inline void MakeRET(AddrType at, uint16_t pop = 0, bool vp = true) {
            WriteMemory(at, pop ? 0xC2 : 0xC3, vp);
            if (pop) WriteMemory(at + 1, pop, vp);
        }
    };
	
    template<typename RawType, typename Type>
    RawType *raw_ptr(Type *p, unsigned int offset = 0) {
        return reinterpret_cast<RawType *>(reinterpret_cast<AddrType>(p) + offset);
    }

    template <AddrType address, typename... Args>
    void Call(Args... args) {
        reinterpret_cast<void(__cdecl *)(Args...)>(address)(args...);
    }

    template <typename Ret, AddrType address, typename... Args>
    Ret CallAndReturn(Args... args) {
        return reinterpret_cast<Ret(__cdecl *)(Args...)>(address)(args...);
    }

    template <AddrType address, typename C, typename... Args>
    void CallMethod(C _this, Args... args) {
        reinterpret_cast<void(__thiscall *)(C, Args...)>(address)(_this, args...);
    }

    template <typename Ret, AddrType address, typename C, typename... Args>
    Ret CallMethodAndReturn(C _this, Args... args) {
        return reinterpret_cast<Ret(__thiscall *)(C, Args...)>(address)(_this, args...);
    }

    template <unsigned int tableIndex, typename C, typename... Args>
    void CallVirtualMethod(C _this, Args... args) {
        reinterpret_cast<void(__thiscall *)(C, Args...)>((*reinterpret_cast<void ***>(_this))[tableIndex])(_this, args...);
    }

    template <typename Ret, unsigned int tableIndex, typename C, typename... Args>
    Ret CallVirtualMethodAndReturn(C _this, Args... args) {
        return reinterpret_cast<Ret(__thiscall *)(C, Args...)>((*reinterpret_cast<void ***>(_this))[tableIndex])(_this, args...);
    }

    template <typename... Args>
    void CallDynGlobal(AddrType address, Args... args) {
        reinterpret_cast<void(__cdecl *)(Args...)>(address)(args...);
    }

    template <typename Ret, typename... Args>
    Ret CallAndReturnDynGlobal(AddrType address, Args... args) {
        return reinterpret_cast<Ret(__cdecl *)(Args...)>(address)(args...);
    }

    template <typename C, typename... Args>
    void CallMethodDynGlobal(AddrType address, C _this, Args... args) {
        reinterpret_cast<void(__thiscall *)(C, Args...)>(address)(_this, args...);
    }

    template <typename Ret, typename C, typename... Args>
    Ret CallMethodAndReturnDynGlobal(AddrType address, C _this, Args... args) {
        return reinterpret_cast<Ret(__thiscall *)(C, Args...)>(address)(_this, args...);
    }

    class patch {
    public:
        inline static void Nop(AddrType address, unsigned int size){
            injector::MakeNOP(address, size, true);
        }

        inline static AddrType RedirectCall(AddrType address, void *func) {
            return injector::MakeCALL(address, (AddrType)func, true);
        }

        inline static AddrType RedirectJump(AddrType address, void *func) {
            return injector::MakeJMP(address, (AddrType)func, true);
        }

        inline static void SetChar(AddrType address, char value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static void SetUChar(AddrType address, unsigned char value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static void SetShort(AddrType address, short value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static void SetUShort(AddrType address, unsigned short value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static void SetInt(AddrType address, int value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static void SetUInt(AddrType address, unsigned int value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }
		
        inline static void SetInt64(AddrType address, int64_t value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static void SetUInt64(AddrType address, uint64_t value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static void SetFloat(AddrType address, float value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static void SetDouble(AddrType address, double value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        inline static AddrType SetPointer(AddrType address, void *value, bool vp = true) {
            AddrType oldValue = injector::ReadMemory<AddrType>(address, vp);
            injector::WriteMemory(address, value, vp);
            return oldValue;
        }

        inline static AddrType SetPointer(AddrType address, char const *value, bool vp = true) {
            AddrType oldValue = injector::ReadMemory<AddrType>(address, vp);
            injector::WriteMemory(address, value, vp);
            return oldValue;
        }

        inline static AddrType SetPointer(AddrType address, wchar_t const *value, bool vp = true) {
            AddrType oldValue = injector::ReadMemory<AddrType>(address, vp);
            injector::WriteMemory(address, value, vp);
            return oldValue;
        }

        inline static char GetChar(AddrType address, bool vp = true) {
            return injector::ReadMemory<char>(address, vp);
        }

        inline static unsigned char GetUChar(AddrType address, bool vp = true) {
            return injector::ReadMemory<unsigned char>(address, vp);
        }

        inline static short GetShort(AddrType address, bool vp = true) {
            return injector::ReadMemory<short>(address, vp);
        }

        inline static unsigned short GetUShort(AddrType address, bool vp = true) {
            return injector::ReadMemory<unsigned short>(address, vp);
        }

        inline static int GetInt(AddrType address, bool vp = true) {
            return injector::ReadMemory<int>(address, vp);
        }

        inline static unsigned int GetUInt(AddrType address, bool vp = true) {
            return injector::ReadMemory<unsigned int>(address, vp);
        }
		
        inline static int64_t GetInt64(AddrType address, bool vp = true) {
            return injector::ReadMemory<int64_t>(address, vp);
        }

        inline static uint64_t GetUInt64(AddrType address, bool vp = true) {
            return injector::ReadMemory<uint64_t>(address, vp);
        }

        inline static float GetFloat(AddrType address, bool vp = true) {
            return injector::ReadMemory<float>(address, vp);
        }

        inline static double GetDouble(AddrType address, bool vp = true) {
            return injector::ReadMemory<double>(address, vp);
        }

        inline static void *GetPointer(AddrType address, bool vp = true) {
            return injector::ReadMemory<void *>(address, vp);
        }

        template <typename T>
        static void Set(AddrType address, T value, bool vp = true) {
            injector::WriteMemory(address, value, vp);
        }

        template <typename T>
        static T Get(AddrType address, bool vp = true) {
            return injector::ReadMemory<T>(address, vp);
        }

        inline static void SetBytes(AddrType addr, std::string const &hexBytes, bool vp = true) {
            injector::WriteHexBytes(addr, hexBytes, vp);
        }

        inline static std::string GetBytes(AddrType addr, size_t size, bool vp = true) {
            return injector::ReadHexBytes(addr, size, vp);
        }

        inline static void MemCpy(AddrType addr, void *value, size_t size, bool vp = true) {
            injector::WriteMemoryRaw(addr, value, size, vp);
        }

        inline static void MemCpy(void *addr, AddrType value, size_t size, bool vp = true) {
            injector::WriteMemoryRaw((AddrType)addr, (void *)value, size, vp);
        }

        inline static void MemCpy(AddrType addr, AddrType value, size_t size, bool vp = true) {
            injector::WriteMemoryRaw(addr, (void *)value, size, vp);
        }

        inline static void MemCpy(void *addr, void *value, size_t size, bool vp = true) {
            injector::WriteMemoryRaw((AddrType)addr, value, size, vp);
        }
    };
};
