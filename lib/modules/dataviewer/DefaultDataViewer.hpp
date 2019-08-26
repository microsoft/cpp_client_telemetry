#pragma once
#include "mat/config.h"

#ifndef DEFAULTDATAVIEWER_HPP
#define DEFAULTDATAVIEWER_HPP

#include "public/Version.hpp"
#include "public/IDataViewer.hpp"
#include "public/IHttpClient.hpp"
#include "public/ctmacros.hpp"

#include <atomic>
#include <queue>
#include <condition_variable>

namespace ARIASDK_NS_BEGIN {

    class DefaultDataViewer final : public MAT::IDataViewer, public MAT::IHttpResponseCallback
    {
    public:
        DefaultDataViewer(std::shared_ptr<IHttpClient> httpClient, const char* machineFriendlyIdentifier);

        void ReceiveData(const std::vector<std::uint8_t>& packetData) noexcept override;

        const char* const GetName() const noexcept override
        {
            return m_name;
        }

        bool EnableRemoteViewer(const char* endpoint);

        bool EnableLocalViewer();

        bool EnableLocalViewer(const std::string& AppId, const std::string& AppPackage);
        bool DisableViewer() noexcept;

        std::shared_ptr<MAT::IHttpClient> GetHttpClient() const noexcept
        {
            return m_httpClient;
        }

        const char* GetMachineFriendlyIdentifier() const noexcept
        {
            return m_machineFriendlyIdentifier;
        }

        bool IsTransmissionEnabled() const noexcept
        {
            return m_isTransmissionEnabled;
        }

    private:
        std::condition_variable m_initializationEvent;
        mutable std::mutex m_transmissionGuard;

        void OnHttpResponse(IHttpResponse* response) override;

        void SendPacket(const std::vector<std::uint8_t>& packetData);
        void ProcessReceivedPacket(const std::vector<std::uint8_t>& packetData);

        static constexpr const char* m_name { "DefaultDataViewer" };
        static constexpr const char* m_httpPrefix { "http://" };
        static constexpr const char* m_httpsPrefix { "https://" };

        std::shared_ptr<MAT::IHttpClient> m_httpClient;
        std::atomic<bool> m_isTransmissionEnabled;
        std::atomic<bool> m_enabledRemoteViewerNotifyCalled;
        const char* m_endpoint;
        const char* m_machineFriendlyIdentifier;
    };

} ARIASDK_NS_END

#endif