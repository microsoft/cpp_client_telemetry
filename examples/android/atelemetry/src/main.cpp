/*
 * Copyright (C) Microsoft Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define SERVICE_NAME	"TelemetryAgent"
#define LOG_TAG			SERVICE_NAME

#include <android/log.h>

#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/TextOutput.h>
#include <binder/IPCThreadState.h>

#include <utils/Vector.h>

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "LoggingMacros.h"

/*
 Single-threaded service, single-threaded client.
 */

#define RUN_AS_SERVICE	1
#define RUN_AS_CLIENT	2

#include <com/microsoft/telemetry/BnTelemetryAgent.h>
#include <com/microsoft/telemetry/BpTelemetryAgent.h>

using namespace com::microsoft::telemetry;
using namespace android;

// Helper function to get a hold of the "TelemetryAgent" service.
sp<ITelemetryAgent> getTelemetryAgent() {
	::android::sp<IServiceManager> sm = defaultServiceManager();
	ASSERT(sm != 0);
	sp<IBinder> binder = sm->getService(String16(SERVICE_NAME));
	// TODO: If the "TelemetryAgent" service is not running, then getService times out and binder == 0.
	ASSERT(binder != 0);

	sp<ITelemetryAgent> agent = interface_cast<ITelemetryAgent>(binder);
	ASSERT(agent != 0);
	return agent;
};

class TelemetryAgent: public BnTelemetryAgent, public ITelemetryAgentDefault
{
#if 0
        public:
            virtual status_t onTransact( uint32_t code,
                    const Parcel& data,
                    Parcel* reply,
                    uint32_t flags = 0);
#endif

    ::android::binder::Status open(const ::android::String16& config, int64_t* _aidl_return) override
    {
    	INFO("TelemetryAgentServer::open");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status close(int64_t id, int64_t* _aidl_return) override
    {
    	INFO("TelemetryAgentServer::close");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status clear(int64_t id, int64_t* _aidl_return) override
    {
    	INFO("TelemetryAgentServer::clear");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status config(int64_t id, const ::android::String16& config, int64_t* _aidl_return) override
    {
    	INFO("TelemetryAgentServer::config");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status writeEvent(int64_t id, int64_t contentType, const ::std::vector<uint8_t>& data, int64_t* _aidl_return) override
    {
    	INFO("TelemetryAgentServer::writeEvent");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status writeBlob(int64_t id, ::android::base::unique_fd pfd, const ::android::String16& name, const ::android::String16& contentType, int64_t byteCount, int64_t* _aidl_return) override
    {
    	INFO("TelemetryAgentServer::writeBlob");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status writeMetadata(int64_t id, const ::std::vector<::android::String16>& metadata, int64_t* _aidl_return) override
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

    ::android::binder::Status upload(int64_t id, int64_t* _aidl_return)  override
    {
    	INFO("TelemetryAgentServer::upload");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status stop(int64_t* _aidl_return)  override
    {
    	INFO("TelemetryAgentServer::stop");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status opmode(int64_t mode, int64_t* _aidl_return)  override
    {
    	INFO("TelemetryAgentServer::opmode");
    	return ::android::binder::Status::ok();
    };

    ::android::binder::Status version(::android::String16* _aidl_return)  override
    {
    	INFO("TelemetryAgentServer::version");
    	return ::android::binder::Status::ok();
    };

#if 0	//
    	// TODO: add
    	//	virtual status_t dump(int fd, const Vector<String16>& args) = 0
    	// to
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

int main(int argc, char **argv) {
	ALOGI("Hello!");
	printf("Hello, how are you?\n");

	switch (argc) {
	case RUN_AS_SERVICE:
		printf("Starting service...\n");
		ALOGD("TelemetryAgent service is starting...");

		defaultServiceManager()->addService(String16(SERVICE_NAME), new TelemetryAgent());
		android::ProcessState::self()->startThreadPool();
		ALOGD("TelemetryAgent service is ready.");
		IPCThreadState::self()->joinThreadPool();
		ALOGD("TelemetryAgent service thread joined.");
		break;

	case RUN_AS_CLIENT:
		printf("Starting client...\n");
		INFO("TelemetryAgent client: %s", argv[1]);
		int v = atoi(argv[1]);
		sp<ITelemetryAgent> agent = getTelemetryAgent();
		int64_t handle = 0;

		String16 config("[]");
		agent->open(config, &handle);
		int64_t result = 0;
		agent->close(handle, &result);
		break;
	}

	return 0;
}
