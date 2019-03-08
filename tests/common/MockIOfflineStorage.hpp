// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IOfflineStorage.hpp"
#include "IHttpClient.hpp"

namespace testing {


class MockIOfflineStorage : public ARIASDK_NS::IOfflineStorage {
  public:
    MockIOfflineStorage();
    virtual ~MockIOfflineStorage();

    MOCK_METHOD1(Initialize, void(ARIASDK_NS::IOfflineStorageObserver & observer));
    MOCK_METHOD0(Shutdown, void());
    MOCK_METHOD0(Flush, void());
    MOCK_METHOD1(StoreRecord, bool(ARIASDK_NS::StorageRecord const &));
    MOCK_METHOD4(GetAndReserveRecords, bool(std::function<bool(ARIASDK_NS::StorageRecord&&)> const &, unsigned, ARIASDK_NS::EventLatency, unsigned));
    MOCK_METHOD0(IsLastReadFromMemory, bool());
    MOCK_METHOD0(LastReadRecordCount, unsigned());
    MOCK_METHOD3(DeleteRecords, void(std::vector<ARIASDK_NS::StorageRecordId> const &, ARIASDK_NS::HttpHeaders, bool& ));
    MOCK_METHOD4(ReleaseRecords, void(std::vector<ARIASDK_NS::StorageRecordId> const &, bool, ARIASDK_NS::HttpHeaders, bool&));
    MOCK_METHOD2(StoreSetting, bool(std::string const &, std::string const &));
    MOCK_METHOD1(GetSetting, std::string(std::string const &));
    MOCK_METHOD0(GetSize, size_t());
    MOCK_METHOD1(DeleteRecords, void(const std::map<std::string, std::string> &));
    MOCK_CONST_METHOD1(GetRecordCount, size_t(ARIASDK_NS::EventLatency));
    MOCK_METHOD3(GetRecords, std::vector<ARIASDK_NS::StorageRecord>(bool, ARIASDK_NS::EventLatency, unsigned));
	MOCK_METHOD0(ResizeDb, bool());
};


} // namespace testing
