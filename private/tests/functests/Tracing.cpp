#if 0
// FIXME: [MG] - enable tracing API

bool TracingAPI_File(const char *filename, ACTTraceLevel verbosity = ACTTraceLevel_Trace, size_t traceFileSize = 30000000, size_t evtCount = 1)
{
    bool result = false;
    // Remove old logging file
    std::remove(filename);
    LogConfiguration config;
    // Log something
    config.traceLevelMask = 0xFFFFFFFF;
    config.minimumTraceLevel = verbosity;
    config.SetProperty(CFG_INT_DBG_TRACE_PROVIDER, "1");

    // Property const char* must remain constant for the duration of the run.
    // Most common pitfall is to pass a const char * to temporary that goes
    // out of scope ... what happens next is that the parser would attempt
    // to parse an invalid memory block!
    std::string size = std::to_string(traceFileSize);
    config.SetProperty(CFG_INT_DBG_TRACE_SIZE, size.c_str());

    if (strlen(filename))
        config.SetProperty(CFG_STR_DBG_TRACE_PATH, filename);
    LogManager::Initialize(TEST_TOKEN, config);
    while (evtCount--)
        LogManager::GetLogger()->LogEvent("TracingAPI");
    LogManager::FlushAndTeardown();
    // Check if default log file has been created
    if (strlen(filename))
        result = common::FileExists(filename);
    else
        result = common::FileExists(PAL::GetDefaultTracePath().c_str());
    return result;
}

TEST(APITest, TracingAPI_DefaultFile)
{
    EXPECT_EQ(TracingAPI_File(""), true);
}

TEST(APITest, TracingAPI_CustomFile)
{
    EXPECT_EQ(TracingAPI_File("mydebug.log"), true);
}

/// <summary>
/// Usage example for EVT_TRACE
/// </summary>
class MyTraceListener : public DebugEventListener {
    std::atomic<unsigned> evtCount;

public:
    MyTraceListener() :
        DebugEventListener(),
        evtCount(0) {};

    // Inherited via DebugEventListener
    virtual void OnDebugEvent(DebugEvent & evt) override
    {
        evtCount++;
        printf("[%d] %s:%d\n", evt.param1, (const char *)evt.data, evt.param2);
    }

    virtual unsigned getEvtCount()
    {
        return evtCount.load();
    }
};

MyTraceListener traceListener;

TEST(APITest, TracingAPI_Callback)
{
    LogConfiguration config;
    config.traceLevelMask = 0xFFFFFFFF;
    config.minimumTraceLevel = ACTTraceLevel_Trace;
    config.SetProperty(CFG_INT_DBG_TRACE_PROVIDER, "1");
    config.SetProperty(CFG_INT_DBG_TRACE_SIZE, "30000000");

    // This event listener is goign to print an error (filename:linenumber) whenever
    // SDK internally experiences any error, warning or fatal event
    LogManager::AddEventListener(EVT_TRACE, traceListener);
    LogManager::Initialize("invalid-token", config);    // We don't check tokens for validity at init ...
    LogManager::GetLogger()->LogEvent("going-nowhere"); // ... but we do check event names.
    LogManager::FlushAndTeardown();
    LogManager::RemoveEventListener(EVT_TRACE, traceListener);

    // Expecting at least one error notification.
    // Invalid event name may trigger several errors at various stages of the pipeline.
    EXPECT_GE(traceListener.getEvtCount(), 1);
}

TEST(APITest, TracingAPI_Verbosity)
{
    // Nothing should be logged - filesize must remain zero
    TracingAPI_File("", ACTTraceLevel_Fatal);
    EXPECT_EQ(common::GetFileSize(PAL::GetDefaultTracePath().c_str()), 0);
    // Default file gets deleted on Release bits on shutdown
}

TEST(APITest, TracingAPI_FileSizeLimit)
{
    // Log file size must remain within 1MB range + 512K contingency buffer
    size_t maxFileSize = 1024000;
    size_t maxContingency = 512000;
    size_t maxEventCount = 50000; // 50K events is sufficient to attempt an overflow on file size
    TracingAPI_File("", ACTTraceLevel_Debug, maxFileSize, maxEventCount);
    EXPECT_LE(common::GetFileSize(PAL::GetDefaultTracePath().c_str()), maxFileSize + maxContingency);
}

#endif
