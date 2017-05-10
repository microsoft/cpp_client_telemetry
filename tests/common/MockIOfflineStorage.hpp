// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/IOfflineStorage.hpp>
#include <aria/IHttpClient.hpp>

namespace testing {


class MockIOfflineStorage : public ARIASDK_NS::IOfflineStorage {
  public:
    MockIOfflineStorage();
    virtual ~MockIOfflineStorage();

    MOCK_METHOD1(Initialize, void(ARIASDK_NS::IOfflineStorageObserver & observer));
    MOCK_METHOD0(Shutdown, void());
    MOCK_METHOD1(StoreRecord, bool(ARIASDK_NS::StorageRecord const &));
    MOCK_METHOD4(GetAndReserveRecords, bool(std::function<bool(ARIASDK_NS::StorageRecord&&)> const &, unsigned, ARIASDK_NS::EventPriority, unsigned));
    MOCK_METHOD2(DeleteRecords, void(std::vector<ARIASDK_NS::StorageRecordId> const &, ARIASDK_NS::HttpHeaders ));
    MOCK_METHOD3(ReleaseRecords, void(std::vector<ARIASDK_NS::StorageRecordId> const &, bool, ARIASDK_NS::HttpHeaders));
    MOCK_METHOD2(StoreSetting, bool(std::string const &, std::string const &));
    MOCK_METHOD1(GetSetting, std::string(std::string const &));
};


} // namespace testing
