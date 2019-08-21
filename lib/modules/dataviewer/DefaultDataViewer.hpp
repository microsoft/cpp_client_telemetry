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

namespace ARIASDK_NS_BEGIN {

    class DefaultDataViewer final : public IDataViewer
    {
    public:
        DefaultDataViewer(std::shared_ptr<IHttpClient> httpClient);

        void RecieveData(const std::vector<std::uint8_t>& packetData) const noexcept override;

        const char* const GetName() const noexcept override
        {
            return m_name;
        }

        bool EnableRemoteViewer(const char* endpoint);

        bool EnableLocalViewer();

        bool EnableLocalViewer(const std::string& AppId, const std::string& AppPackage);
        bool DisableViewer() noexcept;

    private:
        void ProcessQueue();

        static constexpr const char* m_name = "DefaultDataViewer";
        std::shared_ptr<MAT::IHttpClient> m_httpClient;

        std::atomic<bool> m_isTransmissionEnabled;
        const char* m_endpoint;

        mutable std::queue<std::vector<std::uint8_t>> m_packetQueue;

    };

} ARIASDK_NS_END

#endif