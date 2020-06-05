// Copyright (c) Microsoft. All rights reserved.

#include "LogManager.hpp"
#include "LoggerWrapper.hpp"
#include <jni.h>

LOGMANAGER_INSTANCE

extern "C"
JNIEXPORT jlong JNICALL
Java_com_microsoft_applications_events_LoggerWrapper_Initialize(JNIEnv *env, jclass clazz) {
    return reinterpret_cast<jlong>(new MAT::LoggerWrapper());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LoggerWrapper_LogEvent(JNIEnv *env, jobject thiz,
        jlong p_event_properties,
        jlong p_logger_wrapper) {

    MAT::LoggerWrapper* pLoggerWrapper = (MAT::LoggerWrapper*)p_logger_wrapper;
    MAT::EventProperties* pEventProperties = (MAT::EventProperties*)(p_event_properties);
    pLoggerWrapper->LogEvent(*pEventProperties);
}

namespace ARIASDK_NS_BEGIN {
    LoggerWrapper::LoggerWrapper(): logger(LogManager::Initialize()) {}
    void LoggerWrapper::LogEvent(const EventProperties & eventProperties) {
        this->logger->LogEvent(eventProperties);
    }
} ARIASDK_NS_END