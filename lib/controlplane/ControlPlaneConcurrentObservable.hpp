#pragma once

#include "IControlPlane.hpp"
#include "ConcurrentObservable.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  { namespace ControlPlane
// *INDENT-ON*
{
    struct ControlPlaneChangeData : public ChangeDataTemplate<IControlPlane>
    {
        ControlPlaneChangeData::ControlPlaneChangeData(IControlPlane& controlPlane, const GUID_t & ariaTenantId)
            : ChangeDataTemplate(controlPlane, ariaTenantId)
        {
        }
    };

    class ControlPlaneConcurrentObservable : public ConcurrentObservable<IControlPlaneChangeEventHandler*, const ControlPlaneChangeData&>
    {
        inline void NotifyOneObserver(IControlPlaneChangeEventHandler* observer, const ControlPlaneChangeData& data)
        {
            observer->OnChange(data.m_source, data.m_ariaTenantId);
        }
    };

}}}} // namespace Microsoft::Applications::Events ::ControlPlane
