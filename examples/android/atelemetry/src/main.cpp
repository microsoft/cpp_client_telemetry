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

#define SERVICE_NAME "TelemetryAgent"
#define LOG_TAG SERVICE_NAME

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

// 1DS C++ SDK - Microsoft Applications Telemetry C API header
#include "mat.h"

#define RUN_AS_SERVICE 1  // No arguments
#define RUN_AS_CLIENT 2   // Any argument

void startService();

void startClient();

int main(int argc, char** argv)
{
    ALOGI("Hello!");
    printf("Hello, how are you?\n");

    switch (argc)
    {
    case RUN_AS_SERVICE:
        startService();
        break;

    case RUN_AS_CLIENT:
        startClient();
        break;
    }

    return 0;
}
