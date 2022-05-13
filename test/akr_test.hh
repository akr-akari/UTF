#pragma once
#ifndef Z_AKR_TEST_HH
#define Z_AKR_TEST_HH

#include <cassert>

#ifdef  D_AKR_TEST
#define U_AKR_TEST_LINE(AKR_NAME, AKR_LINE) U_AKR_TEST_##AKR_NAME##_##AKR_LINE

#define U_AKR_TEST_WITH_LINE_AND_BLOCK(AKR_NAME, AKR_LINE, AKR_BLCK) \
        inline const auto U_AKR_TEST_LINE(AKR_NAME, AKR_LINE) = []() \
        {                                                            \
            AKR_BLCK;                                                \
            return 0;                                                \
        }();

#define AKR_TEST(AKR_NAME, AKR_BLCK) U_AKR_TEST_WITH_LINE_AND_BLOCK(AKR_NAME, __LINE__, AKR_BLCK) // NOLINT
#else
#define AKR_TEST(AKR_NAME, AKR_BLCK)
#endif

#endif//Z_AKR_TEST_HH
