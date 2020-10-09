//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.room.ColumnInfo;
import androidx.room.Entity;
import androidx.room.Index;
import androidx.room.PrimaryKey;

@Keep
@Entity(indices = {@Index(value = {"id"}, unique = true)})
public class StorageRecord {
    final public static int EventLatency_Unspecified = -1;
    final public static int EventLatency_Normal = 1;
    final public static int EventLatency_RealTime = 3;
    final public static int EventPersistence_Normal = 1;


    public StorageRecord(
        long id,
        String tenantToken,
        int latency,
        int persistence,
        long timestamp,
        int retryCount,
        long reservedUntil,
        byte[] blob)
    {
        this.id = id;
        this.tenantToken = tenantToken;
        this.latency = latency;
        this.persistence = persistence;
        this.timestamp = timestamp;
        this.retryCount = retryCount;
        this.reservedUntil = reservedUntil;
        this.blob = blob;
    }

    @PrimaryKey(autoGenerate = true)
    @ColumnInfo(typeAffinity = ColumnInfo.INTEGER)
    public long id = 0;
    public String tenantToken;
    @ColumnInfo(index = true)
    public int latency = EventLatency_Unspecified;
    public int persistence = EventPersistence_Normal;
    public long timestamp = 0;
    public int retryCount = 0;
    public long reservedUntil = 0;
    public byte[] blob;
}

