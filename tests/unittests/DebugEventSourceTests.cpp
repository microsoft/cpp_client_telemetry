//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include <DebugEvents.hpp>
#include <functional>

using namespace testing;
using namespace MAT;

class TestDebugEventSource : public DebugEventSource
{
public:
   using DebugEventSource::listeners;
   using DebugEventSource::cascaded;
   using DebugEventSource::seq;
};

class TestDebugEventListener : public DebugEventListener
{
public:
   std::function<void(DebugEvent&)> OnDebugEventOverride;
   virtual void OnDebugEvent(DebugEvent& debugEvent) override
   {
      if (OnDebugEventOverride)
         OnDebugEventOverride(debugEvent);
   }
};

TEST(DebugEventSourceTests, Constructor_ZeroListeners)
{
   TestDebugEventSource source;
   ASSERT_EQ(source.listeners.size(), size_t { 0 });
}

TEST(DebugEventSourceTests, Constructor_SeqZero)
{
   TestDebugEventSource source;
   ASSERT_EQ(source.seq, size_t { 0 });
}

TEST(DebugEventSourceTests, Constructor_ZeroCascaded)
{
   TestDebugEventSource source;
   ASSERT_EQ(source.cascaded.size(), size_t { 0 });
}

TEST(DebugEventSourceTests, AddEventListener_OneListener_ListenerTypeCountOneAndListenerCountOne)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   source.AddEventListener(EVT_LOG_EVENT, listener);
   ASSERT_EQ(source.listeners.size(), size_t { 1 });
   ASSERT_EQ(source.listeners[EVT_LOG_EVENT].size(), size_t { 1 });
}

TEST(DebugEventSourceTests, AddEventListener_TwoListenersSameEventType_ListenerTypeCountOneAndListenerCountTwo)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   TestDebugEventListener secondListener;
   source.AddEventListener(EVT_LOG_EVENT, listener);
   source.AddEventListener(EVT_LOG_EVENT, secondListener);
   ASSERT_EQ(source.listeners.size(), size_t { 1 });
   ASSERT_EQ(source.listeners[EVT_LOG_EVENT].size(), size_t { 2 });
}

TEST(DebugEventSourceTests, AddEventListener_TwoListenersDifferentEventType_ListenerTypeCountTwoAndListenerCountOne)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   TestDebugEventListener secondListener;
   source.AddEventListener(EVT_LOG_EVENT, listener);
   source.AddEventListener(EVT_LOG_LIFECYCLE, secondListener);
   ASSERT_EQ(source.listeners.size(), size_t { 2 });
   ASSERT_EQ(source.listeners[EVT_LOG_EVENT].size(), size_t { 1 });
   ASSERT_EQ(source.listeners[EVT_LOG_LIFECYCLE].size(), size_t { 1 });
}

TEST(DebugEventSourceTests, RemoveEventListener_NoRegisteredListeners_DoesNothing)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   source.RemoveEventListener(EVT_LOG_EVENT, listener);
   ASSERT_EQ(source.listeners.size(), size_t { 0 });
}

TEST(DebugEventSourceTests, RemoveEventListener_OneRegisteredListener_RemovesListener)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   source.AddEventListener(EVT_LOG_EVENT, listener);
   source.RemoveEventListener(EVT_LOG_EVENT, listener);
   ASSERT_EQ(source.listeners[EVT_LOG_EVENT].size(), size_t { 0 });
}

TEST(DebugEventSourceTests, RemoveEventListener_TwoListenersOfTheSameTypeRemoveFirst_OneListenerLeft)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   TestDebugEventListener secondListener;
   source.AddEventListener(EVT_LOG_EVENT, listener);
   source.AddEventListener(EVT_LOG_EVENT, secondListener);
   source.RemoveEventListener(EVT_LOG_EVENT, listener);
   ASSERT_EQ(source.listeners[EVT_LOG_EVENT].size(), size_t { 1 });
   ASSERT_EQ(source.listeners[EVT_LOG_EVENT][0], &secondListener);
}

TEST(DebugEventSourceTests, RemoveEventListener_SameTwoListenersRemoveOne_RemovesAllCopies)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   source.AddEventListener(EVT_LOG_EVENT, listener);
   source.AddEventListener(EVT_LOG_EVENT, listener);
   source.RemoveEventListener(EVT_LOG_EVENT, listener);
   ASSERT_EQ(source.listeners[EVT_LOG_EVENT].size(), size_t { 0 });
}

TEST(DebugEventSourceTests, AttachEventSource_SourceIsThis_ReturnsFalseAndCascadedSizeRemainsZero)
{
   TestDebugEventSource source;
   EXPECT_FALSE(source.AttachEventSource(source));
   ASSERT_EQ(source.cascaded.size(), uint64_t { 0 });
}

TEST(DebugEventSourceTests, AttachEventSource_OneSource_CascadedSizeIsOne)
{
   TestDebugEventSource source;
   TestDebugEventSource anotherSource;
   EXPECT_TRUE(source.AttachEventSource(anotherSource));
   ASSERT_EQ(source.cascaded.size(), uint64_t { 1 });
}

TEST(DebugEventSourceTests, DetachEventSource_NoRegisteredSources_ReturnsFalse)
{
   TestDebugEventSource source;
   EXPECT_FALSE(source.DetachEventSource(source));
}

TEST(DebugEventSourceTests, DetachEventSource_OneRegisteredSource_RemovesThatSource)
{
   TestDebugEventSource source;
   TestDebugEventSource anotherSource;
   EXPECT_TRUE(source.AttachEventSource(anotherSource));
   EXPECT_TRUE(source.DetachEventSource(anotherSource));
   ASSERT_EQ(source.cascaded.size(), uint64_t { 0 });
}

TEST(DebugEventSourceTests, DispatchEvent_NoListenersOrCascaded_ReturnsFalse)
{
   TestDebugEventSource source;
   EXPECT_FALSE(source.DispatchEvent(DebugEvent { EVT_LOG_EVENT }));
}

TEST(DebugEventSourceTests, DispatchEvent_OneListener_ReturnsTrue)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   source.AddEventListener(EVT_LOG_EVENT, listener);

   EXPECT_TRUE(source.DispatchEvent(DebugEvent { EVT_LOG_EVENT }));
}

TEST(DebugEventSourceTests, DispatchEvent_OneEventSameAsListenerType_IncrementsSequenceNumber)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   uint64_t sequence {};
   listener.OnDebugEventOverride = [&sequence](DebugEvent& debugEvent) noexcept { sequence = debugEvent.seq; };
   source.AddEventListener(EVT_LOG_EVENT, listener);

   source.DispatchEvent(DebugEvent { EVT_LOG_EVENT });
   ASSERT_EQ(sequence, uint64_t { 1 });
}

TEST(DebugEventSourceTests, DispatchEvent_TwoEventsOneSameAsListenerType_ListenerSeesOneEvent)
{
   TestDebugEventSource source;
   TestDebugEventListener listener;
   uint64_t countOfEventsSeen {};
   listener.OnDebugEventOverride = [&countOfEventsSeen](DebugEvent&) noexcept { countOfEventsSeen++; };
   source.AddEventListener(EVT_LOG_EVENT, listener);

   source.DispatchEvent(DebugEvent { EVT_LOG_EVENT });
   source.DispatchEvent(DebugEvent { EVT_LOG_LIFECYCLE });
   ASSERT_EQ(countOfEventsSeen, uint64_t { 1 });
}

TEST(DebugEventSourceTests, DispatchEvent_OneEventToCascaded_ListenerSeesOneEvent)
{
   TestDebugEventSource source;
   TestDebugEventSource anotherSource;
   TestDebugEventListener listener;
   uint64_t countOfEventsSeen {};
   listener.OnDebugEventOverride = [&countOfEventsSeen](DebugEvent&) noexcept { countOfEventsSeen++; };
   anotherSource.AddEventListener(EVT_LOG_EVENT, listener);
   source.AttachEventSource(anotherSource);

   source.DispatchEvent(DebugEvent { EVT_LOG_EVENT });
   ASSERT_EQ(countOfEventsSeen, uint64_t { 1 });
}

TEST(DebugEventSourceTests, DispatchEvent_OneEventToCascadedAndToSource_ListenerSeesSameEventTwice)
{
   TestDebugEventSource source;
   TestDebugEventSource anotherSource;
   TestDebugEventListener listener;
   std::map<uint64_t, uint64_t> sequenceNumberToCountMap;
   listener.OnDebugEventOverride = [&sequenceNumberToCountMap](DebugEvent& debugEvent) noexcept { sequenceNumberToCountMap[debugEvent.seq]++; };
   anotherSource.AddEventListener(EVT_LOG_EVENT, listener);
   source.AttachEventSource(anotherSource);
   source.AddEventListener(EVT_LOG_EVENT, listener);

   source.DispatchEvent(DebugEvent { EVT_LOG_EVENT });
   ASSERT_EQ(sequenceNumberToCountMap[1], uint64_t { 2 });
}


