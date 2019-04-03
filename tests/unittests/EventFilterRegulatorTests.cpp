#ifdef USE_EVENT_FILTER
// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "filter/EventFilterRegulator.hpp"
#include "common/MockIEventFilter.hpp"
#include <vector>
#include <mutex>

using namespace testing;
using namespace MAT;

// This abomination is only here because I couldn't get a lambda to return a useful mock!
class MockableFactory
{
	static uint32_t                     _nextIndex;
	static std::vector<PIEventFilter>   _filters;
public:

	static void SetFilters(std::vector<PIEventFilter> filters)
	{
		_filters = filters;
		_nextIndex = 0;
	}
	static PIEventFilter NextFilter()
	{
		return _filters[_nextIndex++];
	}
	static void Clear()
	{
		_filters.clear();
		_nextIndex = 0;
	}
};

// This lets us confirm deletion, while still leveraging the Mock framework
class ProxyMockEventFilter : public IEventFilter
{
private:
	MockIEventFilter * _eventFilter;

public:
	ProxyMockEventFilter(MockIEventFilter* eventFilter)
		: _eventFilter(eventFilter)
	{
	}

	~ProxyMockEventFilter()
	{
		_eventFilter->SimulateDelete();
		_eventFilter = nullptr;
	}

	virtual int32_t SetExclusionFilter(const char** filterStrings, uint32_t filterCount) override
	{
		return _eventFilter->SetExclusionFilter(filterStrings, filterCount);
	}

	virtual int32_t SetExclusionFilter(const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount) override
	{
		return _eventFilter->SetExclusionFilter(filterStrings, filterRates, filterCount);
	}

	virtual bool IsEventExcluded(const std::string& eventName) override
	{
		return _eventFilter->IsEventExcluded(eventName);
	}
};

//std::vector<PIEventFilter> MockableFactory::_filters;
//uint32_t MockableFactory::_nextIndex = 0;

static void Fail()
{
	// ASSERT_EQ includes a return method, so we wrap it in its own method
	ASSERT_EQ("EventFilterFactoryShouldNotBeCalled", "EventFilterFactoryWasCalled");
}

static PIEventFilter InertEventFilterFactory()
{
	Fail();
	return (PIEventFilter)nullptr;
}

TEST(EventFilterRegulatorTests, ConstructAndDestructWithNoFilterDoesNothing)
{
	EventFilterRegulator regulator(InertEventFilterFactory);
}

TEST(EventFilterRegulatorTests, ConstructResetAndDestructWithNoFilterDoesNothing)
{
	EventFilterRegulator regulator(InertEventFilterFactory);
	regulator.Reset();
}

TEST(EventFilterRegulatorTests, GetTenantFilterIdenticalCaseConstructsAndReturnsOneFilter)
{
	StrictMock<MockIEventFilter> filter;
	PIEventFilter pFilter = new ProxyMockEventFilter(&filter);

	std::vector<PIEventFilter> filters;
	filters.push_back(pFilter);
	MockableFactory::SetFilters(filters);
	EventFilterRegulator* regulator = new EventFilterRegulator(MockableFactory::NextFilter);
	ASSERT_EQ(&regulator->GetTenantFilter("token"), pFilter);
	ASSERT_EQ(&regulator->GetTenantFilter("token"), pFilter);
	ASSERT_EQ(&regulator->GetTenantFilter("token"), pFilter);
	ASSERT_EQ(&regulator->GetTenantFilter("token"), pFilter);
	ASSERT_EQ(0, filter.GetSimulatedDeleteCount());
	delete regulator;
	ASSERT_EQ(1, filter.GetSimulatedDeleteCount());
}

TEST(EventFilterRegulatorTests, GetTenantFilterDifferentCaseConstructsAndReturnsDifferentOneFilterPerCase)
{
	StrictMock<MockIEventFilter> filter;
	PIEventFilter pFilter1 = new ProxyMockEventFilter(&filter);
	PIEventFilter pFilter2 = new ProxyMockEventFilter(&filter);

	std::vector<PIEventFilter> filters;
	filters.push_back(pFilter1);
	filters.push_back(pFilter2);
	MockableFactory::SetFilters(filters);
	EventFilterRegulator* regulator = new EventFilterRegulator(MockableFactory::NextFilter);
	ASSERT_EQ(&regulator->GetTenantFilter("TOKEN"), pFilter1);
	ASSERT_EQ(&regulator->GetTenantFilter("ToKeN"), pFilter2);
	ASSERT_EQ(&regulator->GetTenantFilter("ToKeN"), pFilter2);
	ASSERT_EQ(&regulator->GetTenantFilter("TOKEN"), pFilter1);
	ASSERT_EQ(0, filter.GetSimulatedDeleteCount());
	delete regulator;
	ASSERT_EQ(2, filter.GetSimulatedDeleteCount());
}

TEST(EventFilterRegulatorTests, ConstructSetInvalidFilterAndDestructDoesNothing)
{
	EventFilterRegulator regulator(InertEventFilterFactory);
	ASSERT_EQ(regulator.SetExclusionFilter(nullptr, nullptr, 0), SetExclusionFilterResult::ErrorBadInput);
}

TEST(EventFilterRegulatorTests, CallingSetExclusionFilterOnceCreatesOneFilterAndUsesItOnce)
{
	std::vector<const char*> strings;
	strings.push_back("a string");

	const char** filterStrings = strings.data();
	uint32_t filterCount = static_cast<uint32_t>(strings.size());

	StrictMock<MockIEventFilter> filter;
	PIEventFilter pFilter = new ProxyMockEventFilter(&filter);
	EXPECT_CALL(filter, SetExclusionFilter(filterStrings, filterCount))
		.WillOnce(Return(SetExclusionFilterResult::Success));

	std::vector<PIEventFilter> filters;
	filters.push_back(pFilter);
	MockableFactory::SetFilters(filters);
	EventFilterRegulator* regulator = new EventFilterRegulator(MockableFactory::NextFilter);
	ASSERT_EQ(regulator->SetExclusionFilter("token", filterStrings, filterCount), SetExclusionFilterResult::Success);
	MockableFactory::Clear();

	ASSERT_EQ(0, filter.GetSimulatedDeleteCount());
	delete regulator;
	ASSERT_EQ(1, filter.GetSimulatedDeleteCount());
}

TEST(EventFilterRegulatorTests, CallingSetExclusionFilterTwiceCreatesOneFilterAndUsesItTwice)
{
	std::vector<const char*> strings;
	strings.push_back("a string");

	const char** filterStrings = strings.data();
	uint32_t filterCount = static_cast<uint32_t>(strings.size());

	StrictMock<MockIEventFilter> filter;
	PIEventFilter pFilter = new ProxyMockEventFilter(&filter);
	EXPECT_CALL(filter, SetExclusionFilter(filterStrings, filterCount))
		.WillOnce(Return(SetExclusionFilterResult::Success))
		.WillOnce(Return(SetExclusionFilterResult::Success));

	std::vector<PIEventFilter> filters;
	filters.push_back(pFilter);
	MockableFactory::SetFilters(filters);
	EventFilterRegulator* regulator = new EventFilterRegulator(MockableFactory::NextFilter);
	ASSERT_EQ(regulator->SetExclusionFilter("token", filterStrings, filterCount), SetExclusionFilterResult::Success);
	ASSERT_EQ(regulator->SetExclusionFilter("token", filterStrings, filterCount), SetExclusionFilterResult::Success);
	ASSERT_EQ(0, filter.GetSimulatedDeleteCount());
	delete regulator;
	ASSERT_EQ(1, filter.GetSimulatedDeleteCount());
}

TEST(EventFilterRegulatorTests, CallingSetExclusionFilterTwiceCreatesTwoFilterAndUsesEachOneTwice)
{
	std::vector<const char*> strings;
	strings.push_back("a string");

	const char** filterStrings = strings.data();
	uint32_t filterCount = static_cast<uint32_t>(strings.size());

	StrictMock<MockIEventFilter> filter1;
	PIEventFilter pFilter1 = new ProxyMockEventFilter(&filter1);
	EXPECT_CALL(filter1, SetExclusionFilter(filterStrings, filterCount))
		.WillOnce(Return(SetExclusionFilterResult::Success))
		.WillOnce(Return(SetExclusionFilterResult::Success));

	StrictMock<MockIEventFilter> filter2;
	PIEventFilter pFilter2 = new ProxyMockEventFilter(&filter2);
	EXPECT_CALL(filter2, SetExclusionFilter(filterStrings, filterCount))
		.WillOnce(Return(SetExclusionFilterResult::Success))
		.WillOnce(Return(SetExclusionFilterResult::Success));

	std::vector<PIEventFilter> filters;
	filters.push_back(pFilter1);
	filters.push_back(pFilter2);
	MockableFactory::SetFilters(filters);
	EventFilterRegulator* regulator = new EventFilterRegulator(MockableFactory::NextFilter);
	ASSERT_EQ(regulator->SetExclusionFilter("token1", filterStrings, filterCount), SetExclusionFilterResult::Success);
	ASSERT_EQ(regulator->SetExclusionFilter("token2", filterStrings, filterCount), SetExclusionFilterResult::Success);
	ASSERT_EQ(regulator->SetExclusionFilter("token1", filterStrings, filterCount), SetExclusionFilterResult::Success);
	ASSERT_EQ(regulator->SetExclusionFilter("token2", filterStrings, filterCount), SetExclusionFilterResult::Success);
	ASSERT_EQ(0, filter1.GetSimulatedDeleteCount());
	ASSERT_EQ(0, filter2.GetSimulatedDeleteCount());
	delete regulator;
	ASSERT_EQ(1, filter1.GetSimulatedDeleteCount());
	ASSERT_EQ(1, filter2.GetSimulatedDeleteCount());
}
#endif
