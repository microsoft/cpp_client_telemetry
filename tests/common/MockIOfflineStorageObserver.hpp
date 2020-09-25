// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IOfflineStorage.hpp"

namespace testing {


class MockIOfflineStorageObserver : public MAT::IOfflineStorageObserver {
  public:
    MockIOfflineStorageObserver();
    virtual ~MockIOfflineStorageObserver();

    MOCK_METHOD1(OnStorageOpened, void(std::string const &));
    MOCK_METHOD1(OnStorageFailed, void(std::string const &));
    MOCK_METHOD1(OnStorageOpenFailed, void(std::string const &));
    MOCK_METHOD1(OnStorageTrimmed, void(std::map<std::string, size_t> const&));
    MOCK_METHOD1(OnStorageRecordsDropped, void(std::map<std::string, size_t> const&));
    MOCK_METHOD1(OnStorageRecordsRejected, void(std::map<std::string, size_t> const&));
    MOCK_METHOD1(OnStorageRecordsSaved, void(size_t numRecords));
};


} // namespace testing
