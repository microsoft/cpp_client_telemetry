// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IOfflineStorage.hpp"
#include "IHttpClient.hpp"

namespace testing {


class MockIOfflineStorage : public MAT::IOfflineStorage {
  public:
    MockIOfflineStorage();
    virtual ~MockIOfflineStorage();

    MOCK_METHOD1(Initialize, void(MAT::IOfflineStorageObserver & observer));
    MOCK_METHOD0(Shutdown, void());
    MOCK_METHOD0(Flush, void());
    MOCK_METHOD1(StoreRecord, bool(MAT::StorageRecord const &));
    MOCK_METHOD1(StoreRecords, size_t(std::vector<MAT::StorageRecord> &));
    MOCK_METHOD4(GetAndReserveRecords, bool(std::function<bool(MAT::StorageRecord&&)> const &, unsigned, MAT::EventLatency, unsigned));
    MOCK_METHOD0(IsLastReadFromMemory, bool());
    MOCK_METHOD0(LastReadRecordCount, unsigned());
    MOCK_METHOD3(DeleteRecords, void(std::vector<MAT::StorageRecordId> const &, MAT::HttpHeaders, bool& ));
    MOCK_METHOD4(ReleaseRecords, void(std::vector<MAT::StorageRecordId> const &, bool, MAT::HttpHeaders, bool&));
    MOCK_METHOD2(StoreSetting, bool(std::string const &, std::string const &));
    MOCK_METHOD1(GetSetting, std::string(std::string const &));
    MOCK_METHOD0(GetSize, size_t());
    MOCK_METHOD1(DeleteRecords, void(const std::map<std::string, std::string> &));
    MOCK_CONST_METHOD1(GetRecordCount, size_t(MAT::EventLatency));
    MOCK_METHOD3(GetRecords, std::vector<MAT::StorageRecord>(bool, MAT::EventLatency, unsigned));
	MOCK_METHOD0(ResizeDb, bool());
};


} // namespace testing
