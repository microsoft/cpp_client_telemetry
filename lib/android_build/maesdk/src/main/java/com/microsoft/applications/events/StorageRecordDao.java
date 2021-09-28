//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import android.util.Log;

import androidx.room.Dao;
import androidx.room.Delete;
import androidx.room.Insert;
import androidx.room.OnConflictStrategy;
import androidx.room.Query;
import androidx.room.Transaction;

import java.util.TreeMap;

@Dao
public abstract class StorageRecordDao {
    @Insert(onConflict = OnConflictStrategy.REPLACE)
    public abstract long[] insertRecords(StorageRecord... records);

    @Query("SELECT count(*) from StorageRecord WHERE latency = :latency")
    public abstract long recordCount(int latency);

    @Query("SELECT count(*) from StorageRecord")
    public abstract long totalRecordCount();

    @Query("DELETE FROM StorageRecord")
    public abstract int deleteAllRecords();

    @Query(value = "SELECT sum(length(id)) + sum(length(tenantToken)) + sum(length(blob)) + 40*count(*) from StorageRecord;")
    public abstract long totalSize();

    @Query("DELETE FROM StorageRecord WHERE id IN (SELECT id FROM StorageRecord ORDER BY persistence ASC, timestamp ASC LIMIT :count)")
    public abstract int trim(long count);

    @Transaction
    @Query("SELECT * FROM StorageRecord WHERE latency >= :minLatency ORDER BY latency DESC, persistence DESC, timestamp ASC LIMIT :limit")
    public abstract StorageRecord[] getRecords(int minLatency, long limit);

    @Transaction
    @Query("SELECT * FROM StorageRecord WHERE latency >= :minLatency AND reservedUntil = 0 ORDER BY latency DESC, persistence DESC, timestamp ASC LIMIT :limit")
    public abstract StorageRecord[] getUnreservedRecords(int minLatency, long limit);

    @Transaction
    @Query("DELETE FROM StorageRecord WHERE id IN (:ids)")
    public abstract int deleteByIdBlock(long[] ids);

    @Query("UPDATE StorageRecord SET reservedUntil = :until WHERE id IN (:ids)")
    public abstract int setReservedBlock(long[] ids, long until);

    @Transaction
    @Query("SELECT * FROM StorageRecord WHERE id IN (:ids) AND retryCount >= :maximumRetries")
    public abstract StorageRecord[] getRetryExpired(long[] ids, long maximumRetries);

    @Delete
    public abstract int deleteRecordInner(StorageRecord[] records);

    @Query("UPDATE StorageRecord SET reservedUntil = 0, retryCount = retryCount + 1 WHERE id IN (:ids)")
    public abstract int releaseAndIncrementRetryCounts(long[] ids);

    @Query("SELECT min(latency) FROM StorageRecord WHERE latency >= :minLatency AND reservedUntil = 0")
    public abstract Long getMinLatency(long minLatency);

    @Transaction
    @Query("SELECT * FROM StorageRecord WHERE latency = :latency AND reservedUntil = 0 ORDER BY persistence DESC, timestamp ASC LIMIT :limit")
    public abstract StorageRecord[] getUnreservedByLatency(long latency, long limit);

    @Query("UPDATE StorageRecord SET reservedUntil = 0 WHERE reservedUntil > 0 AND reservedUntil < :now")
    public abstract int releaseExpired(long now);

    @Query("DELETE FROM StorageRecord WHERE tenantToken = :token")
    public abstract int deleteRecordsByToken(String token);

    static protected final int idCount = 64;

    @Transaction
    public int deleteById(long[] ids)
    {
        int deleted = 0;
        for (int i = 0; i < ids.length; i += idCount) {
            int count = Math.min(idCount, ids.length - i);
            long[] block = new long[count];
            for (int j = 0; j < count; ++j) {
                block[j] = ids[i + j];
            }
            deleted += deleteByIdBlock(block);
        }
        return deleted;
    }

    @Transaction
    public StorageRecord[] getRecords(boolean shutdown, int minLatency, long limit)
    {
        if (shutdown) {
            return getRecords(minLatency, limit);
        }
        while (true) {
            Long dbMin = getMinLatency(minLatency);
            if (dbMin == null) {
                break;
            }
            StorageRecord[] results = getUnreservedByLatency(dbMin, limit);
            if (results.length > 0) {
                return results;
            }
        }
        return new StorageRecord[0];
    }

    @Transaction
    public StorageRecord[] getAndReserve(int minLatency, long limit, long now, long until)
    {
        releaseExpired(now);
        StorageRecord[] selected = getUnreservedRecords(minLatency, limit);
        if (selected.length == 0) {
            return selected;
        }
        long[] ids = new long[selected.length];
        for (int j = 0; j < selected.length; ++j) {
            ids[j] = selected[j].id;
        }
        setReserved(ids, until);
        return selected;
    }

    public void releaseUnconsumed(StorageRecord[] selected, int index)
    {
        int unconsumed = selected.length - index;
        long[] ids = new long[unconsumed];
        int j;
        for (j = 0; j < unconsumed; ++j) {
            ids[j] = selected[j].id;
        }
        setReserved(ids, 0);
    }

    @Transaction
    public void setReserved(long[] ids, long until)
    {
        for (int i = 0; i < ids.length; i += idCount) {
            int count = Math.min(ids.length - i, idCount);
            long[] block = new long[count];
            for (int j = 0; j < count; ++j) {
                block[j] = ids[i + j];
            }
            setReservedBlock(block, until);
        }
    }

    @Transaction
    public long releaseRecords(
        long[] ids,
        boolean incrementRetry,
        long maximumRetries,
        TreeMap<String, Long> byTenant
    )
    {
        long deleted = 0;
        if (incrementRetry) {
            for (int i = 0; i < ids.length; i += idCount) {
                int count = Math.min(ids.length - i, idCount);
                long[] block = new long[count];
                for (int j = 0; j < count; ++j) {
                    block[j] = ids[i + j];
                }
                StorageRecord[] expired = getRetryExpired(block, maximumRetries);
                for (StorageRecord record : expired) {
                    Long n = byTenant.get(record.tenantToken);
                    if (n == null) {
                        n = Long.valueOf(0);
                    }
                    byTenant.put(record.tenantToken, n + 1);
                }
                deleted += deleteRecordInner(expired);
                releaseAndIncrementRetryCounts(block);
            }
        }
        else {
            setReserved(ids, 0);
        }
        return deleted;
    }
}

