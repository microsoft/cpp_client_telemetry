package com.microsoft.applications.events;

import android.content.Context;
import android.database.Cursor;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.room.Room;
import androidx.room.RoomDatabase;

import java.util.Map;
import java.util.TreeMap;

public class OfflineRoom {
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
        Cursor c = null;
        try {
            c = m_db.query("PRAGMA page_size", null);
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
        catch (Exception e) {
            Log.e("MAE", "Exception while querying page size", e);
        }
        finally {
            if (c != null) {
                c.close();
                c = null;
            }
        }
        long pageCount = -1;
        try {
            c = m_db.query("PRAGMA page_count", null);
            if (c.getCount() == 1 && c.getColumnCount() == 1) {
                c.moveToFirst();
                pageCount = c.getLong(0);
            }
        } catch (Exception e) {
            Log.e("MAE", "Exception while getting page count", e);
        }
        finally {
            if (c != null) {
                c.close();
                c = null;
            }
        }
        Log.i("MAE", String.format("Opened %s: %d records, %d settings, page size %d, %d pages",
                name,
                m_srDao.totalRecordCount(),
                m_settingDao.totalSettingCount(),
                m_pageSize,
                pageCount));
    }

    public long[] storeRecords(StorageRecord... records) {
        return m_srDao.insertRecords(records);
    }

    public void closeConnection()
    {
        if (m_db.isOpen()) {
            m_db.close();
        }
    }

    public void explain()
    {
        Cursor c = null;
        try {
            c = m_db.query("EXPLAIN QUERY PLAN SELECT * FROM StorageRecord WHERE latency >= 1 AND reservedUntil = 0 ORDER BY latency DESC, persistence DESC, timestamp ASC", null);
            int n = c.getCount();
            int m = c.getColumnCount();
            boolean noMove = c.moveToFirst();
            String[] names = c.getColumnNames();
            for (int i = 0; i < names.length; ++i) {
                Log.i("MAE", names[i]);
                try {
                    Log.i("MAE", String.format("Type for column %s (%d): %d", names[i], i, c.getType(i)));
                } catch (Exception e) {
                    Log.i("MAE", "woops", e);
                }
            }
            for (int j = 0; j < n; ++j) {
                if (!c.moveToPosition(j)) {
                    break;
                }
                for (int i = 0; i < m; ++i) {
                    try {
                        Log.i("MAE", String.format("%d %s: %s",
                                j,
                                names[i],
                                c.getString(i)));
                    } catch (Exception e) {
                        Log.i("MAE", "oh sad", e);
                    }
                }
            }
        }
        catch (Exception e) {
            Log.e("MAE", "Exception in explain", e);
        }
        finally {
            if (c != null) {
                c.close();
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

    public long vacuum(long pre) {
        Cursor c = null;
        try {
            c = m_db.query("VACUUM", null);
        }
        catch (Exception e) {
            Log.e("MAE", "Exception in vacuum", e);
        }
        finally {
            if (c != null) {
                c.close();
            }
        }
        long post = totalSize();
        Log.i("MAE", String.format(
                "Vacuum: %d before, %d after",
                pre,
                post));
        return post;
    }

    public long trim(long byteLimit) {
        long currentSize = totalSize();
        if (currentSize <= byteLimit) {
            return 0;
        }
        long postVacuum = currentSize;
        try {
            postVacuum = vacuum(currentSize);
        } catch (Exception e) {
            Log.e("MAE", "Exception in VACUUM", e);
            postVacuum = currentSize;
        }
        if (postVacuum <= byteLimit) {
            return 0;
        }

        long records = m_srDao.totalRecordCount();
        double fraction = 0.25; // fraction of current to be dropped
        if (byteLimit > m_pageSize) {
            double dLimit = byteLimit;
            double dCurrent = postVacuum;
            fraction = Math.max(0.25, 1.0 - (dLimit / dCurrent));
        }
        long to_drop = (long) Math.ceil(fraction * records);
        if (to_drop <= 0) {
            return 0;
        }
        int n = m_srDao.trim(to_drop);
        long postDrop = totalSize();
        long reVacuum = postDrop;
        if (postDrop > byteLimit) {
            try {
                reVacuum = vacuum(postDrop);
            } catch (Exception e) {
                Log.e("MAE", "Exception in reVacuum", e);
            }
        }
        Log.i("MAE", String.format(
                "Trim: dropped %d records, new size %d bytes",
                n,
                reVacuum));
        return n;
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
        Cursor c = null;
        try {
            c = m_db.query("PRAGMA page_count", null);
            if (c.getCount() == 1 && c.getColumnCount() == 1) {
                c.moveToFirst();
                long pages = c.getLong(0);
                result = pages * m_pageSize;
            } else {
                Log.e("MAE", String.format("Unexpected result from PRAGMA page_count, %d rows and %d columns",
                        c.getCount(), c.getColumnCount()));
            }
        }
        catch (Exception e) {
            Log.e("MAE", "Exception in PRAGMA page_count", e);
        }
        finally {
            if (c != null) {
                c.close();
                c = null;
            }
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
