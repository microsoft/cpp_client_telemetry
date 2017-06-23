#pragma once

#include "ILocalStorage.hpp"
#include "ConcurrentObservable.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry { namespace ControlPlane
// *INDENT-ON*
{
    struct LocalStorageReaderChangeData : public ChangeDataTemplate<ILocalStorageReader>
    {
        LocalStorageReaderChangeData::LocalStorageReaderChangeData(ILocalStorageReader& controlPlane, const GUID_t & ariaTenantId)
            : ChangeDataTemplate(controlPlane, ariaTenantId)
        {
        }
    };

    class LocalStorageConcurrentObservable : public ConcurrentObservable<ILocalStorageChangeEventHandler *, const LocalStorageReaderChangeData&>
    {
        inline void NotifyOneObserver(ILocalStorageChangeEventHandler * observer, const LocalStorageReaderChangeData& data)
        {
            observer->OnChange(data.m_source, data.m_ariaTenantId);
        }
    };

}}}} // namespace Microsoft::Applications::Telemetry::ControlPlane
