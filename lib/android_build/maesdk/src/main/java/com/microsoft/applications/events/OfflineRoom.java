//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import android.content.Context;
import android.database.Cursor;
import android.util.Log;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.room.Room;
import androidx.room.RoomDatabase;

import java.util.Map;
import java.util.TreeMap;
import java.util.concurrent.Callable;

@Keep
public class OfflineRoom implements AutoCloseable {
    class TrimTransaction implements Callable<Long>
    {
        OfflineRoom m_room = null;
        long m_byteLimit = 0;

        public TrimTransaction(OfflineRoom room, long byteLimit)
        {
            m_room = room;
            m_byteLimit = byteLimit;
        }

        protected long vacuum(long pre) {
            try (Cursor c = m_room.m_db.query("VACUUM", null)) {};
            long post = m_room.totalSize();
            Log.i("MAE", String.format(
                "Vacuum: %d before, %d after",
                pre,
                post));
            return post;
        }

        public Long call()
        {
            if (m_room == null || m_byteLimit == 0) {
                return null;
            }
            long currentSize = m_room.totalSize();
            if (currentSize <= m_byteLimit) {
                return new Long(0);
            }
            long postVacuum = currentSize;
            try {
                postVacuum = vacuum(currentSize);
            } catch (Exception e) {
                Log.e("MAE", "Exception in VACUUM", e);
                postVacuum = currentSize;
            }
            if (postVacuum <= m_byteLimit) {
                return new Long(0);
            }

            long records = m_room.m_srDao.totalRecordCount();
            double fraction = 0.25; // fraction of current to be dropped
            if (m_byteLimit > m_room.m_pageSize) {
                double dLimit = m_byteLimit;
                double dCurrent = postVacuum;
                fraction = Math.max(0.25, 1.0 - (dLimit / dCurrent));
            }
            long to_drop = (long) Math.ceil(fraction * records);
            if (to_drop <= 0) {
                return new Long(0);
            }
            long recordsDropped = m_room.m_srDao.trim(to_drop);
            long postDrop = m_room.totalSize();
            long reVacuum = postDrop;
            if (postDrop > m_byteLimit) {
                reVacuum = vacuum(postDrop);
            }
            Log.i(
                "MAE", String.format(
                    "Trim: dropped %d records, new size %d bytes",
                    recordsDropped,
                    reVacuum));
            return new Long(recordsDropped);

        }
    }

    OfflineRoomDatabase m_db = null;
    StorageRecordDao m_srDao = null;
    StorageSettingDao m_settingDao = null;
    long m_pageSize = 4096;

    public OfflineRoom(Context context, String name) {
        RoomDatabase.Builder<OfflineRoomDatabase> builder;
        if (name.equals(":memory:")) {
            builder = Room.inMemoryDatabaseBuilder(context, OfflineRoomDatabase.class);
        }
        else {
            builder = Room.databaseBuilder(context, OfflineRoomDatabase.class, name);
        }
        builder.fallbackToDestructiveMigration();
        builder.setJournalMode(RoomDatabase.JournalMode.WRITE_AHEAD_LOGGING);
        m_db = builder.build();
        m_srDao = m_db.getStorageRecordDao();
        m_settingDao = m_db.getStorageSettingDao();
        try (Cursor c = m_db.query("PRAGMA page_size", null)) {
            if (c.getCount() == 1 && c.getColumnCount() == 1) {
                c.moveToFirst();
                m_pageSize = c.getLong(0);
            } else {
                Log.e("MAE",
                        String.format("Unexpected result from PRAGMA page_size: %d rows, %d columns",
                                c.getCount(),
                                c.getColumnCount()));
            }
        }
        long pageCount = -1;
        try (Cursor c = m_db.query("PRAGMA page_count", null)) {
            if (c.getCount() == 1 && c.getColumnCount() == 1) {
                c.moveToFirst();
                pageCount = c.getLong(0);
            }
        }
    }

    public long[] storeRecords(StorageRecord... records) {
        return m_srDao.insertRecords(records);
    }

    public void close()
    {
        if (m_db.isOpen()) {
            m_db.close();
        }
        m_srDao = null;
        m_settingDao = null;
        m_db = null;
    }

    public void explain(String query)
    {
        try (Cursor c = m_db.query("EXPLAIN QUERY PLAN " + query, null)) {
            int n = c.getCount();
            int m = c.getColumnCount();
            boolean noMove = c.moveToFirst();
            String[] names = c.getColumnNames();
            for (int i = 0; i < names.length; ++i) {
                Log.i("MAE", String.format("Type for column %s (%d): %d", names[i], i, c.getType(i)));
            }
            for (int j = 0; j < n; ++j) {
                if (!c.moveToPosition(j)) {
                    break;
                }
                for (int i = 0; i < m; ++i) {
                    Log.i("MAE", String.format("%d %s: %s",
                            j,
                            names[i],
                            c.getString(i)));
                }
            }
        }
    }

    public long[] storeFromBuffersIds(int count, int[] indices, byte[] bytes, int[] small, long[] big)
    {
        int byteIndex = 0;
        StorageRecord[] records = new StorageRecord[count];
        for (int i = 0; i < count; ++i) {
            long id = big[3*i];
            String tenantToken = new String(bytes, byteIndex, indices[2 * i]);
            byteIndex += indices[2 * i];
            byte[] blob = new byte[indices[2 * i + 1]];
            for (int j = 0; j < indices[2 * i + 1]; ++j) {
                blob[j] = bytes[j + byteIndex];
            }
            byteIndex += indices[2 * i + 1];
            records[i] = new StorageRecord(
                    id,
                    tenantToken,
                    small[3 * i],
                    small[3 * i + 1],
                    big[3 * i + 1],
                    small[3 * i + 2],
                    big[3 * i + 2],
                    blob);
        }
        long[] ids = storeRecords(records);
        return ids;
    }

    public void storeFromBuffers(int count, int[] indices, byte[] bytes, int[] small, long[] big)
    {
        storeFromBuffersIds(count, indices, bytes, small, big);
    }

    public long getRecordCount(int latency) {
        if (latency == StorageRecord.EventLatency_Unspecified) {
            return m_srDao.totalRecordCount();
        }
        return m_srDao.recordCount(latency);
    }

    public long trim(long byteLimit) {
        long currentSize = totalSize();
        if (currentSize <= byteLimit) {
            return 0;
        }
        Log.i("MAE","Start trim");
        TrimTransaction transaction = new TrimTransaction(this, byteLimit);
        Long recordsDropped = m_db.runInTransaction(transaction);
        if (recordsDropped == null) {
            Log.e("MAE", "Null result from trim");
            return 0;
        }
        Log.i("MAE", String.format("Dropped %d records in trim", recordsDropped));
        return recordsDropped;
    }

    public ByTenant[] releaseRecords(long[] ids, boolean incrementRetry, long maximumRetries)
    {
        TreeMap<String, Long> map = new TreeMap<String, Long>();

        m_srDao.releaseRecords(ids, incrementRetry, maximumRetries, map);
        ByTenant[] results = new ByTenant[map.size()];
        int index = 0;
        for (
            Map.Entry<String, Long> entry = map.firstEntry();
            entry != null;
            entry = map.higherEntry(entry.getKey())
        ) {
             results[index] = new ByTenant(entry.getKey(), entry.getValue());
             index += 1;
        }
        return results;
    }

    public long deleteAllRecords()
    {
        return m_srDao.deleteAllRecords();
    }

    public long storeSetting(String name, String value)
    {
        return m_settingDao.setValue(name, value);
    }

    public String getSetting(String name)
    {
        StorageSetting[] settings = m_settingDao.getValues(name);
        if (settings.length > 0) {
            return settings[0].value;
        }
        return "";
    }

    public long totalSize()
    {
        long result = 0;
        try (Cursor c = m_db.query("PRAGMA page_count", null)) {
            assert(c.getCount() == 1 && c.getColumnCount() == 1);
            c.moveToFirst();
            long pages = c.getLong(0);
            result = pages * m_pageSize;
        }
        return result;
    }

    public StorageRecord[] getRecords(boolean shutdown, int minLatency, long limit)
    {
        return m_srDao.getRecords(shutdown, minLatency, limit);
    }

    public long deleteById(long[] ids) {
        return m_srDao.deleteById(ids);
    }

    public long deleteByToken(String token)
    {
        return m_srDao.deleteRecordsByToken(token);
    }

    public StorageRecord[] getAndReserve(int minLatency, long limit, long now, long until)
    {
        return m_srDao.getAndReserve(minLatency, limit, now, until);
    }

    public void releaseUnconsumed(StorageRecord[] selected, int index)
    {
        m_srDao.releaseUnconsumed(selected, index);
    }

    public void deleteAllSettings() {
        m_settingDao.deleteAllSettings();
    }

    public void deleteSetting(String name) {
        m_settingDao.deleteSetting(name);
    }

    public static native void connectContext(Context context);
}

