// Copyright (c) Microsoft. All rights reserved.

#include "MockIHttpClient.hpp"
#include "MockILogManagerInternal.hpp"
#include "MockIOfflineStorage.hpp"
#include "MockIOfflineStorageObserver.hpp"
#include "MockIRuntimeConfig.hpp"
#include "MockISemanticContext.hpp"
#include "MockISqlite3Proxy.hpp"

#ifdef ARIASDK_PAL_SKYPE
    #include "MockIEcsClient.hpp"
#endif

namespace testing {


// Constructors and destructors of mock classes are compiled separately
// to speed up compilation. See GMock documentation for the rationale:
// https://github.com/google/googlemock/blob/master/googlemock/docs/CookBook.md#making-the-compilation-faster


MockIHttpClient::MockIHttpClient() {}
MockIHttpClient::~MockIHttpClient() {}

MockILogManagerInternal::MockILogManagerInternal() {}
MockILogManagerInternal::~MockILogManagerInternal() {}

MockIOfflineStorageObserver::MockIOfflineStorageObserver() {}
MockIOfflineStorageObserver::~MockIOfflineStorageObserver() {}

MockIOfflineStorage::MockIOfflineStorage() {}
MockIOfflineStorage::~MockIOfflineStorage() {}

MockIRuntimeConfig::MockIRuntimeConfig() {}
MockIRuntimeConfig::~MockIRuntimeConfig() {}

MockISemanticContext::MockISemanticContext() {}
MockISemanticContext::~MockISemanticContext() {}

MockISqlite3Proxy::MockISqlite3Proxy() {}
MockISqlite3Proxy::~MockISqlite3Proxy() {}


#ifdef ARIASDK_PAL_SKYPE

MockIEcsConfig::MockIEcsConfig() {}
MockIEcsConfig::~MockIEcsConfig() {}

MockIEcsClient::MockIEcsClient() {}
MockIEcsClient::~MockIEcsClient() {}

#endif


} // namespace testing
