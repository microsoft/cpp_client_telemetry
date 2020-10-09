LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE            := atelemetry
SRC                     := ../src
LOCAL_SRC_FILES         := $(SRC)/main.cpp $(SRC)/ITelemetryAgent.cpp $(SRC)/ParcelVector.cpp
LOCAL_CFLAGS            += -Wall -Wno-unused-parameter

LOCAL_C_INCLUDES	+= \
	$(ANDROID_BUILD_TOP)/frameworks/native/include	\
	$(ANDROID_BUILD_TOP)/system/core/include	\
	$(ANDROID_BUILD_TOP)/system/core/base/include	\
	$(LOCAL_PATH)/../include	\
	$(LOCAL_PATH)/../public

LOCAL_SHARED_LIBRARIES += libc libbinder liblog libutils libcutils liblog
LOCAL_LDLIBS		:= -lutils -lcutils -lm -ldl -lc -lbinder -llog -lcutils -lutils -llog
LOCAL_LDFLAGS		:= -L$(ANDROID_SYSTEM_LIBS)

include $(BUILD_EXECUTABLE)
