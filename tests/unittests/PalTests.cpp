//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "pal/PseudoRandomGenerator.hpp"
#include "Version.hpp"

using namespace testing;

class PalTests : public Test {};

TEST_F(PalTests, UuidGeneration)
{
    std::string uuid0 = PAL::generateUuidString();

    EXPECT_THAT(uuid0.length(), 36u);

    std::string mask = uuid0;
    for (char& ch : mask) {
#ifdef _WIN32
        if (::isdigit(ch) || (::isupper(ch) && ::isxdigit(ch)))
#else
        if (::isdigit(ch) || (::islower(ch) && ::isxdigit(ch)))
#endif
	{
            ch = 'x';
        }
    }
    EXPECT_THAT(mask, Eq("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"));

    std::string uuid1 = PAL::generateUuidString();

    EXPECT_THAT(uuid1.length(), 36u);

    size_t diff = 0;
    for (size_t i = 0; i < 36; i++) {
        diff += (uuid0[i] != uuid1[i]);
    }
    EXPECT_THAT(diff, Gt(20u));
}

TEST_F(PalTests, PseudoRandomGenerator)
{
    PAL::PseudoRandomGenerator prg;

    size_t const NumQueries = 1000;
    size_t const NumBuckets = 11;
    size_t buckets[NumBuckets] = {};

    for (size_t i = 0; i < NumQueries; i++) {
        double x = prg.getRandomDouble();
        buckets[static_cast<int>(x * NumBuckets)]++;

        // Check range [0,1)
        EXPECT_THAT(x, AllOf(Ge(0.0), Lt(1.0)));
    }

    // Check somewhat uniform distribution
    EXPECT_THAT(buckets, Not(Contains(Lt(NumQueries / NumBuckets / 2))));
    EXPECT_THAT(buckets, Not(Contains(Gt(2 * NumQueries / NumBuckets))));
}

TEST_F(PalTests, SystemTime)
{
    int64_t t0 = PAL::getUtcSystemTimeMs();

    PAL::sleep(369);

    int64_t t1 = PAL::getUtcSystemTimeMs();
    EXPECT_THAT(t1, Gt(t0 + 360));
    EXPECT_THAT(t1, Lt(t0 + 500));
}

TEST_F(PalTests, FormatUtcTimestampMsAsISO8601)
{
    EXPECT_THAT(PAL::formatUtcTimestampMsAsISO8601(0ll),             Eq("1970-01-01T00:00:00.000Z"));
    EXPECT_THAT(PAL::formatUtcTimestampMsAsISO8601(1234567890123ll), Eq("2009-02-13T23:31:30.123Z"));
    EXPECT_THAT(PAL::formatUtcTimestampMsAsISO8601(2147483647999ll), Eq("2038-01-19T03:14:07.999Z"));
}

TEST_F(PalTests, MonotonicTime)
{
    int64_t t0 = PAL::getMonotonicTimeMs();

    PAL::sleep(789);

    int64_t t1 = PAL::getMonotonicTimeMs();
    EXPECT_THAT(t1 - t0, Gt(780));
    EXPECT_THAT(t1 - t0, Lt(900));
}

TEST_F(PalTests, SemanticContextPopulation)
{
 /*   MockISemanticContext context;


    EXPECT_CALL(context, SetAppId(Not(IsEmpty()))).WillOnce(DoDefault());
    EXPECT_CALL(context, SetAppVersion(_)).WillOnce(DoDefault());
    EXPECT_CALL(context, SetAppLanguage(_)).WillOnce(DoDefault());

    EXPECT_CALL(context, SetUserLanguage(_)).WillOnce(DoDefault());
    EXPECT_CALL(context, SetUserTimeZone(Not(IsEmpty()))).WillOnce(DoDefault());
    //EXPECT_CALL(context, SetUserAdvertisingId(_)).WillOnce(DoDefault());

   
    PAL::registerSemanticContext(&context);

    PAL::sleep(500);

    PAL::unregisterSemanticContext(&context);
    */
}

TEST_F(PalTests, SdkVersion)
{
    std::string v = PAL::getSdkVersion();

    // <Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>

    EXPECT_THAT(std::count(v.cbegin(), v.cend(), '-'), Eq(4));
    EXPECT_THAT(v, StartsWith(EVTSDK_VERSION_PREFIX "-"));
    EXPECT_THAT(v.at(v.find('-', 0) + 1), Ne('-'));
    EXPECT_THAT(v, HasSubstr(std::string("-C++-" ECS_SUPP "-")));
    EXPECT_THAT(v, EndsWith(BUILD_VERSION_STR));

    EXPECT_THAT(PAL::getSdkVersion(), Eq(v));
}
