#pragma once

#include "IAFDClient.hpp"
#include "LogManager.hpp"

#define ASSERT(o) { assert(o); }
#define ASSERT_EQ(v, o) { assert(o == v); }

#define CLIENT_VERSION "102909/3.1.0.0"

class AFDClientListener
    : public Microsoft::Applications::Experimentation::AFD::IAFDClientCallback
{
public:
    AFDClientListener();
    ~AFDClientListener();

    void Init(MAT::ILogger* pLogger);
    void Term();

    // IAFDClientCallback
    virtual void OnAFDClientEvent(
        Microsoft::Applications::Experimentation::AFD::IAFDClientCallback::AFDClientEventType evtType,
        Microsoft::Applications::Experimentation::AFD::IAFDClientCallback::AFDClientEventContext evtContext);

public:
    Microsoft::Applications::Experimentation::AFD::IAFDClient* m_pAFDClient;
};
