#define D_AKR_TEST
#include "akr_test.hh"

#include "../utf.hh"

#include <cstdio>

int main()
{
    // "Hello, 世界!";
    const char* u8String = "Hello, \xE4\xB8\x96\xE7\x95\x8C!";

    char*       u8Output;
    std::size_t u8Length;

    char16_t* u16String = nullptr;
    auto u16Length = akr::UTF16::Encode(u8String, u8String + 14, u16String);
    u16String = new char16_t[u16Length + 1] {};
    akr::UTF16::Encode(u8String, u8String + 14, u16String);

    u8Output = nullptr;
    u8Length = akr::UTF16::Decode(u16String, u16String + 10, u8Output);
    u8Output = new char[u8Length + 1] {};
    akr::UTF16::Decode(u16String, u16String + 10, u8Output);

    for (auto p = u16String; *p; p++)
    {
        std::printf("%X ", static_cast<unsigned int>(*p));
    }
    std::puts("");

    std::puts(u8Output);

    delete[] u16String;
    delete[] u8Output;

    char32_t* u32String = nullptr;
    auto u32Length = akr::UTF32::Encode(u8String, u8String + 14, u32String);
    u32String = new char32_t[u32Length + 1] {};
    akr::UTF32::Encode(u8String, u8String + 14, u32String);

    u8Output = nullptr;
    u8Length = akr::UTF32::Decode(u32String, u32String + 10, u8Output);
    u8Output = new char[u8Length + 1] {};
    akr::UTF32::Decode(u32String, u32String + 10, u8Output);

    for (auto p = u32String; *p; p++)
    {
        std::printf("%X ", static_cast<unsigned int>(*p));
    }
    std::puts("");

    std::puts(u8Output);

    delete[] u32String;
    delete[] u8Output;
}
