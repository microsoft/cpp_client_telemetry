//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.room.Dao;
import androidx.room.Query;

@Dao
public interface StorageSettingDao {
    @Query("SELECT * FROM StorageSetting WHERE name = :n")
    StorageSetting[] getValues(String n);
    
    @Query("INSERT OR REPLACE INTO StorageSetting (name, value) VALUES (:n, :v)")
    long setValue(String n, String v);

    @Query("SELECT sum(length(name) + length(value)) FROM StorageSetting")
    long totalSize();

    @Query("DELETE FROM StorageSetting")
    int deleteAllSettings();

    @Query("DELETE FROM StorageSetting WHERE name = :name")
    int deleteSetting(String name);

    @Query("SELECT count(*) FROM StorageSetting")
    long totalSettingCount();
}

