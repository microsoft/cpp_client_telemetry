#include "mat/config.h"
#ifdef HAVE_MAT_STORAGE
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "IOfflineStorage.hpp"
#include "pal/PAL.hpp"

#include "api/IRuntimeConfig.hpp"

#include "ILogManager.hpp"

#include <atomic>
#include <memory>
#include <mutex>

#include <jni.h>

#define ENABLE_LOCKING  // Enable DB locking for flush

namespace MAT_NS_BEGIN
{
    class OfflineStorage_Room : public IOfflineStorage
    {
       protected:
        class ConnectedEnv
        {
            JNIEnv* env = nullptr;
            JavaVM* vm = nullptr;
            size_t push_count = 0;
            static std::mutex s_envValuesMutex;
            static std::map<JNIEnv*, size_t> s_envValues;

           public:
            ConnectedEnv() = delete;
            ConnectedEnv(JavaVM* vm_);
            ~ConnectedEnv();

            bool operator!() const
            {
                return !env;
            }

            void pushLocalFrame(uint32_t frameSize);

            void popLocalFrame();

            JNIEnv* operator->() const
            {
                return env;
            }

            JNIEnv* getInner() const
            {
                return env;
            }
        };

       public:
        OfflineStorage_Room(ILogManager& logManager, IRuntimeConfig& runtimeConfig);
        OfflineStorage_Room() = delete;

        virtual ~OfflineStorage_Room();
        void Initialize(IOfflineStorageObserver& observer) override;
        void Shutdown() override;
        void Flush() override{};
        bool StoreRecord(StorageRecord const& record) override;
        size_t StoreRecords(StorageRecordVector& records) override;
        bool GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventLatency minLatency = EventLatency_Normal, unsigned maxCount = 0) override;
        bool IsLastReadFromMemory() override;
        unsigned LastReadRecordCount() override;

        void DeleteRecords(const std::map<std::string, std::string>& whereFilter) override;
        void DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders, bool&) override;
        virtual void DeleteAllRecords() override;
        void ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders, bool&) override;

        bool StoreSetting(std::string const& name, std::string const& value) override;
        std::string GetSetting(std::string const& name) override;
        bool DeleteSetting(std::string const& name) override;
        size_t GetSize() override;
        size_t GetRecordCount(EventLatency latency) const override;
        StorageRecordVector GetRecords(bool shutdown, EventLatency minLatency = EventLatency_Normal, unsigned maxCount = 0) override;
        bool ResizeDb() override;

        static void ConnectJVM(JNIEnv* env, jobject appContext);

       protected:
        MATSDK_LOG_DECL_COMPONENT_CLASS();

        size_t GetSizeInternal(ConnectedEnv& env) const;
        bool ResizeDbInternal(ConnectedEnv& env);
        ILogManager& m_manager;
        IRuntimeConfig& m_config;
        IOfflineStorageObserver* m_observer;
        jobject m_room = nullptr;
        std::mutex m_resize_mutex;
        constexpr static size_t CHECK_INSERT_COUNT = 1000;
        size_t m_size_limit = 3 * 1024 * 1024;  // 3 MB
        double m_notify_fraction = 0.75;
        uint64_t m_storageFullNotifyTime = 0;
        uint64_t m_storageFullNotifyInterval = DB_FULL_CHECK_INTERVAL_DEFAULT_MS;
        std::atomic<size_t> m_checkAfterInsertCounter;
        std::atomic<unsigned> m_lastReadCount;
        std::mutex m_jniThreadsMutex;
        std::set<JNIEnv*> m_jniThreads;
        void ThrowLogic(ConnectedEnv& env, const char* message) const;
        void ThrowRuntime(ConnectedEnv& env, const char* message) const;

        static JavaVM* s_vm;
        static jobject s_context;
    };

}
MAT_NS_END
#endif

