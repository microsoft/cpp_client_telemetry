#pragma once
#include "mat/config.h"

#ifndef DEFAULTDATAVIEWER_HPP
#define DEFAULTDATAVIEWER_HPP

#include "OnDisableNotificationCollection.hpp"

#include "public/Version.hpp"
#include "public/IDataViewer.hpp"
#include "public/IHttpClient.hpp"
#include "public/ctmacros.hpp"

#include <atomic>
#include <condition_variable>
#include <string>
#include <vector>

namespace ARIASDK_NS_BEGIN {

    class DefaultDataViewer : public MAT::IDataViewer, public MAT::IHttpResponseCallback
    {
    public:
        DefaultDataViewer(const std::shared_ptr<IHttpClient>& httpClient, const std::string& machineFriendlyIdentifier);

        void ReceiveData(const std::vector<uint8_t>& packetData) noexcept override;

        const char* GetName() const noexcept override
        {
            return s_name;
        }

        bool EnableRemoteViewer(const std::string& endpoint);

        bool EnableLocalViewer();

        bool EnableLocalViewer(const std::string& AppId, const std::string& AppPackage);
        bool DisableViewer() noexcept;

        void RegisterOnDisableNotification(const std::function<void()>& onDisabled) noexcept;

    protected:
        MAT::IHttpClient* GetHttpClient() const noexcept
        {
            return m_httpClient.get();
        }

        const std::string& GetMachineFriendlyIdentifier() const noexcept
        {
            return m_machineFriendlyIdentifier;
        }

        bool IsTransmissionEnabled() const noexcept
        {
            return m_isTransmissionEnabled;
        }

        void SetTransmissionEnabled(bool state) noexcept
        {
            m_isTransmissionEnabled.exchange(state);
        }

    private:
        void OnHttpResponse(IHttpResponse* response) override;
        void SendPacket(const std::vector<std::uint8_t>& packetData);
        void ProcessReceivedPacket(const std::vector<std::uint8_t>& packetData);

        bool IsNullOrEmpty(const std::string& toCheck) noexcept;

        std::condition_variable m_initializationEvent;
        mutable std::mutex m_transmissionGuard;

        std::shared_ptr<MAT::IHttpClient> m_httpClient;
        std::atomic<bool> m_isTransmissionEnabled;
        std::atomic<bool> m_enabledRemoteViewerNotifyCalled;

        const std::string m_machineFriendlyIdentifier;
        std::string m_endpoint;
        OnDisableNotificationCollection m_onDisableNotificationCollection;

        static const char* s_name;
    };

} ARIASDK_NS_END

#endif