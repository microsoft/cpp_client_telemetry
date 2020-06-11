package com.microsoft.applications.events;

import androidx.annotation.NonNull;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Entity
public class StorageRecord {
    final public static int EventLatency_Unspecified = -1;
    final public static int EventLatency_Normal = 1;
    final public static int EventLatency_RealTime = 3;
    final public static int EventPersistence_Normal = 1;


    public StorageRecord(
        @NonNull
        String id,
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

    @PrimaryKey
    @NonNull
    public String id;
    public String tenantToken;
    public int latency = EventLatency_Unspecified;
    public int persistence = EventPersistence_Normal;
    public long timestamp = 0;
    public int retryCount = 0;
    public long reservedUntil = 0;
    public byte[] blob;
}
