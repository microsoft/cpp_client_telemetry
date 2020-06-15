package com.microsoft.applications.events;

import android.content.Context;
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
        Log.i("MAE", String.format("Opened %s: %d records, %d settings", name, m_srDao.totalRecordCount(), m_settingDao.totalSettingCount()));
    }

    public void storeRecords(StorageRecord... records) {
        m_srDao.insertRecords(records);
    }

    public void closeConnection()
    {
        if (m_db.isOpen()) {
            m_db.close();
        }
    }

    public void storeFromBuffers(int count, int[] indices, byte[] bytes, int[] small, long[] big)
    {
        int byteIndex = 0;
        StorageRecord[] records = new StorageRecord[count];
        for (int i = 0; i < count; ++i) {
            String id = new String(bytes, byteIndex, indices[3 * i]);
            byteIndex += indices[3 * i];
            String tenantToken = new String(bytes, byteIndex, indices[3 * i + 1]);
            byteIndex += indices[3 * i + 1];
            byte[] blob = new byte[indices[3 * i + 2]];
            for (int j = 0; j < indices[3 * i + 2]; ++j) {
                blob[j] = bytes[j + byteIndex];
            }
            byteIndex += indices[3 * i + 2];
            records[i] = new StorageRecord(
                    id,
                    tenantToken,
                    small[3 * i],
                    small[3 * i + 1],
                    big[2 * i],
                    small[3 * i + 2],
                    big[2 * i + 1],
                    blob);
        }
        storeRecords(records);
    }

    public long getRecordCount(int latency) {
        if (latency == StorageRecord.EventLatency_Unspecified) {
            return m_srDao.totalRecordCount();
        }
        return m_srDao.recordCount(latency);
    }

    public long trim(double fraction) {
        long records = m_srDao.totalRecordCount();
        long to_drop = (long) Math.ceil(fraction * records);
        if (to_drop == 0) {
            return 0;
        }
        return m_srDao.trim(to_drop);
    }

    public ByTenant[] releaseRecords(String[] ids, boolean incrementRetry, long maximumRetries)
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
        return m_srDao.totalSize() + m_settingDao.totalSize();
    }

    public StorageRecord[] getRecords(boolean shutdown, int minLatency, long limit)
    {
        return m_srDao.getRecords(shutdown, minLatency, limit);
    }

    public long deleteById(String[] ids) {
        return m_srDao.deleteById(ids);
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
