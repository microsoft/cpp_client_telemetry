#pragma once

#include "IECSClient.hpp"
#include "LogManager.hpp"

#define ASSERT(o) { assert(o); }
#define ASSERT_EQ(v, o) { assert(o == v); }

#define CLIENT_VERSION "102909/3.1.0.0"

class ECSClientListener
    : public Microsoft::Applications::Experimentation::ECS::IECSClientCallback
{
public:
    ECSClientListener();
    ~ECSClientListener();

    void Init(Microsoft::Applications::Telemetry::ILogger* pLogger);
    void Term();

    // IECSClientCallback
    virtual void OnECSClientEvent(
        Microsoft::Applications::Experimentation::ECS::IECSClientCallback::ECSClientEventType evtType,
        Microsoft::Applications::Experimentation::ECS::IECSClientCallback::ECSClientEventContext evtContext);

public:
    Microsoft::Applications::Experimentation::ECS::IECSClient* m_pECSClient;
};
