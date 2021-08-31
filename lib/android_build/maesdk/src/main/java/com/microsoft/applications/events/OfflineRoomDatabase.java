//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;
import androidx.room.Database;
import androidx.room.RoomDatabase;

@Keep
@Database(version = 3, entities = { StorageRecord.class, StorageSetting.class })
public abstract class OfflineRoomDatabase extends RoomDatabase {
    abstract public StorageRecordDao getStorageRecordDao();
    abstract public StorageSettingDao getStorageSettingDao();
}

