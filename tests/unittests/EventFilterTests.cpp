#ifdef USE_EVENT_FILTER
// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "filter/EventFilter.hpp"
#include <vector>

using namespace testing;
using namespace MAT;

static const char* event1 = "MyNamespace.MyEvent1";
static const char* event2 = "MyNamespace.MyEvent2";
static const char* event3 = "MyOtherNamespace.MyEvent1";

TEST(EventFilterTests, DefaultBlocksNoEvents)
{
	EventFilter filter;

	EXPECT_FALSE(filter.IsEventExcluded(event1));
	EXPECT_FALSE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));
}

TEST(EventFilterTests, SetExclusionFilterChangesFilters)
{
	EventFilter filter;
	std::vector<const char*> exclusionList;

	// Default status -- nothing filtered
	EXPECT_FALSE(filter.IsEventExcluded(event1));
	EXPECT_FALSE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));

	// Exclude event1
	exclusionList.push_back(event1);
	EXPECT_TRUE(filter.SetExclusionFilter(exclusionList.data(), static_cast<uint32_t>(exclusionList.size())) > 0);
	EXPECT_TRUE(filter.IsEventExcluded(event1));
	EXPECT_FALSE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));

	// This is invalid and should get ignored
	EXPECT_TRUE(filter.SetExclusionFilter(nullptr, static_cast<uint32_t>(exclusionList.size())) < 0);
	EXPECT_TRUE(filter.IsEventExcluded(event1));
	EXPECT_FALSE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));

	// Exclude event2
	exclusionList[0] = event2;
	EXPECT_TRUE(filter.SetExclusionFilter(exclusionList.data(), static_cast<uint32_t>(exclusionList.size())) > 0);
	EXPECT_FALSE(filter.IsEventExcluded(event1));
	EXPECT_TRUE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));
	
	/* TODO: wildcard matching
	// Exclude both namespaces
	exclusionList[0] = "MyNamespace.*";
	exclusionList.push_back("MyOtherNamespace.*");
	EXPECT_TRUE(filter.SetExclusionFilter(exclusionList.data(), static_cast<uint32_t>(exclusionList.size())) > 0);
	EXPECT_TRUE(filter.IsEventExcluded(event1));
	EXPECT_TRUE(filter.IsEventExcluded(event2));
	EXPECT_TRUE(filter.IsEventExcluded(event3));
	*/

	// Reset to the default
	EXPECT_TRUE(filter.SetExclusionFilter(nullptr, 0) > 0);
	EXPECT_FALSE(filter.IsEventExcluded(event1));
	EXPECT_FALSE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));

	// Exclude event1 and event3, but include a null filter, just to be mean
	exclusionList.clear();
	exclusionList.push_back(nullptr);
	exclusionList.push_back(event3);
	exclusionList.push_back(event1);
	EXPECT_TRUE(filter.SetExclusionFilter(exclusionList.data(), static_cast<uint32_t>(exclusionList.size())) > 0);
	EXPECT_TRUE(filter.IsEventExcluded(event1));
	EXPECT_FALSE(filter.IsEventExcluded(event2));
	EXPECT_TRUE(filter.IsEventExcluded(event3));
}

TEST(EventFilterTests, ExactMatchBlocksEvent1)
{
	EventFilter filter;

	std::vector<const char*> exclusionList;
	exclusionList.push_back(event1);

	filter.SetExclusionFilter(exclusionList.data(), static_cast<uint32_t>(exclusionList.size()));

	EXPECT_TRUE(filter.IsEventExcluded(event1));
	EXPECT_FALSE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));
}

/* TODO: wildcard matching
TEST(EventFilterTests, WildcardMatchBlocksEvents1And2)
{
	EventFilter filter;

	std::string filterString(event1);
	*rbegin(filterString) = '*';

	std::vector<const char*> exclusionList;
	exclusionList.push_back(filterString.c_str());

	filter.SetExclusionFilter(exclusionList.data(), static_cast<uint32_t>(exclusionList.size()));

	EXPECT_TRUE(filter.IsEventExcluded(event1));
	EXPECT_TRUE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));
}

TEST(EventFilterTests, BadMatchBlocksNoEvents)
{
	EventFilter filter;

	std::vector<const char*> exclusionList;
	exclusionList.push_back("MyEvent*");

	filter.SetExclusionFilter(exclusionList.data(), static_cast<uint32_t>(exclusionList.size()));

	EXPECT_FALSE(filter.IsEventExcluded(event1));
	EXPECT_FALSE(filter.IsEventExcluded(event2));
	EXPECT_FALSE(filter.IsEventExcluded(event3));
}
*/
#endif
