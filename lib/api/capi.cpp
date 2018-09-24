#ifdef _WIN32
#define ARIASDK_DECLSPEC __declspec(dllexport)
#endif

#include <LogManager.hpp>

namespace ARIASDK_NS_BEGIN {
    class ModuleCAPI : public ILogConfiguration {};
    class LogManagerC : public LogManagerBase<ModuleCAPI> {};
    DEFINE_LOGMANAGER(LogManagerC, ModuleCAPI);
} ARIASDK_NS_END;

using namespace MAT;

static std::atomic<bool> is_inited = false;

extern "C" {

    //
    // TODO: expose struct LogConfiguration as a second parameter
    // TODO: allow the customer to specify their module name
    //
    bool aria_initialize(const char* token)
    {
        if (!is_inited.exchange(true))
        {

            // Each pure C API caller module name starts with CAPI
            // with tenant token appended after dash.
            std::string moduleName = "CAPI";
            moduleName += "-";
            moduleName += token;

            // Obtain the current LogManagerC configuration.
            // Different customers going thru the pure C API
            // all get the same guest log manager for now
            auto& config = LogManagerC::GetLogConfiguration();
            config["name"] = moduleName;
            config["version"] = "1.0";
            config["config"] = { { "host", "*" } }; // Any host
            return (LogManagerC::Initialize(token) != nullptr);
        }
        // Already initialized.
        // TODO: should we log an error here?
        return false;
    }

    //
    // Marashal C struct tro Aria C++ API
    //
    void aria_logevent(aria_prop* evt)
    {
        EventProperties props;
        props.unpack(evt);

        // TODO: should we remove the iKey from props?
        auto m = props.GetProperties();
        EventProperty &prop = m["iKey"];
        std::string token = prop.as_string;

        // Initialize if needed
        if (!token.empty())
        {
            aria_initialize(token.c_str());
        }

        // TODO: should we support source passed in evt?
        ILogger *logger = LogManagerC::GetLogger(token);
        logger->LogEvent(props);
    }

    void aria_teardown()
    {
        LogManagerC::FlushAndTeardown();
    }

    void aria_pause()
    {
        LogManagerC::PauseTransmission();
    }

    void aria_resume()
    {
        LogManagerC::ResumeTransmission();
    }

    void aria_upload()
    {
        LogManagerC::UploadNow();
    }

    void aria_flush()
    {
        LogManagerC::Flush();
    }

}