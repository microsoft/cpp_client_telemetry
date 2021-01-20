//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "IOfflineStorage.hpp"

namespace testing {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"  // GMock MOCK_METHOD* macros don't use override.
#endif

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

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace testing

