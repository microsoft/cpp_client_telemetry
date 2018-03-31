#pragma once

#define USE_ECS

#ifdef USE_ECS
#include "IECSClient.hpp"
namespace EXP = Microsoft::Applications::Experimentation::ECS;

typedef EXP::IECSClientCallback IEXPClientCallback;
typedef EXP::IECSClient         IEXPClient;
typedef EXP::ECSClientConfiguration EXPClientConfiguration;

#define EXPEventType    EXP::IECSClientCallback::ECSClientEventType
#define EXPEventContext EXP::IECSClientCallback::ECSClientEventContext

#define ET_CONFIG_UPDATE_SUCCEEDED IECSClientCallback::ET_CONFIG_UPDATE_SUCCEEDED
#define OnEXPEvent OnECSClientEvent

#else
#include "IAFDClient.hpp"
// TODO: FIXME for AFD
namespace EXP = Microsoft::Applications::Experimentation::AFD;
typedef EXP::IAFDClientCallback IEXPClientCallback;
typedef EXP::IAFDClient         IEXPClient;
typedef EXP::IAFDClientCallback::ECSClientEventType EXPEventType;
typedef EXP::IAFDClientCallback::ECSClientEventContext EXPEventContext;
typedef EXP::AFDClientConfiguration EXPClientConfiguration;
#define OnEvent OnAFDClientEvent
#endif

#include "LogManager.hpp"
#include <vector>

#define ASSERT(o) { assert(o); }
#define ASSERT_EQ(v, o) { assert(o == v); }

#define CLIENT_VERSION "102909/3.1.0.0"

class EXPClientListener : public IEXPClientCallback
{

public:

    EXPClientListener() {};

    /**
    * Destroy all unreleased clients
    */
    ~EXPClientListener()
    {
        DetachAll();
    }

    IEXPClient* Attach(Microsoft::Applications::Telemetry::ILogger* pLogger = nullptr);

    void Detach(IEXPClient* client);

    virtual void OnEXPEvent(EXPEventType evtType, EXPEventContext evtContext);

    void DetachAll()
    {
        for (auto client : m_clients)
            Destroy(client);
        m_clients.clear();
    }

protected:

    std::vector<IEXPClient*> m_clients;
    void Destroy(IEXPClient *client);

};
