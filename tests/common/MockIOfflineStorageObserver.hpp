// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <IOfflineStorage.hpp>

namespace testing {


class MockIOfflineStorageObserver : public ARIASDK_NS::IOfflineStorageObserver {
  public:
    MockIOfflineStorageObserver();
    virtual ~MockIOfflineStorageObserver();

    MOCK_METHOD1(OnStorageOpened, void(std::string const &));
    MOCK_METHOD1(OnStorageFailed, void(std::string const &));
    MOCK_METHOD1(OnStorageTrimmed, void(unsigned));
    MOCK_METHOD1(OnStorageRecordsDropped, void(unsigned));
};


} // namespace testing
