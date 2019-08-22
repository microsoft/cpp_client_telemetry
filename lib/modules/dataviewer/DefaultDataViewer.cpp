#include "DefaultDataViewer.hpp"

#include "http/HttpClientFactory.hpp"
#include "public/DebugEvents.hpp"
#include "pal/PAL.hpp"

#include <chrono>

namespace ARIASDK_NS_BEGIN
{

    DefaultDataViewer::DefaultDataViewer(std::shared_ptr<IHttpClient> httpClient, const char* machineFriendlyIdentifier)
          : m_httpClient(httpClient),
            m_machineFriendlyIdentifier(machineFriendlyIdentifier),
            m_endpoint(""),
            m_isTransmissionEnabled(false)
    {
        if (m_httpClient == nullptr)
        {
    #ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
            m_httpClient = std::shared_ptr<IHttpClient>(HttpClientFactory::Create());
    #else
            throw std::invalid_argument("httpClient");
    #endif
        }

        if (!m_machineFriendlyIdentifier || strlen(m_machineFriendlyIdentifier) == 0)
        {
            throw std::invalid_argument("machineFriendlyIdentifier");
        }
    }

    void DefaultDataViewer::RecieveData(const std::vector<std::uint8_t>& packetData) const noexcept
    {
        const_cast<DefaultDataViewer*>(this)->ProcessReceivedPacket(packetData);
    }

    bool DefaultDataViewer::EnableLocalViewer()
    {
        throw std::logic_error("Function not implemented");
    }

    bool DefaultDataViewer::EnableLocalViewer(const std::string& /*AppId*/, const std::string& /*AppPackage*/)
    {
        throw std::logic_error("Function not implemented");
    }

    void DefaultDataViewer::ProcessReceivedPacket(const std::vector<std::uint8_t>& packetData)
    {
        std::unique_lock<std::mutex> transmissionLock(m_transmissionGuard);
        if (!m_isTransmissionEnabled)
            return;
        SendPacket(packetData);
    }

    void DefaultDataViewer::SendPacket(const std::vector<std::uint8_t>& packetData)
    {
        auto request = m_httpClient->CreateRequest();
        request->SetMethod("POST");
        auto localPacketDataCopy = std::vector<std::uint8_t> { packetData };
        request->SetBody(localPacketDataCopy);

        auto& header = request->GetHeaders();
        header.add("Machine-Identifier", m_machineFriendlyIdentifier);
        header.add("App-Name", PAL::GetSystemInformation()->GetAppId());
        header.add("App-Platform", PAL::GetSystemInformation()->GetDeviceClass());
        header.add("Content-Type", "Application/bond-compact-binary");

        request->SetUrl(m_endpoint);
        m_httpClient->SendRequestAsync(request, this);
    }

    void DefaultDataViewer::OnHttpResponse(IHttpResponse* response)
    {
        if (response->GetStatusCode() != 200)
        {
            m_isTransmissionEnabled.exchange(false);
            m_httpClient->CancelAllRequests();
        }
        else
        {
            m_isTransmissionEnabled.exchange(true);
            m_initializationEvent.notify_all();
        }
    }

    bool DefaultDataViewer::EnableRemoteViewer(const char* endpoint)
    {
        std::unique_lock<std::mutex> transmissionLock(m_transmissionGuard);
        
        m_endpoint = endpoint;
        SendPacket(std::vector<std::uint8_t>{});

        m_initializationEvent.wait_until(transmissionLock, std::chrono::system_clock::now() + std::chrono::seconds(30));
        if (!m_isTransmissionEnabled)
        {
            m_endpoint = "";
        }
        return m_isTransmissionEnabled;
    }

    bool DefaultDataViewer::DisableViewer() noexcept
    {
        m_isTransmissionEnabled.exchange(false);

        return true;
    }

} ARIASDK_NS_END
