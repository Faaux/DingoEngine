#pragma once
#include <optional>
#include <vector>
#include "DG_Include.h"

namespace DG
{
struct StringId
{
    u32 id = 0;
    const char* data = nullptr;


    friend bool operator==(const StringId& lhs, const StringId& rhs)
    {
        return lhs.id == rhs.id;
    }

    friend bool operator!=(const StringId& lhs, const StringId& rhs)
    {
        return !(lhs == rhs);
    }
};

#if DEBUG

template <u32 tableSize>
class StringHashTable
{
   public:
    StringHashTable() : _values(tableSize) {}
    void Put(u32 key, const char* value);

   private:
    std::vector<std::optional<const char*>> _values;
};

template <u32 tableSize>
void StringHashTable<tableSize>::Put(u32 key, const char* value)
{
    auto index = key % tableSize;
    if (_values[index].has_value())
    {
        Assert(SDL_strcmp(_values[index].value(), value) == 0);

        return;
    }

    _values[index] = SDL_strdup(value);
}

extern StringHashTable<1024> g_StringHashTable;
#endif

// Taken from:
// https://stackoverflow.com/questions/28675727/using-crc32-algorithm-to-hash-string-at-compile-time
// Generate CRC lookup table
template <unsigned c, int k = 8>
struct f : f<((c & 1) ? 0xedb88320 : 0) ^ (c >> 1), k - 1>
{
};
template <unsigned c>
struct f<c, 0>
{
    enum
    {
        value = c
    };
};

#define A(x) B(x) B(x + 128)
#define B(x) C(x) C(x + 64)
#define C(x) D(x) D(x + 32)
#define D(x) E(x) E(x + 16)
#define E(x) F(x) F(x + 8)
#define F(x) G(x) G(x + 4)
#define G(x) H(x) H(x + 2)
#define H(x) I(x) I(x + 1)
#define I(x) f<x>::value,

constexpr unsigned crc_table[] = {A(0)};

// Constexpr implementation and helpers
constexpr u32 crc32_impl(const u8* p, size_t len, u32 crc)
{
    return len ? crc32_impl(p + 1, len - 1, (crc >> 8) ^ crc_table[(crc & 0xFF) ^ *p]) : crc;
}

constexpr u32 crc32(const u8* data, size_t length) { return ~crc32_impl(data, length, ~0); }

//// Trick taken from: https://stackoverflow.com/questions/8936549/constexpr-overloading
//template <typename T>
//constexpr typename std::remove_reference<T>::type makeprval(T&& t)
//{
//    return t;
//}

//#define isprvalconstexpr(e) noexcept(makeprval(e))
//
//#define WSID(X) (isprvalconstexpr(X) ? WSID_CompileTime(X) : WSID_RunTime(X))
//constexpr StringId WSID_CompileTime(const char* str)
//{
//    StringId result;
//    result.id = crc32((u8*)str, sizeof(str));
//    result.data = str;
//    return result;
//}

inline StringId WSID(const char* str)
{
    StringId result;
    result.id = crc32((u8*)str, sizeof(str));
    result.data = str;
#if DEBUG
    g_StringHashTable.Put(result.id, str);
#endif
    return result;
}
}  // namespace DG
