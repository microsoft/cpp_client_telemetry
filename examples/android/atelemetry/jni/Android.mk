LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
LOCAL_MODULE            := atelemetry

# Modify this to point to 1DS SDK headers
MATSDK_INCLUDE			:= $(abspath $(LOCAL_PATH)../../../../../lib/include/public)
$(info MATSDK_INCLUDE	= $(MATSDK_INCLUDE))

MATSDK_SOURCE			:= $(abspath $(LOCAL_PATH)../../../../../lib)
$(info MATSDK_INCLUDE	= $(MATSDK_SOURCE))

# Make sure that the shared library has been built with /build-android-os.[sh|cmd] script
# NOTE: linking statically would require a bit of extra C++ runtime secret sauce in LDLIBS
MATSDK_LIBDIR			:= $(LOCAL_PATH)/../../../../out/shared/lib/

SRC                     := ../src

LOCAL_SRC_FILES         :=										\
	$(SRC)/main.cpp												\
	$(MATSDK_SOURCE)/binder/server/ITelemetryAgent.cpp			\
	$(MATSDK_SOURCE)/binder/server/capi-binder-server.cpp		\
	$(MATSDK_SOURCE)/binder/ParcelVector.cpp					\
	$(MATSDK_SOURCE)/binder/client/capi-binder-client.cpp		\

LOCAL_CFLAGS            += -Wall -Wno-unused-parameter

LOCAL_C_INCLUDES	+=											\
	$(ANDROID_BUILD_TOP)/frameworks/native/include				\
	$(ANDROID_BUILD_TOP)/system/core/include					\
	$(ANDROID_BUILD_TOP)/system/core/base/include				\
	$(LOCAL_PATH)/../include									\
	$(LOCAL_PATH)/../public										\
	$(MATSDK_INCLUDE)											\
	$(MATSDK_SOURCE)/binder/include								\
	$(MATSDK_SOURCE)/api										\
	.

# Android OS System libraries
LOCAL_LDFLAGS		:= -L$(ANDROID_SYSTEM_LIBS)
LOCAL_LDLIBS		:= -lutils -lcutils -lm -ldl -lc -lbinder -llog -lcutils -lutils -llog

# 1DS C++ SDK static library (libmat.a)
LOCAL_LDFLAGS		+= -L$(MATSDK_LIBDIR)
LOCAL_LDLIBS		+= -lmat

include $(BUILD_EXECUTABLE)
