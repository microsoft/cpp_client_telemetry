#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"

#include "EXPClientListener.hpp"
#include "LogManager.hpp"
#include "Utils.hpp"

#include <string>
#include <thread>

using namespace std;
using namespace Microsoft::Applications::Telemetry;
using namespace EXP;

const std::string tenantToken = "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322";
                              //"d34863e7429e4e569bb90ff06d00e019-404caa7c-ba18-4dae-ac9e-7e08bade9c60-7278";

static ILogConfiguration& configuration = LogManager::GetLogConfiguration();

/**
 * Start on process start
 * Stop on process shutdown
 */
struct AriaInstance {

    EXPClientListener listener;
    IEXPClient* client;
    bool stopped = false;

    /**
     * Temporary solution for 5 seconds refresh for sampling & filtering demo
     */
    void configRefresher()
    {
        do {
            client->Resume(true);
            // FIXME: this is racy because we might terminate the app
            std::this_thread::sleep_for(std::chrono::seconds(5));
            client->Suspend();
        } while (!stopped);
    }

    /**
    * This approach allows to start any number of times.
    * Each start creates a separate EXP client.
    */
    void ExpClientStart()
    {
        client = listener.Attach(LogManager::GetLogger());
        std::thread refresher([this]() { configRefresher(); } );
        refresher.detach();
    }

    /**
    * Stop all EXP client instances.
    */
    void ExpClientStop()
    {
        stopped = true;
        listener.DetachAll();
    }

    AriaInstance()
    {
        LogManager::Initialize(tenantToken);
        ExpClientStart();
    }

    ~AriaInstance()
    {
        stopped = false;
        LogManager::FlushAndTeardown();
    };

};

static AriaInstance aria;
