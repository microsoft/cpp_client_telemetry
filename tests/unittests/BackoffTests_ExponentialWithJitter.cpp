//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "backoff/Backoff_ExponentialWithJitter.hpp"

using namespace testing;


class BackoffTests_ExponentialWithJitter : public Test
{
  protected:
    void checkValuesDistributedBetween(MAT::IBackoff& b, int lo, int hi)
    {
        int const NumQueries = 1000;
        int const NumBuckets = 5;
        int buckets[NumBuckets] = {};

        for (int i = 0; i < NumQueries; i++) {
            int x = b.getValue();
            EXPECT_THAT(x, AllOf(Ge(lo), Lt(hi)));
            buckets[std::max(0, std::min(NumBuckets - 1, NumBuckets * (x - lo) / (hi - lo + 1)))]++;
        }

        EXPECT_THAT(buckets, Not(Contains(Lt(NumQueries / NumBuckets / 2))));
        EXPECT_THAT(buckets, Not(Contains(Gt(2 * NumQueries / NumBuckets))));
    }
};


TEST_F(BackoffTests_ExponentialWithJitter, ValidatesArguments)
{
    EXPECT_THAT(MAT::Backoff_ExponentialWithJitter(1000, 300000, 2.0,     1.0).good(), true);
    EXPECT_THAT(MAT::Backoff_ExponentialWithJitter(0,         0, 1.0001,  0.0).good(), true);

    EXPECT_THAT(MAT::Backoff_ExponentialWithJitter(-1,   300000, 2.0,     1.0).good(), false);
    EXPECT_THAT(MAT::Backoff_ExponentialWithJitter(1000,    999, 2.0,     1.0).good(), false);
    EXPECT_THAT(MAT::Backoff_ExponentialWithJitter(1000, 300000, 0.9999,  1.0).good(), false);
    EXPECT_THAT(MAT::Backoff_ExponentialWithJitter(1000, 300000, 2.0,    -0.1).good(), false);
}

TEST_F(BackoffTests_ExponentialWithJitter, StartsAtInitialValue)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 16000, 2.0, 0.0);
    ASSERT_THAT(eb.good(), true);
    EXPECT_THAT(eb.getValue(), 1000);
}

TEST_F(BackoffTests_ExponentialWithJitter, IncreasesExponentially)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 16000, 2.0, 0.0);
    ASSERT_THAT(eb.good(), true);
    EXPECT_THAT(eb.getValue(), 1000);
    EXPECT_THAT(eb.getValue(), 1000);
    eb.increase();
    EXPECT_THAT(eb.getValue(), 2000);
    EXPECT_THAT(eb.getValue(), 2000);
    EXPECT_THAT(eb.getValue(), 2000);
    eb.increase();
    eb.increase();
    EXPECT_THAT(eb.getValue(), 8000);
}

TEST_F(BackoffTests_ExponentialWithJitter, StopsAtMaximumValue)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 16000, 2.0, 0.0);
    ASSERT_THAT(eb.good(), true);
    for (int i = 0; i < 4; i++) {
        eb.increase();
    }
    EXPECT_THAT(eb.getValue(), 16000);
    EXPECT_THAT(eb.getValue(), 16000);
    eb.increase();
    EXPECT_THAT(eb.getValue(), 16000);
}

TEST_F(BackoffTests_ExponentialWithJitter, StopsAtUnevenMaximumValue)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 15432, 2.0, 0.0);
    ASSERT_THAT(eb.good(), true);
    for (int i = 0; i < 4; i++) {
        eb.increase();
    }
    EXPECT_THAT(eb.getValue(), 15432);
    EXPECT_THAT(eb.getValue(), 15432);
    eb.increase();
    EXPECT_THAT(eb.getValue(), 15432);
}

TEST_F(BackoffTests_ExponentialWithJitter, SupportsArbitraryMultiplier)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 1900, 1.25, 0.0);
    ASSERT_THAT(eb.good(), true);
    EXPECT_THAT(eb.getValue(), 1000);
    eb.increase();
    EXPECT_THAT(eb.getValue(), 1250);
    eb.increase();
    EXPECT_THAT(eb.getValue(), 1562);
    eb.increase();
    EXPECT_THAT(eb.getValue(), 1900);
}

TEST_F(BackoffTests_ExponentialWithJitter, AddsFullStepJitter)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 16000, 2.0, 1.0);
    ASSERT_THAT(eb.good(), true);
    checkValuesDistributedBetween(eb, 1000, 2000);
    eb.increase();
    checkValuesDistributedBetween(eb, 2000, 4000);
    eb.increase();
    eb.increase();
    checkValuesDistributedBetween(eb, 8000, 16000);
}

TEST_F(BackoffTests_ExponentialWithJitter, JitterStopsAtMaximumValue)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 16000, 2.0, 1.0);
    ASSERT_THAT(eb.good(), true);
    for (int i = 0; i < 3; i++) {
        eb.increase();
    }
    checkValuesDistributedBetween(eb, 8000, 16000);
    eb.increase();
    checkValuesDistributedBetween(eb, 8000, 16000);
}

TEST_F(BackoffTests_ExponentialWithJitter, JitterStopsAtUnevenMaximumValue)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 15432, 2.0, 1.0);
    ASSERT_THAT(eb.good(), true);
    for (int i = 0; i < 3; i++) {
        eb.increase();
    }
    checkValuesDistributedBetween(eb, 7432, 15432);
    eb.increase();
    checkValuesDistributedBetween(eb, 7432, 15432);
}

TEST_F(BackoffTests_ExponentialWithJitter, JitterDoesNotGoBelowInitialValue)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 1543, 2.0, 1.0);
    ASSERT_THAT(eb.good(), true);
    checkValuesDistributedBetween(eb, 1000, 1543);
}

TEST_F(BackoffTests_ExponentialWithJitter, AddsArbitrarySizedJitter)
{
    MAT::Backoff_ExponentialWithJitter eb(1000, 16000, 2.0, 0.75);
    ASSERT_THAT(eb.good(), true);
    checkValuesDistributedBetween(eb, 1000, 1681);
    eb.increase();
    checkValuesDistributedBetween(eb, 2000, 3363);
    eb.increase();
    eb.increase();
    checkValuesDistributedBetween(eb, 8000, 13454);
    eb.increase();
    checkValuesDistributedBetween(eb, 5092, 16000);
    eb.increase();
    checkValuesDistributedBetween(eb, 5092, 16000);
}

TEST_F(BackoffTests_ExponentialWithJitter, CanBeCreatedFromConfig)
{
    std::unique_ptr<MAT::IBackoff> b = MAT::IBackoff::createFromConfig("E,1000,16000,2.0,0.75");

    // TODO: Move these three to some generic createFromConfig() test sometime.
    EXPECT_THAT(MAT::IBackoff::createFromConfig("x,1000,300000,2.0,1.0"),    IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig(",1000,300000,2.0,1.0"),     IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E!1000,300000,2.0,1.0"),    IsNull());

    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000!300000,2.0,1.0"),    IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000!2.0,1.0"),    IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000,2.0!1.0"),    IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000,2.0,1.0!"),   IsNull());

    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,,300000,2.0,1.0"),        IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,,2.0,1.0"),          IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000,,1.0"),       IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000,2.0,"),       IsNull());

    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000!,300000,2.0,1.0"),    IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000!,2.0,1.0"),    IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000,2.0!,1.0"),    IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000,2.0,1.0!"),    IsNull());

    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,-1,300000,2.0,1.0"),      IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,999,2.0,1.0"),       IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000,0.9999,1.0"), IsNull());
    EXPECT_THAT(MAT::IBackoff::createFromConfig("E,1000,300000,2.0,-0.1"),   IsNull());

    ASSERT_THAT(b, NotNull());
    checkValuesDistributedBetween(*b, 1000, 1681);
    b->increase();
    checkValuesDistributedBetween(*b, 2000, 3363);
    b->increase();
    b->increase();
    checkValuesDistributedBetween(*b, 8000, 13454);
    b->increase();
    checkValuesDistributedBetween(*b, 5092, 16000);
    b->increase();
    checkValuesDistributedBetween(*b, 5092, 16000);
}

