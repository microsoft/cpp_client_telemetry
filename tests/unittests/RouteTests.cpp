//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "system/Route.hpp"

using namespace testing;
using namespace MAT;


class NonCopyableThing {
  public:
    NonCopyableThing()
    {
    }

    NonCopyableThing(NonCopyableThing const&) = delete;
    NonCopyableThing& operator=(NonCopyableThing const&) = delete;
};


class Canary : public std::string {
  public:
    Canary()
    {
    }

    Canary(Canary const&) = default;
    Canary& operator=(Canary const&) = default;

#if (_MSC_VER == 1800) // Visual Studio 2013
    Canary(Canary&& other)
      : std::string(other)
    {
    }

    Canary& operator=(Canary&& other)
    {
        assign(std::move(other));
        return *this;
    }
#else
    Canary(Canary&&) = default;
    Canary& operator=(Canary&&) = default;
#endif

    void bear()
    {
        assign("alive");
    }

    void operator()()
    {
        assign("dead");
    }
};


class RouteTests : public StrictMock<Test> {
  public:
    RouteSink<RouteTests>                                                                  sink0{this, &RouteTests::handleSink0};
    RouteSink<RouteTests, int>                                                             sink1{this, &RouteTests::handleSink1};
    RoutePassThrough<RouteTests, int>                                                      passThrough1a{this, &RouteTests::handlePassThrough1a};
    RoutePassThrough<RouteTests, int>                                                      passThrough1b{this, &RouteTests::handlePassThrough1b};
    RouteSink<RouteTests, int, bool&, NonCopyableThing const&, Canary, std::vector<int>&&> sink5{this, &RouteTests::handleSink5x};

    MOCK_METHOD0(handleSink0, void());
    MOCK_METHOD1(handleSink1, void(int));
    MOCK_METHOD1(handlePassThrough1a, bool(int));
    MOCK_METHOD1(handlePassThrough1b, bool(int));
    MOCK_METHOD4(handleSink5, void(int, bool&, NonCopyableThing const &, Canary));

    void handleSink5x(int a, bool& b, NonCopyableThing const& c, Canary d, std::vector<int>&& e)
    {
        // This extra function exists just to handle the r-value reference.
        // The move cannot be done properly in a mocked method.
        std::vector<int> x = std::move(e);
        handleSink5(a, b, c, d);
    }
};


TEST_F(RouteTests, SinkPassesArgsAsDefined)
{
    EXPECT_CALL(*this, handleSink0())
        .WillOnce(Return());
    sink0();

    bool flag = false;
    NonCopyableThing thing;
    Canary canary;
    canary.bear();
    std::vector<int> data{1, 2, 3};

    EXPECT_CALL(*this, handleSink5(123, _, Ref(thing), Eq("alive")))
        .WillOnce(DoAll(
        SetArgReferee<1>(true),
        InvokeArgument<3>()
        ));
    sink5(123, flag, thing, canary, std::move(data));

    EXPECT_THAT(flag, true);
    EXPECT_THAT(canary, Eq("alive"));
    EXPECT_THAT(data, IsEmpty());
}

TEST_F(RouteTests, SourceIsNoopWhenUnbound)
{
    RouteSource<int> test;
    test(1);
}

TEST_F(RouteTests, SourceCallsSink)
{
    RouteSource<> source0;
    source0 >> sink0;

    RouteSource<int, bool&, NonCopyableThing const&, Canary, std::vector<int>&&> source5;
    source5 >> sink5;


    EXPECT_CALL(*this, handleSink0())
        .WillOnce(Return());
    source0();


    bool flag = false;
    NonCopyableThing thing;
    Canary canary;
    canary.bear();
    std::vector<int> data{1, 2, 3};

    EXPECT_CALL(*this, handleSink5(123, _, Ref(thing), Eq("alive")))
        .WillOnce(DoAll(
        SetArgReferee<1>(true),
        InvokeArgument<3>()
        ));
    source5(123, flag, thing, canary, std::move(data));
    EXPECT_THAT(flag, true);
    EXPECT_THAT(canary, Eq("alive"));
    EXPECT_THAT(data, IsEmpty());
}

TEST_F(RouteTests, PassThroughsAreInvokedInBetween)
{
    RouteSource<int> source1;
    source1 >> passThrough1a >> passThrough1b >> sink1;

    InSequence order;
    EXPECT_CALL(*this, handlePassThrough1a(123))
        .WillOnce(Return(true));
    EXPECT_CALL(*this, handlePassThrough1b(123))
        .WillOnce(Return(true));
    EXPECT_CALL(*this, handleSink1(123))
        .WillOnce(Return());
    source1(123);
}

TEST_F(RouteTests, PassThroughsWithoutSinkAreOk)
{
    RouteSource<int> source1;
    source1 >> passThrough1a >> passThrough1b;

    InSequence order;
    EXPECT_CALL(*this, handlePassThrough1a(123))
        .WillOnce(Return(true));
    EXPECT_CALL(*this, handlePassThrough1b(123))
        .WillOnce(Return(true));
    source1(123);
}

TEST_F(RouteTests, PassThroughCanStopTheFlow)
{
    RouteSource<int> source1;
    source1 >> passThrough1a >> passThrough1b >> sink1;

    InSequence order;
    EXPECT_CALL(*this, handlePassThrough1a(123))
        .WillOnce(Return(false));
    source1(123);
}

TEST_F(RouteTests, RoutesCanCoexistsAcrossTheSamePath)
{
    RouteSource<int> sourceA;
    RouteSource<int> sourceB;
    sourceA >> passThrough1a >> passThrough1b >> sink1;
    sourceB >> passThrough1b >> passThrough1a >> sink1;

    InSequence order;

    EXPECT_CALL(*this, handlePassThrough1b(123))
        .WillOnce(Return(true));
    EXPECT_CALL(*this, handlePassThrough1a(123))
        .WillOnce(Return(true));
    EXPECT_CALL(*this, handleSink1(123))
        .WillOnce(Return());
    sourceB(123);

    EXPECT_CALL(*this, handlePassThrough1a(123))
        .WillOnce(Return(true));
    EXPECT_CALL(*this, handlePassThrough1b(123))
        .WillOnce(Return(true));
    EXPECT_CALL(*this, handleSink1(123))
        .WillOnce(Return());
    sourceA(123);
}

