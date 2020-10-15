#ifndef AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_BN_TELEMETRY_AGENT_H_
#define AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_BN_TELEMETRY_AGENT_H_

#include <binder/IInterface.h>

#include "ITelemetryAgent.h"

namespace com
{
    namespace microsoft
    {
        namespace telemetry
        {
            class BnTelemetryAgent : public ::android::BnInterface<ITelemetryAgent>
            {
               public:
                explicit BnTelemetryAgent();
                ::android::status_t onTransact(uint32_t _aidl_code, const ::android::Parcel& _aidl_data, ::android::Parcel* _aidl_reply, uint32_t _aidl_flags) override;
            };  // class BnTelemetryAgent

        }  // namespace telemetry

    }  // namespace microsoft

}  // namespace com

#endif  // AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_BN_TELEMETRY_AGENT_H_
