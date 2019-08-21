#include "DefaultDataViewer.hpp"
#include "http/HttpClientFactory.hpp"
#include "public/DebugEvents.hpp"

namespace ARIASDK_NS_BEGIN
{

DefaultDataViewer::DefaultDataViewer(std::shared_ptr<IHttpClient> httpClient)
    : m_httpClient(httpClient), m_endpoint(""), m_isTransmissionEnabled(false)
{
    if (m_httpClient == nullptr)
    {
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
        m_httpClient = std::shared_ptr<IHttpClient>(HttpClientFactory::Create());
#else
        throw std::invalid_argument("httpClient");
#endif
    }
}

void DefaultDataViewer::RecieveData(const std::vector<std::uint8_t>& packetData) const noexcept
{
    if (!m_isTransmissionEnabled)
        return;

    m_packetQueue.push(packetData);
}

bool DefaultDataViewer::EnableLocalViewer()
{
    throw std::logic_error("Function not implemented");
}

bool DefaultDataViewer::EnableLocalViewer(const std::string& /*AppId*/, const std::string& /*AppPackage*/)
{
    throw std::logic_error("Function not implemented");
}

void DefaultDataViewer::ProcessQueue()
{
    if (!m_isTransmissionEnabled)
        return;

    
}

bool DefaultDataViewer::EnableRemoteViewer(const char* endpoint)
{
    m_endpoint = endpoint;
    m_isTransmissionEnabled.exchange(true);
    
    return true;
}

bool DefaultDataViewer::DisableViewer() noexcept
{
    m_isTransmissionEnabled.exchange(false);

    return true;
}

} ARIASDK_NS_END
