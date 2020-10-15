#ifndef AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_BP_TELEMETRY_AGENT_H_
#define AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_BP_TELEMETRY_AGENT_H_

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <utils/Errors.h>

#include "ITelemetryAgent.h"

namespace com
{
    namespace microsoft
    {
        namespace telemetry
        {
            class BpTelemetryAgent : public ::android::BpInterface<ITelemetryAgent>
            {
               public:
                explicit BpTelemetryAgent(const ::android::sp<::android::IBinder>& _aidl_impl);
                virtual ~BpTelemetryAgent() = default;
                ::android::binder::Status open(const ::android::String16& config, int64_t* _aidl_return) override;
                ::android::binder::Status close(int64_t id, int64_t* _aidl_return) override;
                ::android::binder::Status clear(int64_t id, int64_t* _aidl_return) override;
                ::android::binder::Status config(int64_t id, const ::android::String16& config, int64_t* _aidl_return) override;
                ::android::binder::Status writeEvent(int64_t id, int64_t contentType, const ::std::vector<uint8_t>& data, int64_t* _aidl_return) override;
                ::android::binder::Status writeBlob(int64_t id, ::android::base::unique_fd pfd, const ::android::String16& name, const ::android::String16& contentType, int64_t byteCount, int64_t* _aidl_return) override;
                ::android::binder::Status writeMetadata(int64_t id, const ::std::vector<::android::String16>& metadata, int64_t* _aidl_return) override;
                ::android::binder::Status pause(int64_t id, int64_t* _aidl_return) override;
                ::android::binder::Status resume(int64_t id, int64_t* _aidl_return) override;
                ::android::binder::Status upload(int64_t id, int64_t* _aidl_return) override;
                ::android::binder::Status stop(int64_t* _aidl_return) override;
                ::android::binder::Status opmode(int64_t mode, int64_t* _aidl_return) override;
                ::android::binder::Status version(::android::String16* _aidl_return) override;
            };  // class BpTelemetryAgent

        }  // namespace telemetry

    }  // namespace microsoft

}  // namespace com

#endif  // AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_BP_TELEMETRY_AGENT_H_
