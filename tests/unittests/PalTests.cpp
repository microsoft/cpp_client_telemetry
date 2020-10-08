#if 0
//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "common/MockISemanticContext.hpp"

using namespace testing;

class PalTests : public Test {};

//---

TEST_F(PalTests, Logging)
{
    int i = 123;
    char const* s = "abc";

    LOG_TRACE("Detail: %d != %s", i, s);
    LOG_INFO("Info: %d != %s", i, s);
    LOG_WARN("Warning: %d != %s", i, s);
    LOG_ERROR("Error: %d != %s", i, s);
}

//---

class IOne
{
  public:
    virtual int secret() const = 0;
};

class ITwo
{
  public:
    virtual void setContent(std::string const& content) = 0;
    virtual const std::string& getContent() const = 0;
};

class IThree
{
  public:
    virtual long process() = 0;
};

class Implementation : public IOne, public ITwo, public IThree
{
  protected:
    int m_secret;
    std::string m_content;
    static int s_constructions, s_instances;

  public:
    int m_public;

  public:
    Implementation()
      : m_secret(0),
        m_content("empty")
    {
        s_constructions++;
        s_instances++;
    }

    Implementation(bool& reference)
      : m_secret(666),
        m_content("reference")
    {
        s_constructions++;
        s_instances++;
        reference = true;
    }

    Implementation(int secret, std::string const& content = "default")
      : m_secret(secret),
        m_content(content)
    {
        s_constructions++;
        s_instances++;
    }

    ~Implementation()
    {
        s_instances--;
    }

    static int constructions() { int result = s_constructions; s_constructions = 0; return result; }
    static int instances() { return s_instances; }

    virtual int secret() const override { return m_secret; }

    virtual void setContent(std::string const& content) override { m_content = content; }
    virtual const std::string& getContent() const override { return m_content; }

    virtual long process() { static long s_value = 0; return ++s_value; }
};

int Implementation::s_constructions = 0;
int Implementation::s_instances = 0;

TEST_F(PalTests, RefCountedCreateForwardsArguments)
{
    ASSERT_THAT(Implementation::constructions(), 0);
    ASSERT_THAT(Implementation::instances(), 0);

    auto empty = Implementation::create();
    EXPECT_THAT(empty->secret(), 0);
    EXPECT_THAT(empty->getContent(), Eq("empty"));
    long base = empty->process();

    bool reference = false;
    auto oneArgWithRef = Implementation::create(reference);
    EXPECT_THAT(reference, true);
    EXPECT_THAT(oneArgWithRef->secret(), 666);
    EXPECT_THAT(oneArgWithRef->getContent(), Eq("reference"));
    EXPECT_THAT(oneArgWithRef->process(), base + 1);

    auto oneArgSecondDefault = Implementation::create(42);
    EXPECT_THAT(oneArgSecondDefault->secret(), 42);
    EXPECT_THAT(oneArgSecondDefault->getContent(), Eq("default"));
    EXPECT_THAT(oneArgSecondDefault->process(), base + 2);

    auto twoArgs = Implementation::create(43, "custom");
    EXPECT_THAT(twoArgs->secret(), 43);
    EXPECT_THAT(twoArgs->getContent(), Eq("custom"));
    EXPECT_THAT(twoArgs->process(), base + 3);

    EXPECT_THAT(Implementation::constructions(), 4);
    EXPECT_THAT(Implementation::instances(), 4);
}

TEST_F(PalTests, RefCountedPtrConstruction)
{
    ASSERT_THAT(Implementation::constructions(), 0);
    ASSERT_THAT(Implementation::instances(), 0);

    {
        PAL::RefCountedPtr<Implementation> ptr(Implementation::create());

        PAL::RefCountedPtr<Implementation> copyConstr(ptr);
        EXPECT_THAT(copyConstr.get(), ptr.get());
        EXPECT_THAT(copyConstr, ptr);

        PAL::RefCountedPtr<Implementation> assignOp;
        assignOp = ptr;
        EXPECT_THAT(assignOp.get(), ptr.get());
        EXPECT_THAT(assignOp, ptr);

        PAL::RefCountedPtr<IOne> ifaceCopyConstr(ptr);
        EXPECT_THAT(ifaceCopyConstr.get(), ptr.get());
        EXPECT_THAT(ifaceCopyConstr, ptr);

        PAL::RefCountedPtr<IOne> ifaceAssignOp;
        ifaceAssignOp = ptr;
        EXPECT_THAT(ifaceAssignOp.get(), ptr.get());
        EXPECT_THAT(ifaceAssignOp, ptr);

        ITwo* plain = ptr.get();
        EXPECT_THAT(plain, ptr.get());

        PAL::RefCountedPtr<ITwo> plainConstr(plain, true);
        EXPECT_THAT(plainConstr.get(), static_cast<ITwo*>(ptr.get()));
        EXPECT_THAT(plainConstr, ptr);

        PAL::RefCountedPtr<ITwo> plainAssignOp;
        // plainAssignOp = plain; - does not work for safety reasons
        plainAssignOp = PAL::RefCountedPtr<ITwo>(plain, true);
        EXPECT_THAT(plainAssignOp.get(), static_cast<ITwo*>(ptr.get()));
        EXPECT_THAT(plainAssignOp, ptr);

        EXPECT_THAT(Implementation::instances(), 1);
    }

    EXPECT_THAT(Implementation::constructions(), 1);
    EXPECT_THAT(Implementation::instances(), 0);
}

TEST_F(PalTests, RefCountedOperators)
{
    ASSERT_THAT(Implementation::constructions(), 0);
    ASSERT_THAT(Implementation::instances(), 0);

    PAL::RefCountedPtr<Implementation> impl(Implementation::create());
    EXPECT_THAT(Implementation::instances(), 1);

    EXPECT_THAT(&*impl, impl.get());
    EXPECT_THAT(&impl->m_public, &(*impl).m_public);
    EXPECT_THAT(!!impl, true);
    EXPECT_THAT(!impl, false);

    {
        PAL::RefCountedPtr<Implementation> implCopy(impl);
        PAL::RefCountedPtr<IOne> oneCopy(impl);
        PAL::RefCountedPtr<Implementation> other;

        EXPECT_THAT(impl == impl,     true);
        EXPECT_THAT(implCopy == impl, true);
        EXPECT_THAT(oneCopy == impl,  true);
        EXPECT_THAT(impl == other,    false);

        EXPECT_THAT(impl != impl,     false);
        EXPECT_THAT(implCopy != impl, false);
        EXPECT_THAT(oneCopy != impl,  false);
        EXPECT_THAT(impl != other,    true);
    }

    impl.reset();
    EXPECT_THAT(!!impl, false);
    EXPECT_THAT(!impl, true);

    EXPECT_THAT(Implementation::constructions(), 1);
    EXPECT_THAT(Implementation::instances(), 0);
}

//---

class Deferred : public PAL::RefCountedImpl<Deferred>
{
  public:
    volatile bool noArgsResult = false;
    volatile int threeArgsResult = 0;
    volatile int waitedMs = 0;

  public:
    Deferred()
    {
    }

    Deferred(Deferred const&) = delete;
    Deferred& operator=(Deferred const&) = delete;

    void noArgs()
    {
        noArgsResult = true;
    }

    void threeArgs(int a, bool b, std::string& c)
    {
        threeArgsResult = a + (b ? 20 : 0) + (c == "values are passed through" ? 30 : 0);
        c = "references are lost";
    }

    void wait(unsigned ms)
    {
        int before = waitedMs;
        PAL::sleep(ms);
        waitedMs = before + ms;
    }
};

TEST_F(PalTests, WorkerThreadCallsMethod)
{
    auto obj = Deferred::create();
    auto workerThread = PAL::WorkerThreadFactory::Create();

    EXPECT_THAT(obj->noArgsResult, false);
    PAL::dispatchTask(workerThread, obj, &Deferred::noArgs);
    PAL::sleep(300);
    EXPECT_THAT(obj->noArgsResult, true);

    EXPECT_THAT(obj->threeArgsResult, 0);
    std::string output("values are passed through");
    PAL::dispatchTask(workerThread, obj.get(), &Deferred::threeArgs, 10, true, output);
    PAL::sleep(300);
    EXPECT_THAT(obj->threeArgsResult, 60);
    EXPECT_THAT(output, Ne("references are lost"));

    delete workerThread;
}

TEST_F(PalTests, WorkerThreadCallsMethodWithDelay)
{
    auto obj = Deferred::create();
    auto workerThread = PAL::WorkerThreadFactory::Create();

    EXPECT_THAT(obj->noArgsResult, false);
    PAL::scheduleTask(workerThread, 400, obj, &Deferred::noArgs);

    EXPECT_THAT(obj->threeArgsResult, 0);
    std::string output("values are passed through");
    PAL::scheduleTask(workerThread, 800, obj.get(), &Deferred::threeArgs, 10, true, output);

    PAL::sleep(100);
    EXPECT_THAT(obj->noArgsResult, false);
    EXPECT_THAT(obj->threeArgsResult, 0);
    EXPECT_THAT(output, Eq("values are passed through"));

    PAL::sleep(400);
    EXPECT_THAT(obj->noArgsResult, true);
    EXPECT_THAT(obj->threeArgsResult, 0);
    EXPECT_THAT(output, Eq("values are passed through"));

    PAL::sleep(500);
    EXPECT_THAT(obj->noArgsResult, true);
    EXPECT_THAT(obj->threeArgsResult, 60);
    EXPECT_THAT(output, Ne("references are lost"));

    delete workerThread;
}

TEST_F(PalTests, WorkerThreadIsSerialized)
{
    auto obj = Deferred::create();
    auto workerThread = PAL::WorkerThreadFactory::Create();

    ASSERT_THAT(obj->waitedMs, 0);

    PAL::dispatchTask(workerThread, obj, &Deferred::wait, 500);
    PAL::scheduleTask(workerThread, 300, obj, &Deferred::wait, 100);

    PAL::sleep(100);

    PAL::dispatchTask(workerThread, obj, &Deferred::wait, 50);

    PAL::sleep(800);

    EXPECT_THAT(obj->waitedMs, 500 + 100 + 50
    auto workerThread = PAL::WorkerThreadFactory::Create(););
}

TEST_F(PalTests, WorkerThreadDeferredCallbackIsCancellable)
{
    auto obj = Deferred::create();
    auto workerThread = PAL::WorkerThreadFactory::Create();

    PAL::DeferredCallbackHandle call0 = PAL::scheduleTask(workerThread, 100, obj, &Deferred::wait, 1);
    ASSERT_THAT(!!call0, true);

    PAL::DeferredCallbackHandle call1 = PAL::scheduleTask(workerThread, 400, obj, &Deferred::wait, 2);
    ASSERT_THAT(!!call1, true);

    PAL::DeferredCallbackHandle call0copy;
    EXPECT_THAT(!!call0copy, false);
    call0copy = call0;
    EXPECT_THAT(!!call0copy, true);
    call0copy.reset();
    EXPECT_THAT(!!call0copy, false);
    EXPECT_THAT(!!call0, true);
    call0copy.cancel(); // Harmless

    PAL::DeferredCallbackHandle call1copy;
    EXPECT_THAT(!!call1copy, false);
    call1copy = call1;
    EXPECT_THAT(!!call1copy, true);
    call1.reset();
    EXPECT_THAT(!!call1, false);
    EXPECT_THAT(!!call1copy, true);

    PAL::sleep(200);
    EXPECT_THAT(!!call0, false);
    EXPECT_THAT(!!call1copy, true);

    call0.cancel(); // No-op, already ran
    call1copy.cancel();
    EXPECT_THAT(!!call0, false);
    EXPECT_THAT(!!call1copy, false);

    PAL::sleep(300);
    EXPECT_THAT(obj->waitedMs, 1);

    delete workerThread;
}

//---

TEST_F(PalTests, UuidGeneration)
{
    std::string uuid0 = PAL::generateUuidString();

    EXPECT_THAT(uuid0.length(), 36u);

    std::string mask = uuid0;
    for (char& ch : mask) {
        if (::isdigit(ch) || (::isupper(ch) && ::isxdigit(ch))) {
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
    EXPECT_THAT(v, HasSubstr("-C++-No-"));
    EXPECT_THAT(v, EndsWith(BUILD_VERSION_STR));

    EXPECT_THAT(PAL::getSdkVersion(), Eq(v));
}
#endif

