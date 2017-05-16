#pragma once

#include <mutex>
#include <unordered_map>

#include "aria/IControlPlane.hpp"
#include "ConcurrentObservable.hpp"
#include "ILocalStorage.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry { namespace ControlPlane
// *INDENT-ON*
{
    /// <summary>
    /// Implements IControlPlane for a control plane with a single source of data
    /// </summary>
    class SingleControlPlane : public IControlPlane, ILocalStorageChangeEventHandler
    {
    private:
        ILocalStorageReader& m_localStorageReader;
        ControlPlaneConcurrentObservable m_observable;
        std::unordered_map<GUID_t, TenantDataPtr, GuidMapHasher> m_tenantMap;
        std::mutex m_mutex;

        TenantDataPtr GetTenantDataPtr(const GUID_t& ariaTenantId);
        void OnChange(ILocalStorageReader& localStorageReader, const GUID_t& ariaTenantId);

    public:
        SingleControlPlane(ILocalStorageReader& localStorageReader);
        virtual ~SingleControlPlane();

        const std::string& GetStringParameter(const GUID_t& ariaTenantId, const std::string& parameterId, const std::string& defaultValue) override;
        long GetLongParameter(const GUID_t& ariaTenantId, const std::string& parameterId, long defaultValue) override;
        bool GetBoolParameter(const GUID_t& ariaTenantId, const std::string& parameterId, bool defaultValue) override;
        bool TryGetStringParameter(const GUID_t& ariaTenantId, const std::string& parameterId, std::string& value) override;
        bool TryGetLongParameter(const GUID_t& ariaTenantId, const std::string& parameterId, long& value) override;
        bool TryGetBoolParameter(const GUID_t& ariaTenantId, const std::string& parameterId, bool& value) override;

        void RegisterChangeEventHandler(IControlPlaneChangeEventHandler* handler) override;
        void UnregisterChangeEventHandler(IControlPlaneChangeEventHandler* handler) override;
    };

}}}} // namespace Microsoft::Applications::Telemetry::ControlPlane