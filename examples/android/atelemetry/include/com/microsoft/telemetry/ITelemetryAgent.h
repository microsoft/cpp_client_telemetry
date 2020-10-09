#ifndef AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_I_TELEMETRY_AGENT_H_
#define AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_I_TELEMETRY_AGENT_H_

#include <android-base/unique_fd.h>
#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <binder/Status.h>
#include <cstdint>
#include <utils/String16.h>
#include <utils/StrongPointer.h>
#include <vector>

namespace com
{
    namespace microsoft
    {
        namespace telemetry
        {
            class ITelemetryAgent : public ::android::IInterface
            {
               public:
                DECLARE_META_INTERFACE(TelemetryAgent)
                virtual ::android::binder::Status open(const ::android::String16& config, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status close(int64_t id, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status clear(int64_t id, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status config(int64_t id, const ::android::String16& config, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status writeEvent(int64_t id, int64_t contentType, const ::std::vector<uint8_t>& data, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status writeBlob(int64_t id, ::android::base::unique_fd pfd, const ::android::String16& name, const ::android::String16& contentType, int64_t byteCount, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status writeMetadata(int64_t id, const ::std::vector<::android::String16>& metadata, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status pause(int64_t id, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status resume(int64_t id, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status upload(int64_t id, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status stop(int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status opmode(int64_t mode, int64_t* _aidl_return) = 0;
                virtual ::android::binder::Status version(::android::String16* _aidl_return) = 0;
            };  // class ITelemetryAgent

            class ITelemetryAgentDefault : public ITelemetryAgent
            {
               public:
                ::android::IBinder* onAsBinder() override
                {
                    return nullptr;
                }
                ::android::binder::Status open(const ::android::String16&, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status close(int64_t, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status clear(int64_t, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status config(int64_t, const ::android::String16&, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status writeEvent(int64_t, int64_t, const ::std::vector<uint8_t>&, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status writeBlob(int64_t, ::android::base::unique_fd, const ::android::String16&, const ::android::String16&, int64_t, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status writeMetadata(int64_t, const ::std::vector<::android::String16>&, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status pause(int64_t, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status resume(int64_t, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status upload(int64_t, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status stop(int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status opmode(int64_t, int64_t*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
                ::android::binder::Status version(::android::String16*) override
                {
                    return ::android::binder::Status::fromStatusT(::android::UNKNOWN_TRANSACTION);
                }
            };  // class ITelemetryAgentDefault

        }  // namespace telemetry

    }  // namespace microsoft

}  // namespace com

#endif  // AIDL_GENERATED_COM_MICROSOFT_TELEMETRY_I_TELEMETRY_AGENT_H_
