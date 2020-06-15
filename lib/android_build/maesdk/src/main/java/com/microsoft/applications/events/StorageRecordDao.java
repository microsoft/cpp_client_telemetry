package com.microsoft.applications.events;

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
    public abstract void insertRecords(StorageRecord... records);

    @Query("SELECT count(*) from StorageRecord WHERE latency = :latency")
    public abstract long recordCount(int latency);

    @Query("SELECT count(*) from StorageRecord")
    public abstract long totalRecordCount();

    @Query("DELETE FROM StorageRecord")
    public abstract int deleteAllRecords();

    @Query(value = "SELECT sum(length(id)) + sum(length(tenantToken)) + sum(length(blob)) + 32*count(*) from StorageRecord;")
    public abstract long totalSize();

    @Query("DELETE FROM StorageRecord WHERE id IN (SELECT id FROM StorageRecord ORDER BY persistence ASC, timestamp ASC LIMIT :count)")
    public abstract int trim(long count);

    @Query("SELECT * FROM StorageRecord WHERE latency >= :minLatency ORDER BY latency DESC, persistence DESC, timestamp ASC LIMIT :limit")
    public abstract StorageRecord[] getRecords(int minLatency, long limit);

    @Query("SELECT * FROM StorageRecord WHERE latency >= :minLatency AND reservedUntil = 0 ORDER BY latency DESC, persistence DESC, timestamp ASC LIMIT :limit")
    public abstract StorageRecord[] getUnreservedRecords(int minLatency, long limit);

    @Transaction
    @Query("DELETE FROM StorageRecord WHERE id IN (:ids)")
    public abstract int deleteByIdBlock(String[] ids);

    @Query("UPDATE StorageRecord SET reservedUntil = :until WHERE id IN (:ids)")
    public abstract int setReservedBlock(String[] ids, long until);

    @Query("SELECT * FROM StorageRecord WHERE id IN (:ids) AND retryCount >= :maximumRetries")
    public abstract StorageRecord[] getRetryExpired(String[] ids, long maximumRetries);

    @Delete
    public abstract int deleteRecordInner(StorageRecord[] records);

    @Query("UPDATE StorageRecord SET reservedUntil = 0, retryCount = retryCount + 1 WHERE id IN (:ids)")
    public abstract int releaseAndIncrementRetryCounts(String[] ids);

    @Query("SELECT min(latency) FROM StorageRecord WHERE latency >= :minLatency AND reservedUntil = 0")
    public abstract Long getMinLatency(long minLatency);

    @Query("SELECT * FROM StorageRecord WHERE latency = :latency AND reservedUntil = 0 ORDER BY persistence DESC, timestamp ASC LIMIT :limit")
    public abstract StorageRecord[] getUnreservedByLatency(long latency, long limit);

    @Query("UPDATE StorageRecord SET reservedUntil = 0 WHERE reservedUntil > 0 AND reservedUntil < :now")
    public abstract int releaseExpired(long now);

    static protected final int idCount = 64;

    @Transaction
    public int deleteById(String[] ids)
    {
        int deleted = 0;
        for (int i = 0; i < ids.length; i += idCount) {
            int count = Math.min(idCount, ids.length - i);
            String[] block = new String[count];
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
        String[] ids = new String[selected.length];
        for (int j = 0; j < selected.length; ++j) {
            ids[j] = selected[j].id;
        }
        setReserved(ids, until);
        return selected;
    }

    public void releaseUnconsumed(StorageRecord[] selected, int index)
    {
        int unconsumed = selected.length - index;
        String[] ids = new String[unconsumed];
        int j;
        for (j = 0; j < unconsumed; ++j) {
            ids[j] = selected[j].id;
        }
        setReserved(ids, 0);
    }

    @Transaction
    public void setReserved(String[] ids, long until)
    {
        for (int i = 0; i < ids.length; i += idCount) {
            int count = Math.min(ids.length - i, idCount);
            String[] block = new String[count];
            for (int j = 0; j < count; ++j) {
                block[j] = ids[i + j];
            }
            setReservedBlock(block, until);
        }
    }

    @Transaction
    public long releaseRecords(
        String[] ids,
        boolean incrementRetry,
        long maximumRetries,
        TreeMap<String, Long> byTenant
    )
    {
        long deleted = 0;
        if (incrementRetry) {
            for (int i = 0; i < ids.length; i += idCount) {
                int count = Math.min(ids.length - i, idCount);
                String[] block = new String[count];
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
