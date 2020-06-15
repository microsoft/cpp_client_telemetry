package com.microsoft.applications.events;

import androidx.room.Database;
import androidx.room.RoomDatabase;

@Database(version = 1, entities = { StorageRecord.class, StorageSetting.class })
public abstract class OfflineRoomDatabase extends RoomDatabase {
    abstract public StorageRecordDao getStorageRecordDao();
    abstract public StorageSettingDao getStorageSettingDao();
}
