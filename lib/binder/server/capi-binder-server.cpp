#define SERVICE_NAME "TelemetryAgent"
#define LOG_TAG "TelemetryServer"

#include "CommonFields.h"
#include "Version.hpp"
#include "ctmacros.hpp"
#include "mat.h"

#include <android/log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/TextOutput.h>

#include <utils/Vector.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "LoggingMacros.h"

#include "EvtPropConverter.hpp"

#include <BnTelemetryAgent.h>
#include <BpTelemetryAgent.h>

using namespace com::microsoft::telemetry;
using namespace android;

class TelemetryAgent : public BnTelemetryAgent, public ITelemetryAgentDefault
{
#if 0
        public:
            virtual status_t onTransact( uint32_t code,
                    const Parcel& data,
                    Parcel* reply,
                    uint32_t flags = 0);
#endif

    ::android::binder::Status open(const ::android::String16& config,
                                   int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::open");
        ::android::String8 cfg(config);
        *_aidl_return = evt_open(cfg.c_str());
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status close(int64_t id, int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::close");
        evt_close(id);
        *_aidl_return = 0;
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status clear(int64_t id, int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::clear");
        // TODO: not implemented
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status config(int64_t id,
                                     const ::android::String16& config,
                                     int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::config");
        // TODO: not implemented
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status writeEvent(int64_t id, int64_t contentType, const ::std::vector<uint8_t>& data, int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::writeEvent");

        std::vector<evt_prop> event;
        INFO("TelemetryAgentServer::writeEvent - rcv data. size=%u", data.size());
        EvtPropConverter::deserialize(data, event);
        evt_log(id, event.data());
        EvtPropConverter::clear(event);

        return ::android::binder::Status::ok();
    };

    ::android::binder::Status writeBlob(int64_t id,
                                        ::android::base::unique_fd pfd,
                                        const ::android::String16& name,
                                        const ::android::String16& contentType,
                                        int64_t byteCount,
                                        int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::writeBlob");
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status writeMetadata(int64_t id,
                                            const ::std::vector<::android::String16>& metadata,
                                            int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::writeMetadata");
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status pause(int64_t id, int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::pause");
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status resume(int64_t id, int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::resume");
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status upload(int64_t id, int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::upload");
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status stop(int64_t* _aidl_return) override
    {
        INFO("TelemetryAgentServer::stop");
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status opmode(int64_t mode, int64_t* _aidl_return)
        override
    {
        INFO("TelemetryAgentServer::opmode");
        return ::android::binder::Status::ok();
    };

    ::android::binder::Status version(::android::String16* _aidl_return)
        override
    {
        INFO("TelemetryAgentServer::version");
        return ::android::binder::Status::ok();
    };

#if 0  //                                                                 \
       // TODO: add                                                       \
       //	virtual status_t dump(int fd, const Vector<String16>& args) = 0 \
       // to                                                              \
       //

	status_t dump(int fd, const Vector<String16>& args) override {
#if 0
		IPCThreadState* self = IPCThreadState::self();
		const int pid = self->getCallingPid();
		const int uid = self->getCallingUid();
		if ((uid != AID_SHELL) &&
				!PermissionCache::checkPermission(
						String16("android.permission.DUMP"), pid, uid))
		return PERMISSION_DENIED;
#endif
		if (fd<0) {
			printf("Someone's messing up with fd=%d !!!\n", fd);
		}

		printf("Got a dumpsys request, writing to fd=%d\n", fd);

		String8 result = String8::format("Hello, world! fd=%d\n", fd);
		write(fd, result.string(), result.size());
		write(fd, "\n", 1);
		fsync(fd);

		return NO_ERROR;
	}
#endif
};

void startService()
{
    printf("Starting service...\n");
    ALOGD("TelemetryAgent service is starting...");
    defaultServiceManager()->addService(String16(SERVICE_NAME), new TelemetryAgent());
    android::ProcessState::self()->startThreadPool();
    ALOGD("TelemetryAgent service is ready.");
    IPCThreadState::self()->joinThreadPool();
    ALOGD("TelemetryAgent service thread joined.");
};
