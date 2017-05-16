#include <exception>

#include "SingleControlPlane.hpp"
#include "utils/Common.hpp"

using namespace Microsoft::Applications::Telemetry;

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry { namespace ControlPlane
// *INDENT-ON*
{
    SingleControlPlane::SingleControlPlane(ILocalStorageReader& localStorageReader)
        :m_localStorageReader(localStorageReader)
    {
        m_localStorageReader.RegisterChangeEventHandler(this);
    }

    SingleControlPlane::~SingleControlPlane()
    {
        m_localStorageReader.UnregisterChangeEventHandler(this);
        {
            std::lock_guard<std::mutex> lockguard(m_mutex);
            m_tenantMap.clear();
        }
    }

    const std::string& SingleControlPlane::GetStringParameter(const GUID_t& ariaTenantId, const std::string& parameterId, const std::string& defaultValue)
    {
        std::string value;

        if (TryGetStringParameter(ariaTenantId, parameterId, value))
        {
            return *(new std::string(value));  // Return a COPY of the string, so caller owns the delete
        }

        return defaultValue;
    }

    long SingleControlPlane::GetLongParameter(const GUID_t& ariaTenantId, const std::string& parameterId, long defaultValue)
    {
        long value;

        if (TryGetLongParameter(ariaTenantId, parameterId, value))
        {
            return value;
        }

        return defaultValue;
    }

    bool SingleControlPlane::GetBoolParameter(const GUID_t& ariaTenantId, const std::string& parameterId, bool defaultValue)
    {
        bool value;

        if (TryGetBoolParameter(ariaTenantId, parameterId, value))
        {
            return value;
        }

        return defaultValue;
    }

    bool SingleControlPlane::TryGetStringParameter(const GUID_t& ariaTenantId, const std::string& parameterId, std::string& value)
    {
        std::lock_guard<std::mutex> lockguard(m_mutex);

        TenantDataPtr tenantDataPtr = GetTenantDataPtr(ariaTenantId);

        if (tenantDataPtr->m_isDummy)
            return false;

        std::unordered_map<std::string, std::string>::const_iterator found = tenantDataPtr->m_stringMap.find(parameterId);

        if (found == tenantDataPtr->m_stringMap.end())
            return false;

        value = found->second;
        return true;
    }

    bool SingleControlPlane::TryGetLongParameter(const GUID_t& ariaTenantId, const std::string& parameterId, long& value)
    {
        std::lock_guard<std::mutex> lockguard(m_mutex);

        TenantDataPtr tenantDataPtr = GetTenantDataPtr(ariaTenantId);

        if (tenantDataPtr->m_isDummy)
            return false;

        std::unordered_map<std::string, long>::const_iterator found = tenantDataPtr->m_longMap.find(parameterId);

        if (found == tenantDataPtr->m_longMap.end())
            return false;

        value = found->second;
        return true;
    }

    bool SingleControlPlane::TryGetBoolParameter(const GUID_t& ariaTenantId, const std::string& parameterId, bool& value)
    {
        std::lock_guard<std::mutex> lockguard(m_mutex);

        TenantDataPtr tenantDataPtr = GetTenantDataPtr(ariaTenantId);

        if (tenantDataPtr->m_isDummy)
            return false;

        std::unordered_map<std::string, bool>::const_iterator found = tenantDataPtr->m_boolMap.find(parameterId);

        if (found == tenantDataPtr->m_boolMap.end())
            return false;

        value = found->second;
        return true;
    }

    void SingleControlPlane::RegisterChangeEventHandler(IControlPlaneChangeEventHandler* handler)
    {
        m_observable.Register(handler);
    }

    void SingleControlPlane::UnregisterChangeEventHandler(IControlPlaneChangeEventHandler* handler)
    {
        m_observable.Unregister(handler);
    }

    void SingleControlPlane::OnChange(ILocalStorageReader& localStorageReader, const GUID_t& ariaTenantId)
    {
        UNREFERENCED_PARAMETER(localStorageReader);   // Not needed for this implementation

        // Erase any data cached for this tenant
        {
            std::lock_guard<std::mutex> lockguard(m_mutex);
            m_tenantMap.erase(ariaTenantId);
        }

        // Notify all observers
        ControlPlaneChangeData controlPlaneChangeData((IControlPlane&) *this, ariaTenantId);
        m_observable.NotifyObservers(controlPlaneChangeData);
    }

    /// <summary>
    /// Provide a TenantDataPtr for the specified ariaTenantId
    /// </summary>
    /// <returns>A valid object that exists in m_TenantMap</returns>
    TenantDataPtr SingleControlPlane::GetTenantDataPtr(const GUID_t& ariaTenantId)
    {
        std::unordered_map<GUID_t, TenantDataPtr, GuidMapHasher>::const_iterator found = m_tenantMap.find(ariaTenantId);

        if (found != m_tenantMap.end())
            return found->second;

        TenantDataPtr dataFromStorage = m_localStorageReader.ReadTenantData(ariaTenantId);

        if (dataFromStorage != nullptr)
        {
            m_tenantMap.insert({ { ariaTenantId, dataFromStorage } });
            return dataFromStorage;
        }

        // Create a dummy object and return it
        TenantDataPtr dummyTenantData = new TenantData();
        m_tenantMap.insert({ { ariaTenantId, dummyTenantData } });
        return dummyTenantData;
    }

}}}} // namespace Microsoft::Applications::Telemetry::ControlPlane