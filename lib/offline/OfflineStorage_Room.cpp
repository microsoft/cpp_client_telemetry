//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "OfflineStorage_Room.hpp"
#include "pal/PAL.hpp"
#include <cmath>
#include <exception>
#include <jni.h>

namespace
{
    static constexpr bool s_throwExceptions = true;
}

namespace MAT_NS_BEGIN
{
    /**
             * Java virtual machine
             * Set by ConnectJVM or the JNI connectContext methods.
             */

    JavaVM* OfflineStorage_Room::s_vm = nullptr;

    /**
             * Application context
             * The context we will use to construct the database.
             * Set by ConnectJVM or the JNI connectContext methods.
             */

    jobject OfflineStorage_Room::s_context = nullptr;

    /**
             * We start by pushing a local JNI frame of this size
             */
    constexpr static size_t INITIAL_FRAME_SIZE = 64;

    /**
             * JNI AttachCurrentThread and PushLocalFrame helper
             */

    /**
             * Constructor: attach thread, save JNIEnv pointer
             */
    OfflineStorage_Room::ConnectedEnv::ConnectedEnv(JavaVM* vm_)
    {
        vm = vm_;
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            env = nullptr;
            MATSDK_THROW(std::runtime_error("Unable to connect to Java thread"));
            return;
        }
        pushLocalFrame(INITIAL_FRAME_SIZE);
    }

    /**
             * Destructor: pop local frames on exit
             */
    OfflineStorage_Room::ConnectedEnv::~ConnectedEnv()
    {
        if (!!env && !!vm)
        {
            while (push_count != 0)
            {
                env->PopLocalFrame(nullptr);
                --push_count;
            }
        }
    }

    /**
             * call PushLocalFrame and track current depth
             */

    void
    OfflineStorage_Room::ConnectedEnv::pushLocalFrame(uint32_t frameSize)
    {
        if (env->PushLocalFrame(frameSize) == JNI_OK)
        {
            ++push_count;
        }
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
            if (s_throwExceptions)
            {
                MATSDK_THROW(std::runtime_error("Push Local Frame"));
            }
        }
    }

    /**
             * PopLocalFrame and decrement depth
             */

    void
    OfflineStorage_Room::ConnectedEnv::popLocalFrame()
    {
        try
        {
            if (push_count > 0)
            {
                env->PopLocalFrame(nullptr);
                --push_count;
            }
        }
        catch (std::exception e)
        {
            LOG_ERROR("Exception in popLocalFrame");
        }
    }

    /**
             * Drop-in replacement for OfflineStorage_SQLite.
             *
             * @param[in] logManager Send DebugEvent here
             * @param[in] runtimeConfig Configuration (imagine that)
             */

    OfflineStorage_Room::OfflineStorage_Room(ILogManager& logManager,
                                             IRuntimeConfig& runtimeConfig) :
        m_manager(logManager), m_config(runtimeConfig)
    {
        m_checkAfterInsertCounter.store(CHECK_INSERT_COUNT);
        m_lastReadCount.store(0);
        m_size_limit = m_config[CFG_INT_CACHE_FILE_SIZE];
        int percent = m_config[CFG_INT_STORAGE_FULL_PCT];
        if (percent <= 0 || percent >= 150)
        {
            percent = DB_FULL_NOTIFICATION_DEFAULT_PERCENTAGE;
        }
        m_storageFullNotifyInterval = m_config[CFG_INT_STORAGE_FULL_CHECK_TIME];
        m_notify_fraction = static_cast<double>(percent) / 100.0;
    }

    /**
             * On destruction, call the Closeable close() method on Java OfflineRoom.
             */

    OfflineStorage_Room::~OfflineStorage_Room()
    {
        if (s_vm && m_room)
        {
            try
            {
                ConnectedEnv env(s_vm);

                auto roomClass = env->GetObjectClass(m_room);
                auto closeId = env->GetMethodID(roomClass, "close", "()V");
                if (env->ExceptionCheck() == JNI_TRUE)
                {
                    env->ExceptionDescribe();
                    env->ExceptionClear();
                }
                else
                {
                    // We must call the close() method on the
                    // database before we drop our reference
                    env->CallVoidMethod(m_room, closeId);
                    if (env->ExceptionCheck() == JNI_TRUE)
                    {
                        env->ExceptionDescribe();
                        env->ExceptionClear();
                    }
                }
                env->DeleteGlobalRef(m_room);
                env->ExceptionClear();
            }
            catch (std::logic_error& e)
            {
                // just swallow the error
            }
            m_room = nullptr;
        }
    }

    /**
             * Connect Java VM and application context for later use in reverse-JNI
             *
             * There is a static native method connectContext on the Java class OfflineRoom which will call
             * this static method, as a convenient way to pass the application context to this method.
             *
             * @param [in] env: JNI environment.
             * @param [in] appContext: Context that will own database (usually application Context)
             */
    void OfflineStorage_Room::ConnectJVM(JNIEnv* env, jobject appContext)
    {
        if (env->GetJavaVM(&s_vm) != JNI_OK)
        {
            s_vm = nullptr;
            env->ExceptionDescribe();
            env->ExceptionClear();
            throw std::runtime_error("Unable to acquire JavaVM pointer");
            return;
        }
        s_context = env->NewGlobalRef(appContext);
    }

    /**
             * Delete all records
             *
             * Not Implemented
             */
    void OfflineStorage_Room::DeleteAllRecords()
    {
        MATSDK_THROW(std::logic_error("DeleteAllRecords not implemented"));
    }

    /**
             * Delete records matching a set of WHERE equality conditions
             *
             * Only implements equality-on-tenantToken.
             * @param[in] whereFilter The keys are column selectors (only tenant_token
             * is supported). The corresponding value specifies the column value
             * to match.
             */
    void OfflineStorage_Room::DeleteRecords(
        const std::map<std::string, std::string>& whereFilter)
    {
        using Filter = std::map<std::string, std::string>;
        ConnectedEnv env(s_vm);
        Filter::const_iterator token = whereFilter.find("tenant_token");
        if (whereFilter.size() != 1 || token == whereFilter.cend())
        {
            MATSDK_THROW(std::logic_error("whereFilter not implemented"));
        }

        auto room_class = env->GetObjectClass(m_room);
        auto deleteByToken = env->GetMethodID(room_class,
                                              "deleteByToken",
                                              "(Ljava/lang/String;)J");
        ThrowLogic(env, "dbt method");
        auto jToken = env->NewStringUTF(token->second.c_str());
        ThrowRuntime(env, "dbt token");
        env->CallLongMethod(m_room, deleteByToken, jToken);
    }

    /**
             * Delete records by identifier.
             *
             * @param[in] ids A vector of std::string record ids.
             * @param[out] fromMemory Always false (even when the database
             * is held in memory, which can happen in tests).
             */
    void OfflineStorage_Room::DeleteRecords(
        std::vector<StorageRecordId> const& ids,
        HttpHeaders,
        bool& fromMemory)
    {
        try
        {
            fromMemory = false;
            if (ids.empty())
            {
                return;
            }
            ConnectedEnv env(s_vm);
            if (!env)
            {
                return;
            }
            auto room_class = env->GetObjectClass(m_room);
            auto method = env->GetMethodID(room_class, "deleteById", "([J)J");
            ThrowLogic(env, "Unable to get deleteById method");
            size_t index = 0;

            /* Convert string identifiers to int64_t */

            env.pushLocalFrame(32);
            std::vector<jlong> roomIds;
            roomIds.reserve(ids.size());
            for (auto& id : ids)
            {
                long long n = 0;
                try
                {
                    n = std::stoll(id);
                    if (n > 0)
                    {
                        roomIds.push_back(n);
                    }
                }
                catch (std::out_of_range e)
                {
                    m_observer->OnStorageFailed("ID out of range");
                }
                catch (std::invalid_argument e)
                {
                    m_observer->OnStorageFailed("Empty ID");
                }
            }
            if (roomIds.empty())
            {
                return;
            }
            // Convert to java array, call OfflineRoom.deleteById
            auto ids_java = env->NewLongArray(roomIds.size());
            ThrowRuntime(env, "Unable to allocate id array");
            env->SetLongArrayRegion(ids_java, 0, roomIds.size(), roomIds.data());
            ThrowLogic(env, "set delete ids");
            env->CallLongMethod(m_room, method, ids_java);
            ThrowRuntime(env, "deleteById");
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in DeleteRecords: %s", what);
            // do nothing more; no recovery
        }
    }

    /**
             * Get records for the packager
             *
             * If leaseTime is non-zero, this will set the reservedUntil
             * column in the selected
             * records. This method only selects records with a
             * reservedUntil value in the past, so setting the column reserves the
             * record and prevents later calls to GetAndReserveRecords
             * from picking up and retransmitting the reserved records.
             *
             * Because we retrieve the records in a batch, if the consumer
             * functor returns false before the end of the batch, we
             * will reset reservedUntil on the unconsumed records.
             *
             * Records are sorted by:
             * - Latency (Latency_RealTime first)
             * - Persistence (Critical first)
             * - Time (oldest first)
             *
             * @param[in] consumer We call this functor for each
             * record, and it may move or copy the record as it likes. If
             * the functor returns false, we assume that it did not
             * consume the record (and so we will not reserve it), and
             * we will not call the functor again.
             * @param[in] leaseTimeMs How long to reserve the records
             * (in milliseconds). If zero, we do not reserve
             * the records.
             * @param[in] minLatency The lowest latency we will select.
             * @param[in] maxCount The maximum number of records to select
             * (and thus the maximum number of times we will call the
             * functor).
             * @return true for success. Could return false for failures, but
             * this implementation does not.
             */
    bool OfflineStorage_Room::GetAndReserveRecords(
        std::function<bool(StorageRecord&&)> const& consumer,
        unsigned leaseTimeMs,
        EventLatency minLatency,
        unsigned maxCount)
    {
        constexpr int64_t chunkSize = 1024;
        int64_t requested = maxCount ? maxCount : INT64_MAX;
        try
        {
            ConnectedEnv env(s_vm);
            if (!env)
            {
                return false;
            }
            auto room_class = env->GetObjectClass(m_room);
            auto reserve = env->GetMethodID(room_class, "getAndReserve",
                                            "(IJJJ)[Lcom/microsoft/applications/events/StorageRecord;");
            ThrowLogic(env, "getAndReserve");
            auto now = PAL::getUtcSystemTimeMs();
            auto until = now + leaseTimeMs;
            int64_t collected = 0;  // Signed because JNI likes signed numbers
            while (requested > collected)
            {
                int64_t loopChunk = std::min(chunkSize, requested - collected);
                auto selected = static_cast<jobjectArray>(env->CallObjectMethod(m_room,
                                                                                reserve,
                                                                                static_cast<int>(minLatency),
                                                                                loopChunk,
                                                                                static_cast<int64_t>(now),
                                                                                static_cast<int64_t>(until)));
                ThrowRuntime(env, "Call getAndReserve");
                size_t index;
                size_t limit = env->GetArrayLength(selected);
                if (limit == 0)
                {
                    break;  // out of r > c loop; no more records
                }
                // we don't collect these here because GetObjectClass is
                // less fragile than FindClass
                jclass record_class = nullptr;
                jfieldID id_id;
                jfieldID tenantToken_id;
                jfieldID latency_id;
                jfieldID persistence_id;
                jfieldID timestamp_id;
                jfieldID retryCount_id;
                jfieldID reservedUntil_id;
                jfieldID blob_id;

                // Set limits for conversion from int to enum
                int latency_lb = static_cast<int>(EventLatency_Off);
                int latency_ub = static_cast<int>(EventLatency_Max);
                int persist_lb = static_cast<int>(EventPersistence_Normal);
                int persist_ub = static_cast<int>(EventPersistence_DoNotStoreOnDisk);

                for (index = 0; index < limit; ++index)
                {
                    env.pushLocalFrame(32);
                    auto record = env->GetObjectArrayElement(selected, index);
                    ThrowLogic(env, "getAndReserve element");
                    if (!record_class)
                    {
                        record_class = env->GetObjectClass(record);
                        id_id = env->GetFieldID(record_class, "id", "J");
                        ThrowLogic(env, "gar id");
                        tenantToken_id = env->GetFieldID(record_class, "tenantToken",
                                                         "Ljava/lang/String;");
                        ThrowLogic(env, "gar tenant");
                        latency_id = env->GetFieldID(record_class, "latency", "I");
                        ThrowLogic(env, "gar latency");
                        persistence_id = env->GetFieldID(record_class, "persistence", "I");
                        ThrowLogic(env, "gar persistence");
                        timestamp_id = env->GetFieldID(record_class, "timestamp", "J");
                        ThrowLogic(env, "gar timestamp");
                        retryCount_id = env->GetFieldID(record_class, "retryCount", "I");
                        ThrowLogic(env, "gar retryCount");
                        reservedUntil_id = env->GetFieldID(record_class, "reservedUntil",
                                                           "J");
                        ThrowLogic(env, "gar reserved");
                        blob_id = env->GetFieldID(record_class, "blob", "[B");
                        ThrowLogic(env, "gar blob");
                    }

                    auto id_java = env->GetLongField(record, id_id);
                    ThrowLogic(env, "get id");
                    auto tenantToken_java = static_cast<jstring>(env->GetObjectField(record,
                                                                                     tenantToken_id));
                    ThrowRuntime(env, "get tenant");
                    auto token_utf = env->GetStringUTFChars(tenantToken_java, nullptr);
                    ThrowRuntime(env, "string tenant");
                    auto latency = static_cast<EventLatency>(std::max(latency_lb,
                                                                      std::min<int>(
                                                                          latency_ub,
                                                                          env->GetIntField(
                                                                              record,
                                                                              latency_id))));
                    ThrowLogic(env, "get latency");
                    auto persistence = static_cast<EventPersistence>(std::max(latency_lb,
                                                                              std::min<int>(
                                                                                  latency_ub,
                                                                                  env->GetIntField(
                                                                                      record,
                                                                                      persistence_id))));
                    ThrowLogic(env, "get persistence");
                    auto timestamp = static_cast<uint64_t>(env->GetLongField(record,
                                                                             timestamp_id));
                    ThrowLogic(env, "get timestamp");
                    auto retryCount = env->GetIntField(record, retryCount_id);
                    ThrowLogic(env, "get retry");
                    auto reservedUntil = static_cast<uint64_t>(env->GetLongField(record,
                                                                                 reservedUntil_id));
                    ThrowLogic(env, "get reservedUntil");
                    auto blob_java = static_cast<jbyteArray>(env->GetObjectField(record,
                                                                                 blob_id));
                    ThrowLogic(env, "get blob");
                    uint8_t* start = reinterpret_cast<uint8_t*>(env->GetByteArrayElements(
                        blob_java,
                        nullptr));
                    ThrowLogic(env, "get blob storage");
                    uint8_t* end = start + env->GetArrayLength(blob_java);
                    StorageRecord dest(
                        std::to_string(id_java),
                        token_utf,
                        latency,
                        persistence,
                        timestamp,
                        StorageBlob(start, end),
                        retryCount,
                        reservedUntil);
                    env->ReleaseStringUTFChars(tenantToken_java, token_utf);
                    env->ReleaseByteArrayElements(blob_java,
                                                  reinterpret_cast<jbyte*>(start), 0);
                    env.popLocalFrame();
                    if (!consumer(std::move(dest)))
                    {
                        break;
                    }
                    collected += 1;
                }
                if (index < limit)
                {
                    // we did not consume all these events
                    auto release = env->GetMethodID(room_class, "releaseUnconsumed",
                                                    "([Lcom/microsoft/applications/events/StorageRecord;I)V");
                    ThrowLogic(env, "releaseUnconsumed");
                    env->CallVoidMethod(m_room, release, selected, static_cast<int>(index));
                    ThrowRuntime(env, "call ru");
                    break;  // break out of the request > collected loop--end early by request
                }
            }
            m_lastReadCount.store(std::min(collected, static_cast<int64_t>(INT32_MAX)));
            return collected > 0;
        }
        catch (const std::runtime_error& e)
        {
            auto what = e.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in GetAndReserveRecords: %s", what);
            return false;
        }
    }

    /**
             * Initialize the database (and instantiate our androidx Room objects).
             *
             * The ConnectJVM method must be called before this method to set up our
             * connection to the JVM and the desired Context (usually the
             * application context).
             *
             * @param[in] observer In practice, an instance of StorageObserver. We
             * communicate significant events back to this IOfflineStorageObserver.
             */
    void OfflineStorage_Room::Initialize(IOfflineStorageObserver& observer)
    {
        static constexpr char k_init_string[] = "Room/Init";

        m_observer = &observer;
        try
        {
            ConnectedEnv env(s_vm);
            if (!env)
            {
                return;
            }
            auto db_name = static_cast<const char*>(m_config[CFG_STR_CACHE_FILE_PATH]);
            if (!db_name || !*db_name)
            {
                db_name = "MAEvents";
            }
            static constexpr char room_class_name[] = "com/microsoft/applications/events/OfflineRoom";
            auto room_class = env->FindClass(room_class_name);
            ThrowLogic(env, "room class");
            auto constructor = env->GetMethodID(room_class, "<init>",
                                                "(Landroid/content/Context;Ljava/lang/String;)V");
            ThrowLogic(env, "No constructor for OfflineRoom");
            auto java_db_name = env->NewStringUTF(db_name);
            ThrowRuntime(env, "Failed to create db_name string");
            auto local_room = env->NewObject(room_class, constructor, s_context,
                                             java_db_name);
            ThrowRuntime(env, "Exception constructing OfflineRoom");
            // we take a global reference on our instance of OfflineRoom, since we want
            // it to stay alive across JNI calls.
            m_room = env->NewGlobalRef(local_room);
            ThrowRuntime(env, "Exception creating global ref to OfflineRoom");
            m_observer->OnStorageOpened(k_init_string);
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in Initialize: %s", what);
        }
    }

    /**
             * Was the last read from MemoryStorage? No. Always returns false.
             */
    bool OfflineStorage_Room::IsLastReadFromMemory()
    {
        return false;
    }

    /**
             * How many records were accepted by the functor in GetAndReserveRecords().
             */
    unsigned OfflineStorage_Room::LastReadRecordCount()
    {
        return m_lastReadCount;
    }

    /**
             * Release lease and optionally increment retry count for records.
             *
             * @param[in] ids Vector of StorageRecord ids to release.
             * @param[in] incrementRetryCount True if we should increment the retryCount column
             * for these records.
             *
             */
    void OfflineStorage_Room::ReleaseRecords(
        std::vector<StorageRecordId> const& ids,
        bool incrementRetryCount,
        HttpHeaders,
        bool&)
    {
        if (ids.empty())
        {
            return;
        }
        try
        {
            ConnectedEnv env(s_vm);
            auto room_class = env->GetObjectClass(m_room);
            ThrowLogic(env, "GetObjectClass for m_room");
            auto release = env->GetMethodID(room_class,
                                            "releaseRecords",
                                            "([JZJ)[Lcom/microsoft/applications/events/ByTenant;");
            ThrowLogic(env, "Exception finding releaseRecords");
            int64_t maximumRetries = 0;
            if (incrementRetryCount)
            {
                maximumRetries = m_config.GetMaximumRetryCount();
            }
            std::vector<jlong> roomIds;
            roomIds.reserve(ids.size());
            for (auto const& id : ids)
            {
                try
                {
                    long long roomId = std::stoll(id);
                    if (roomId > 0)
                    {
                        roomIds.push_back(roomId);
                    }
                }
                catch (std::out_of_range e)
                {
                    m_observer->OnStorageFailed("id out of range");
                }
                catch (std::invalid_argument e)
                {
                    m_observer->OnStorageFailed("id empty");
                }
            }
            if (roomIds.empty())
            {
                return;
            }
            auto ids_java = env->NewLongArray(roomIds.size());
            ThrowRuntime(env, "ids_java");
            env->SetLongArrayRegion(ids_java, 0, roomIds.size(), roomIds.data());
            ThrowLogic(env, "ids_java");
            jobjectArray results = static_cast<jobjectArray>(
                env->CallObjectMethod(
                    m_room,
                    release,
                    ids_java,
                    incrementRetryCount,
                    maximumRetries));
            ThrowRuntime(env, "Exception in releaseRecords");
            size_t tokens = 0;
            if (!!results)
            {
                tokens = env->GetArrayLength(results);
            }
            if (tokens > 0)
            {
                DroppedMap dropped;
                jclass bt_class = nullptr;
                jfieldID token_id;
                jfieldID count_id;
                for (size_t index = 0; index < tokens; ++index)
                {
                    env.pushLocalFrame(8);
                    auto byTenant = env->GetObjectArrayElement(results, index);
                    ThrowRuntime(env, "Exception fetching element from results");
                    if (!bt_class)
                    {
                        bt_class = env->GetObjectClass(byTenant);
                        token_id = env->GetFieldID(bt_class, "tenantToken",
                                                   "Ljava/lang/String;");
                        ThrowLogic(env, "Error fetching tenantToken field id");
                        count_id = env->GetFieldID(bt_class, "count", "J");
                        ThrowLogic(env, "Error fetching count field id");
                    }
                    jstring token = static_cast<jstring>(env->GetObjectField(byTenant,
                                                                             token_id));
                    ThrowLogic(env, "Exception fetching token");
                    auto count = env->GetLongField(byTenant, count_id);
                    ThrowLogic(env, "Exception fetching count");
                    auto utf = env->GetStringUTFChars(token, nullptr);
                    std::string key(utf);
                    env->ReleaseStringUTFChars(token, utf);
                    dropped[key] = static_cast<size_t>(count);
                    env.popLocalFrame();
                }
                m_observer->OnStorageRecordsDropped(dropped);
            }
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in ReleaseRecords", what);
        }
    }

    /**
             * We do nothing for Shutdown() (our destructor closes the database)
             */
    void OfflineStorage_Room::Shutdown()
    {
    }

    /**
             * Store a single record.
             *
             * This makes a reverse-JNI call, so
             * the StoreRecords() method with a batch of records will be
             * more efficient.
             *
             * @param[in] record StorageRecord to persist.
             * @return true if we stored the record.
             */

    bool OfflineStorage_Room::StoreRecord(StorageRecord const& record)
    {
        StorageRecordVector records;
        records.push_back(record);
        return StoreRecords(records) > 0;
    }

    /**
             * Store a std::vector of records.
             *
             * We ignore the id on these records. SQLite will assign a unique
             * row id to each record we persist, and we will return that
             * whenever we retrieve records.
             *
             * @param[in] records The records to be persisted.
             * @return the number of records we persisted.
             */

    size_t OfflineStorage_Room::StoreRecords(StorageRecordVector& records)
    {
        if (records.size() == 0)
        {
            return 0;
        }

        try
        {
            ConnectedEnv env(s_vm);
            if (!env)
            {
                return 0;
            }

            static constexpr char newRecordSignature[] =
                "(JIIJIJ[B)Lcom/microsoft/applications/events/StorageRecord;";
            auto room_class = env->GetObjectClass(m_room);

            size_t count = std::min<size_t>(records.size(), INT32_MAX);
            env.pushLocalFrame(8);
            size_t buffer_size = 0;
            for (auto const& record : records)
            {
                buffer_size += record.tenantToken.size() + record.blob.size();
            }
            if (buffer_size >= UINT32_MAX)
            {
                MATSDK_THROW(std::runtime_error("Buffer size"));
            }
            std::vector<jbyte> buffer;  // tenantToken, blob
            std::vector<jint> indices;
            std::vector<jint> smallNumbers;  // latency, persistence, retryCount
            std::vector<jlong> bigNumbers;   // id, timestamp, reservedUntil
            buffer.reserve(buffer_size);
            indices.reserve(3 * count);
            smallNumbers.reserve(3 * count);
            bigNumbers.reserve(3 * count);

            for (auto& record : records)
            {
                indices.push_back(record.tenantToken.size());
                for (size_t i = 0; i < record.tenantToken.size(); ++i)
                {
                    buffer.push_back(record.tenantToken[i]);
                }
                indices.push_back(record.blob.size());
                for (size_t i = 0; i < record.blob.size(); ++i)
                {
                    buffer.push_back(record.blob[i]);
                }
                smallNumbers.push_back(record.latency);
                smallNumbers.push_back(record.persistence);
                smallNumbers.push_back(record.retryCount);
                bigNumbers.push_back(0);
                bigNumbers.push_back(record.timestamp);
                bigNumbers.push_back(record.reservedUntil);
            }
            auto byteBuffer = env->NewByteArray(buffer.size());
            ThrowRuntime(env, "buffer");
            env->SetByteArrayRegion(byteBuffer, 0, buffer.size(), buffer.data());
            ThrowLogic(env, "set buffer");
            auto indicesBuffer = env->NewIntArray(indices.size());
            ThrowRuntime(env, "indices");
            env->SetIntArrayRegion(indicesBuffer, 0, indices.size(), indices.data());
            auto smallBuffer = env->NewIntArray(smallNumbers.size());
            ThrowRuntime(env, "small");
            env->SetIntArrayRegion(smallBuffer, 0, smallNumbers.size(),
                                   smallNumbers.data());
            ThrowLogic(env, "set small");
            auto bigBuffer = env->NewLongArray(bigNumbers.size());
            ThrowRuntime(env, "big");
            env->SetLongArrayRegion(bigBuffer, 0, bigNumbers.size(), bigNumbers.data());
            static constexpr char storeSignature[] = "(I[I[B[I[J)V";
            auto storeId = env->GetMethodID(room_class, "storeFromBuffers", storeSignature);
            ThrowLogic(env, "store");
            env->CallVoidMethod(m_room, storeId, count, indicesBuffer, byteBuffer,
                                smallBuffer,
                                bigBuffer);
            ThrowRuntime(env, "call store");

            bool shouldCheck = false;
            auto current = m_checkAfterInsertCounter.load();
            size_t newValue;
            // atomic decrement counter
            do
            {
                if (m_checkAfterInsertCounter <= count)
                {
                    shouldCheck = true;
                    newValue = CHECK_INSERT_COUNT;
                }
                else
                {
                    shouldCheck = false;
                    newValue = current - count;
                }
            } while (!m_checkAfterInsertCounter.compare_exchange_weak(current, newValue));
            if (shouldCheck)
            {
                auto sizeEstimate = GetSizeInternal(env);
                double ratio =
                    static_cast<double>(sizeEstimate) /
                    static_cast<double>(m_size_limit);
                if (m_notify_fraction <= ratio)
                {
                    auto now = PAL::getMonotonicTimeMs();
                    if (now > m_storageFullNotifyTime + m_storageFullNotifyInterval)
                    {
                        m_storageFullNotifyTime = now;
                        DebugEvent evt;
                        evt.type = DebugEventType::EVT_STORAGE_FULL;
                        evt.param1 = std::max<long long int>(0,
                                                             std::llround(100.0 * ratio));
                        m_manager.DispatchEvent(evt);
                    }
                }
                if (sizeEstimate >= m_size_limit)
                {
                    ResizeDbInternal(env);
                }
            }
            return count;
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in StoreRecords: %s", what);
            return 0;
        }
    }

    /**
             * Delete one setting (helper for StoreSetting)
             *
             * @param[in] name The key to delete from the database.
             */

    bool OfflineStorage_Room::DeleteSetting(std::string const& name)
    {
        try
        {
            ConnectedEnv env(s_vm);
            auto room_class = env->GetObjectClass(m_room);
            auto delete_method = env->GetMethodID(room_class, "deleteSetting",
                                                  "(Ljava/lang/String;)V");
            ThrowLogic(env, "delete one setting");
            auto jName = env->NewStringUTF(name.c_str());
            ThrowRuntime(env, "newstring");
            env->CallVoidMethod(m_room, delete_method, jName);
            ThrowLogic(env, "exception in delete setting");
            return true;
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in DeleteSetting: %s", what);
            return false;
        }
    }

    /**
             * Store a value into our key-value Setting database
             *
             * @param[in] name Key.
             * @param[in] value Value.
             * @return true if we persisted the key-value pair.
             */

    bool
    OfflineStorage_Room::StoreSetting(std::string const& name, std::string const& value)
    {
        if (value.size() == 0)
        {
            return DeleteSetting(name);
        }
        try
        {
            ConnectedEnv env(s_vm);
            auto room_class = env->GetObjectClass(m_room);
            jmethodID store_setting = env->GetMethodID(
                room_class,
                "storeSetting",
                "(Ljava/lang/String;Ljava/lang/String;)J");
            ThrowLogic(env, "method storeSetting");
            env.pushLocalFrame(8);

            auto java_name = env->NewStringUTF(name.c_str());
            ThrowRuntime(env, "setting name string");
            auto java_value = env->NewStringUTF(value.c_str());
            ThrowRuntime(env, "setting value string");
            auto count = env->CallLongMethod(m_room, store_setting, java_name, java_value);
            ThrowRuntime(env, "Exception StoreSetting");
            return (count == 1);
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in StoreSetting: %s", what);
            return false;
        }
    }

    /**
             * Retrieve value from key-value store.
             *
             * Returns empty string if the key is not in the database, or we
             * encounter errors.
             *
             * @param[in] name Key.
             * @return Corresponding value.
             */

    std::string OfflineStorage_Room::GetSetting(std::string const& name)
    {
        if (!s_vm || !m_room)
        {
            return "";
        }
        try
        {
            ConnectedEnv env(s_vm);
            if (!env)
            {
                return "";
            }
            auto room_class = env->GetObjectClass(m_room);
            auto get_method = env->GetMethodID(room_class, "getSetting",
                                               "(Ljava/lang/String;)Ljava/lang/String;");
            ThrowLogic(env, "getSetting method");
            env.pushLocalFrame(8);
            auto java_name = env->NewStringUTF(name.c_str());
            ThrowRuntime(env, "name string");
            auto java_value = static_cast<jstring>(
                env->CallObjectMethod(m_room, get_method, java_name));
            ThrowRuntime(env, "Exception getSetting");
            std::string result;
            if (java_value)
            {
                auto utf = env->GetStringUTFChars(java_value, nullptr);
                ThrowRuntime(env, "copy setting value");
                result = utf;
                env->ReleaseStringUTFChars(java_value, utf);
            }
            return result;
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in GetSetting: %s", what);
            return "";
        }
    }

    /**
             * @return The total size of the database in bytes
             */

    size_t OfflineStorage_Room::GetSize()
    {
        try
        {
            ConnectedEnv env(s_vm);
            if (!env)
            {
                return 0;
            }
            return GetSizeInternal(env);
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in GetSetting: %s", what);
            return 0;
        }
    }

    /**
             * @param[in] env Our caller's ConnectedEnv.
             * @returns The total size of the database in bytes.
             */

    size_t OfflineStorage_Room::GetSizeInternal(ConnectedEnv& env) const
    {
        auto room_class = env->GetObjectClass(m_room);
        auto method = env->GetMethodID(room_class, "totalSize", "()J");
        if (!method)
        {
            return 0;
        }
        return env->CallLongMethod(m_room, method);
    }

    /**
             * Number of records (StorageRecord) for a latency (or all latencies)
             *
             * @param[in] latency Desired latency (or EventLatency_Unspecified to get
             * the total count for all latencies).
             * @return number of records.
             */

    size_t OfflineStorage_Room::GetRecordCount(EventLatency latency) const
    {
        try
        {
            ConnectedEnv env(s_vm);
            if (!env)
            {
                return 0;
            }
            auto room_class = env->GetObjectClass(m_room);
            auto count_id = env->GetMethodID(room_class, "getRecordCount", "(I)J");
            ThrowLogic(env, "getRecordCount");
            auto count = env->CallLongMethod(m_room, count_id, static_cast<int>(latency));
            return count;
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in GetRecordCount: %s", what);
            return 0;
        }
    }

    bool OfflineStorage_Room::ResizeDb()
    {
        try
        {
            ConnectedEnv env(s_vm);
            if (!env)
            {
                return false;
            }
            bool result = ResizeDbInternal(env);
            return result;
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in ResizeDb: %s", what);
            return false;
        }
    }

    bool OfflineStorage_Room::ResizeDbInternal(ConnectedEnv& env)
    {
        std::lock_guard<std::mutex> lock(m_resize_mutex);
        if (!env)
        {
            return false;
        }
        auto limit = static_cast<uint32_t>(m_config.GetOfflineStorageMaximumSizeBytes());
        auto current = GetSizeInternal(env);
        if (current < limit)
        {
            return false;
        }
        auto room_class = env->GetObjectClass(m_room);
        auto trim_id = env->GetMethodID(room_class, "trim", "(J)J");
        ThrowLogic(env, "trim");
        size_t dropped = env->CallLongMethod(m_room, trim_id, static_cast<jlong>(limit));

        DebugEvent evt(DebugEventType::EVT_DROPPED);
        evt.param1 = dropped;
        evt.size = dropped;
        m_manager.DispatchEvent(evt);

        return true;
    }

    StorageRecordVector OfflineStorage_Room::GetRecords(
        bool shutdown,
        EventLatency minLatency,
        unsigned int maxCount)
    {
        int64_t limit = maxCount;
        if (limit == 0)
        {
            limit = -1;
        }
        StorageRecordVector records;

        try
        {
            ConnectedEnv env(s_vm);

            auto room_class = env->GetObjectClass(m_room);
            auto method = env->GetMethodID(room_class, "getRecords",
                                           "(ZIJ)[Lcom/microsoft/applications/events/StorageRecord;");
            ThrowLogic(env, "getRecords method");

            jclass record_class = nullptr;
            jfieldID id_id = nullptr;
            jfieldID tenantToken_id;
            jfieldID latency_id;
            jfieldID persistence_id;
            jfieldID timestamp_id;
            jfieldID retryCount_id;
            jfieldID reservedUntil_id;
            jfieldID blob_id;

            auto java_records = static_cast<jobjectArray>(env->CallObjectMethod(m_room,
                                                                                method,
                                                                                shutdown,
                                                                                static_cast<jint>(minLatency),
                                                                                limit));
            ThrowRuntime(env, "call getRecords");
            auto result_count = env->GetArrayLength(java_records);
            records.reserve(result_count);
            for (size_t record_index = 0; record_index < result_count; ++record_index)
            {
                env.pushLocalFrame(64);
                auto record = env->GetObjectArrayElement(java_records, record_index);
                ThrowLogic(env, "access result element");
                if (!record_class)
                {
                    record_class = env->GetObjectClass(record);
                    id_id = env->GetFieldID(record_class, "id", "J");
                    ThrowLogic(env, "id field");
                    tenantToken_id = env->GetFieldID(record_class, "tenantToken",
                                                     "Ljava/lang/String;");
                    ThrowLogic(env, "tenant field");
                    latency_id = env->GetFieldID(record_class, "latency", "I");
                    ThrowLogic(env, "latency field");
                    persistence_id = env->GetFieldID(record_class, "persistence", "I");
                    ThrowLogic(env, "persistence field");
                    timestamp_id = env->GetFieldID(record_class, "timestamp", "J");
                    ThrowLogic(env, "timestamp field");
                    retryCount_id = env->GetFieldID(record_class, "retryCount", "I");
                    ThrowLogic(env, "retryCount field");
                    reservedUntil_id = env->GetFieldID(record_class, "reservedUntil", "J");
                    ThrowLogic(env, "reservedUntil field");
                    blob_id = env->GetFieldID(record_class, "blob", "[B");
                    ThrowLogic(env, "blob field");
                }
                StorageRecord dest;
                auto id_j = env->GetLongField(record, id_id);
                auto tenant_j = static_cast<jstring>(env->GetObjectField(record,
                                                                         tenantToken_id));
                auto tenant_utf = env->GetStringUTFChars(tenant_j, nullptr);
                auto latency = static_cast<EventLatency>(env->GetIntField(record,
                                                                          latency_id));
                auto persistence = static_cast<EventPersistence>(env->GetIntField(record,
                                                                                  persistence_id));
                uint64_t timestamp = env->GetLongField(record, timestamp_id);
                auto retryCount = env->GetIntField(record, retryCount_id);
                uint64_t reservedUntil = env->GetLongField(record, reservedUntil_id);
                jbyteArray blob_j = static_cast<jbyteArray>(env->GetObjectField(record,
                                                                                blob_id));
                auto elements = env->GetByteArrayElements(blob_j, nullptr);
                auto blob_store = reinterpret_cast<const uint8_t*>(elements);
                size_t blob_length = env->GetArrayLength(blob_j);
                auto blob_end = blob_store + blob_length;
                records.emplace_back(
                    std::to_string(id_j),
                    tenant_utf,
                    latency,
                    persistence,
                    timestamp,
                    StorageBlob(blob_store, blob_end),
                    retryCount,
                    reservedUntil);
                env->ReleaseStringUTFChars(tenant_j, tenant_utf);
                env->ReleaseByteArrayElements(blob_j, elements, 0);
                env.popLocalFrame();
            }
            return records;
        }
        catch (const std::runtime_error& error)
        {
            auto what = error.what();
            if (!what)
            {
                what = "*nothing*";
            }
            LOG_ERROR("Exception in GetRecords: %s", what);
            return records;
        }
    }

    void
    OfflineStorage_Room::ThrowLogic(OfflineStorage_Room::ConnectedEnv& env,
                                    const char* message) const
    {
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
            if (m_observer)
            {
                m_observer->OnStorageFailed(message);
            }
            if (s_throwExceptions)
            {
                MATSDK_THROW(std::logic_error(message));
            }
        }
    }

    void
    OfflineStorage_Room::ThrowRuntime(OfflineStorage_Room::ConnectedEnv& env,
                                      const char* message) const
    {
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
            if (m_observer)
            {
                m_observer->OnStorageFailed(message);
            }
            if (s_throwExceptions)
            {
                MATSDK_THROW(std::runtime_error(message));
            }
        }
    }

    MATSDK_LOG_INST_COMPONENT_CLASS(OfflineStorage_Room, "EventsSDK.RoomStorage", "Offline Storage: Android Room database")
}
MAT_NS_END

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_OfflineRoom_connectContext(
    JNIEnv* env,
    jclass /* OfflineRoom class */,
    jobject context)
{
    ::Microsoft::Applications::Events::OfflineStorage_Room::ConnectJVM(env, context);
}
