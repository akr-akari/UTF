#ifndef Z_AKR_UTF_HH
#define Z_AKR_UTF_HH

#include <bit>
#include <cstdint>
#include <utility>

namespace akr
{
    using Byte = unsigned char;

    struct UTF8  final
    {
        public:
        static constexpr auto CountCodeUnit(std::uint32_t codePoint) noexcept -> std::size_t
        {
            if (codePoint <= 0x0000'007F)
            {
                return 1;
            }
            else if (codePoint <= 0x0000'07FF)
            {
                return 2;
            }
            else if (codePoint <= 0x0000'FFFF)
            {
                return 3;
            }
            else if (codePoint <= 0x0010'FFFF)
            {
                return 4;
            }
            else
            {
                return 0;
            }
        }

        static constexpr auto FromCodePoint(std::uint32_t codePoint, Byte* dstBegin) noexcept -> Byte*
        {
            const auto codePointTo = [&]<std::size_t... I_>(std::index_sequence<I_...>) constexpr noexcept
            {
                constexpr auto restCount_      = sizeof...(I_);

                constexpr auto headRightShift_ = 6 * restCount_;

                constexpr auto headMask_       = ~(0b0111'1111 >> restCount_);

                *dstBegin = static_cast<Byte>((codePoint >> headRightShift_) | headMask_);

                for (std::size_t i_ = 1; i_ <= restCount_; i_++)
                {
                    dstBegin[i_] = static_cast<Byte>(((codePoint >> (6 * (restCount_ - i_))) & 0b0011'1111)
                                                     | 0b1000'0000);
                }

                constexpr auto byteCount_ = restCount_ + 1;

                return dstBegin + byteCount_;
            };

            const auto codeUnitCount = CountCodeUnit(codePoint);

            switch (codeUnitCount)
            {
                case 1:
                    *dstBegin = static_cast<Byte>(codePoint);
                    return dstBegin + 1;
                case 2:
                    return codePointTo(std::make_index_sequence<1>{});
                case 3:
                    return codePointTo(std::make_index_sequence<2>{});
                case 4:
                    return codePointTo(std::make_index_sequence<3>{});
                case 5:
                    return codePointTo(std::make_index_sequence<4>{});
                case 6:
                    return codePointTo(std::make_index_sequence<5>{});
                default:
                    return dstBegin;
            }
        }

        static constexpr auto NextCodePoint(const Byte* srcBegin) noexcept
        {
            struct Result final
            {
                std::uint32_t  CodePoint;

                const Byte*    Next;
            };

            const auto toCodePoint = [&]<std::size_t... I_>(std::index_sequence<I_...>) constexpr noexcept
            {
                constexpr auto restCount_     = sizeof...(I_);

                constexpr auto byteCount_     = restCount_ + 1;

                constexpr auto headMask_      = 0b0001'1111 >> (restCount_ - 1);
                constexpr auto headLeftShift_ = 6 * restCount_;

                constexpr auto restMask_      = 0b0011'1111;
                constexpr auto restLeftShift_ = [](auto index_) consteval noexcept
                {
                    return 6 * (sizeof...(I_) - index_);
                };

                constexpr auto index_ = [](auto i_) consteval noexcept
                {
                    return i_ + 1;
                };

                return Result { static_cast<std::uint32_t>((*srcBegin & headMask_) << headLeftShift_
                                      | (... | ((srcBegin[index_(I_)] & restMask_) << restLeftShift_(index_(I_))))),
                                srcBegin  + byteCount_ };
            };

            const auto leadingOneCount = std::countl_one(*srcBegin);

            switch (leadingOneCount)
            {
                case 0:
                    return Result { static_cast<std::uint32_t>(*srcBegin), srcBegin + 1 };
                case 2:
                    return toCodePoint(std::make_index_sequence<1>{});
                case 3:
                    return toCodePoint(std::make_index_sequence<2>{});
                case 4:
                    return toCodePoint(std::make_index_sequence<3>{});
                case 5:
                    return toCodePoint(std::make_index_sequence<4>{});
                case 6:
                    return toCodePoint(std::make_index_sequence<5>{});
                default:
                    return Result { 0, srcBegin };
            }
        }

        public:
        template<class Encoding, class Char>
        static constexpr auto Decode(const Byte*    srcBegin, const Byte*    srcEnd, Char*    dstBegin)
            noexcept -> std::size_t
        {
            auto dstLength = std::size_t();

            for (; srcBegin < srcEnd;)
            {
                const auto& [codePoint, next] = UTF8::NextCodePoint(srcBegin);

                if (next == srcBegin)
                {
                    return dstLength;
                }

                if (dstBegin)
                {
                    dstBegin = Encoding::FromCodePoint(codePoint, dstBegin);
                }

                dstLength += Encoding::CountCodeUnit(codePoint);

                srcBegin = next;
            }

            return dstLength;
        }

        template<class Encoding, class Char>
        static constexpr auto Encode(const Char*    srcBegin, const Char*    srcEnd, Byte*    dstBegin)
            noexcept -> std::size_t
        {
            auto dstLength = std::size_t();

            for (; srcBegin < srcEnd;)
            {
                const auto& [codePoint, next] = Encoding::NextCodePoint(srcBegin);

                if (next == srcBegin)
                {
                    return dstLength;
                }

                if (dstBegin)
                {
                    dstBegin = UTF8::FromCodePoint(codePoint, dstBegin);
                }

                dstLength += UTF8::CountCodeUnit(codePoint);

                srcBegin = next;
            }

            return dstLength;
        }

        template<class Encoding, class Char>
        static constexpr auto Decode(const char*    srcBegin, const char*    srcEnd, Char*    dstBegin)
            noexcept -> std::size_t
        {
            return Decode<Encoding>(std::bit_cast<const Byte*>(srcBegin), std::bit_cast<const Byte*>(srcEnd), dstBegin);
        }

        template<class Encoding, class Char>
        static constexpr auto Encode(const Char*    srcBegin, const Char*    srcEnd, char*    dstBegin)
            noexcept -> std::size_t
        {
            return Decode<Encoding>(srcBegin, srcEnd, std::bit_cast<Byte*>(dstBegin));
        }

        template<class Encoding, class Char>
        static constexpr auto Decode(const char8_t* srcBegin, const char8_t* srcEnd, Char*    dstBegin)
            noexcept -> std::size_t
        {
            return Decode<Encoding>(std::bit_cast<const Byte*>(srcBegin), std::bit_cast<const Byte*>(srcEnd), dstBegin);
        }

        template<class Encoding, class Char>
        static constexpr auto Encode(const Char*    srcBegin, const Char*    srcEnd, char8_t* dstBegin)
            noexcept -> std::size_t
        {
            return Decode<Encoding>(srcBegin, srcEnd, std::bit_cast<Byte*>(dstBegin));
        }
    };

    struct UTF16 final
    {
        public:
        static constexpr auto CountCodeUnit(std::uint32_t codePoint) noexcept -> std::size_t
        {
            if (codePoint <= 0x0000'FFFF)
            {
                return 1;
            }
            else if (codePoint <= 0x0010'FFFF)
            {
                return 2;
            }
            else
            {
                return 0;
            }
        }

        static constexpr auto FromCodePoint(std::uint32_t codePoint, char16_t* dstBegin) noexcept -> char16_t*
        {
            const auto codePointTo = [&]() constexpr noexcept
            {
                codePoint -= 0x1'0000;

                dstBegin[0] = static_cast<char16_t>(((codePoint >> 10) & 0b0000'0011'1111'1111)
                                                    | 0b1101'1000'0000'0000);

                dstBegin[1] = static_cast<char16_t>(((codePoint      ) & 0b0000'0011'1111'1111)
                                                    | 0b1101'1100'0000'0000);

                return dstBegin + 2;
            };

            const auto codeUnitCount = CountCodeUnit(codePoint);

            switch (codeUnitCount)
            {
                case 1:
                    *dstBegin = static_cast<char16_t>(codePoint);
                    return dstBegin + 1;
                case 2:
                    return codePointTo();
                default:
                    return dstBegin;
            }
        }

        static constexpr auto NextCodePoint(const char16_t* srcBegin) noexcept
        {
            struct Result final
            {
                std::uint32_t   CodePoint;

                const char16_t* Next;
            };

            const auto toCodePoint = [&]() constexpr noexcept
            {
                return Result { static_cast<std::uint32_t>((((srcBegin[0] & 0b0000'0011'1111'1111) << 10)
                                                            |(srcBegin[1] & 0b0000'0011'1111'1111))
                                                           + 0x1'0000),
                                srcBegin + 2 };
            };

            const auto check = [&]() constexpr noexcept
            {
                return ((srcBegin[0] >> 10) == 0b1101'10) && ((srcBegin[1] >> 10) == 0b1101'11);
            };

            if (!check())
            {
                return Result { static_cast<std::uint32_t>(*srcBegin), srcBegin + 1 };
            }
            else
            {
                return toCodePoint();
            }
        }

        public:
        static constexpr auto Decode(const char16_t* srcBegin, const char16_t* srcEnd, Byte*     dstBegin)
            noexcept -> std::size_t
        {
            return UTF8::Encode<UTF16>(srcBegin, srcEnd, dstBegin);
        }

        static constexpr auto Encode(const Byte*     srcBegin, const Byte*     srcEnd, char16_t* dstBegin)
            noexcept -> std::size_t
        {
            return UTF8::Decode<UTF16>(srcBegin, srcEnd, dstBegin);
        }

        static constexpr auto Decode(const char16_t* srcBegin, const char16_t* srcEnd, char*     dstBegin)
            noexcept -> std::size_t
        {
            return Decode(srcBegin, srcEnd, std::bit_cast<Byte*>(dstBegin));
        }

        static constexpr auto Encode(const char*     srcBegin, const char*     srcEnd, char16_t* dstBegin)
            noexcept -> std::size_t
        {
            return Encode(std::bit_cast<const Byte*>(srcBegin), std::bit_cast<const Byte*>(srcEnd), dstBegin);
        }

        static constexpr auto Decode(const char16_t* srcBegin, const char16_t* srcEnd, char8_t*  dstBegin)
            noexcept -> std::size_t
        {
            return Decode(srcBegin, srcEnd, std::bit_cast<Byte*>(dstBegin));
        }

        static constexpr auto Encode(const char8_t*  srcBegin, const char8_t*  srcEnd, char16_t* dstBegin)
            noexcept -> std::size_t
        {
            return Encode(std::bit_cast<const Byte*>(srcBegin), std::bit_cast<const Byte*>(srcEnd), dstBegin);
        }
    };

    struct UTF32 final
    {
        public:
        static constexpr auto CountCodeUnit(std::uint32_t codePoint) noexcept -> std::size_t
        {
            return sizeof(codePoint) / sizeof(char32_t);
        }

        static constexpr auto FromCodePoint(std::uint32_t codePoint, char32_t* dstBegin) noexcept -> char32_t*
        {
            *dstBegin = static_cast<char32_t>(codePoint);

            return dstBegin + 1;
        }

        static constexpr auto NextCodePoint(const char32_t* srcBegin) noexcept
        {
            struct Result final
            {
                std::uint32_t   CodePoint;

                const char32_t* Next;
            };

            return Result { static_cast<std::uint32_t>(*srcBegin), srcBegin + 1 };
        }

        public:
        static constexpr auto Decode(const char32_t* srcBegin, const char32_t* srcEnd, Byte*     dstBegin)
            noexcept -> std::size_t
        {
            return UTF8::Encode<UTF32>(srcBegin, srcEnd, dstBegin);
        }

        static constexpr auto Encode(const Byte*     srcBegin, const Byte*     srcEnd, char32_t* dstBegin)
            noexcept -> std::size_t
        {
            return UTF8::Decode<UTF32>(srcBegin, srcEnd, dstBegin);
        }

        static constexpr auto Decode(const char32_t* srcBegin, const char32_t* srcEnd, char*     dstBegin)
            noexcept -> std::size_t
        {
            return Decode(srcBegin, srcEnd, std::bit_cast<Byte*>(dstBegin));
        }

        static constexpr auto Encode(const char*     srcBegin, const char*     srcEnd, char32_t* dstBegin)
            noexcept -> std::size_t
        {
            return Encode(std::bit_cast<const Byte*>(srcBegin), std::bit_cast<const Byte*>(srcEnd), dstBegin);
        }

        static constexpr auto Decode(const char32_t* srcBegin, const char32_t* srcEnd, char8_t*  dstBegin)
            noexcept -> std::size_t
        {
            return Decode(srcBegin, srcEnd, std::bit_cast<Byte*>(dstBegin));
        }

        static constexpr auto Encode(const char8_t*  srcBegin, const char8_t*  srcEnd, char32_t* dstBegin)
            noexcept -> std::size_t
        {
            return Encode(std::bit_cast<const Byte*>(srcBegin), std::bit_cast<const Byte*>(srcEnd), dstBegin);
        }
    };
}

#ifdef  D_AKR_TEST
#include <cstring>

namespace akr::test
{
    AKR_TEST(UTF,
    {
        auto test = [](const char*     u_8String, std::size_t u_8Length,
                       const char16_t* u16String, std::size_t u16Length,
                       const char32_t* u32String, std::size_t u32Length)
        {
            {
                char16_t* u16String1 = nullptr;
                auto u16Length1 = UTF16::Encode(u_8String, u_8String + u_8Length, u16String1);
                assert(u16Length1 == u16Length);

                u16String1 = new char16_t[u16Length1 + 1] {};
                u16Length1 = UTF16::Encode(u_8String, u_8String + u_8Length, u16String1);
                assert(u16Length1 == u16Length);
                assert(!std::memcmp(u16String1, u16String, sizeof(char16_t) * u16Length1));

                char*     u_8string1 = nullptr;
                auto u_8Length1 = UTF16::Decode(u16String1, u16String1 + u16Length1, u_8string1);
                assert(u_8Length1 == u_8Length);

                u_8string1 = new char    [u_8Length1 + 1] {};
                u_8Length1 = UTF16::Decode(u16String1, u16String1 + u16Length1, u_8string1);
                assert(u_8Length1 == u_8Length);
                assert(!std::memcmp(u_8string1, u_8String, sizeof(char    ) * u_8Length1));

                delete[] u16String1;
                delete[] u_8string1;
            }
            {
                char32_t* u32String1 = nullptr;
                auto u32Length1 = UTF32::Encode(u_8String, u_8String + u_8Length, u32String1);
                assert(u32Length1 == u32Length);

                u32String1 = new char32_t[u32Length1 + 1] {};
                u32Length1 = UTF32::Encode(u_8String, u_8String + u_8Length, u32String1);
                assert(u32Length1 == u32Length);
                assert(!std::memcmp(u32String1, u32String, sizeof(char32_t) * u32Length1));

                char*     u_8string1 = nullptr;
                auto u_8Length1 = UTF32::Decode(u32String1, u32String1 + u32Length1, u_8string1);
                assert(u_8Length1 == u_8Length);

                u_8string1 = new char    [u_8Length1 + 1] {};
                u_8Length1 = UTF32::Decode(u32String1, u32String1 + u32Length1, u_8string1);
                assert(u_8Length1 == u_8Length);
                assert(!std::memcmp(u_8string1, u_8String, sizeof(char    ) * u_8Length1));

                delete[] u32String1;
                delete[] u_8string1;
            }
        };

        // Hello, 世界𪚥!🤗
        test("Hello, \xE4\xB8\x96\xE7\x95\x8C\xF0\xAA\x9A\xA5!\xF0\x9F\xA4\x97", 22,
            u"Hello, \u4E16\u754C\U0002A6A5!\U0001F917", 14,
            U"Hello, \u4E16\u754C\U0002A6A5!\U0001F917", 12);
    });
}
#endif//D_AKR_TEST

#endif//Z_AKR_UTF_HH
