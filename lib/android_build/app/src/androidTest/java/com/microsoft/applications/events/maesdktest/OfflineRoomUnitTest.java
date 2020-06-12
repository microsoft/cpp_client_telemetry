package com.microsoft.applications.events.maesdktest;

import android.content.Context;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import com.microsoft.applications.events.ByTenant;
import com.microsoft.applications.events.OfflineRoom;
import com.microsoft.applications.events.StorageRecord;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

@RunWith(AndroidJUnit4.class)
public class OfflineRoomUnitTest {
    @Test
    public void storeOneRecord() {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        OfflineRoom room = new OfflineRoom(appContext, "OfflineRoomUnit");

        room.deleteAllRecords();
        StorageRecord record = new StorageRecord(
            "Fred", "George",
            StorageRecord.EventLatency_Normal,
            StorageRecord.EventPersistence_Normal,
            32,
            1,
            0,
            new byte[] {1, 2, 3});
        room.storeRecords(record);
        assertEquals(1, room.getRecordCount(StorageRecord.EventLatency_Unspecified));
        assertEquals(1, room.getRecordCount(StorageRecord.EventLatency_Normal));
        assertEquals(
            record.id.length() + record.tenantToken.length() + record.blob.length + 32,
            room.totalSize());
        assertEquals(1, room.deleteAllRecords());
    }

    protected void makeTenRecords(
        OfflineRoom room,
        int latency,
        int persistence
    ) {
        room.deleteAllRecords();

        StorageRecord record = new StorageRecord(
            "",
            String.format("George-%d-%d", latency, persistence),
            latency,
            persistence,
            0,
            0,
            0,
            new byte[] {1, 2, 3}
        );
        for (int i = 0; i < 10; ++i) {
            record.id = String.format("Fred-%d-%d-%d", latency, persistence, i);
            record.tenantToken = String.format("George-%d-%d", latency, i);
            room.storeRecords(record);
        }

    }

    @Test
    public void TrimRecords() {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        OfflineRoom room = new OfflineRoom(appContext, "OfflineRoomUnit");

        makeTenRecords(room, StorageRecord.EventLatency_Normal, StorageRecord.EventPersistence_Normal);
        assertEquals(10, room.getRecordCount(StorageRecord.EventLatency_Normal));
        assertEquals(5, room.trim(0.5));
        assertEquals(5, room.getRecordCount(StorageRecord.EventLatency_Normal));
    }

    @Test
    public void GetAndReserve() {
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        OfflineRoom room = new OfflineRoom(appContext, "OfflineRoomUnit");

        makeTenRecords(room, StorageRecord.EventLatency_Normal, StorageRecord.EventPersistence_Normal);
        makeTenRecords(room, StorageRecord.EventLatency_RealTime, StorageRecord.EventPersistence_Normal);
        assertEquals(10, room.getRecordCount(StorageRecord.EventLatency_Unspecified));
        StorageRecord[] records = room.getAndReserve(StorageRecord.EventLatency_Normal, 3, 2, 5);
        assertEquals(3, records.length);
        for (StorageRecord record : records) {
            assertEquals(StorageRecord.EventLatency_RealTime, record.latency);
            assertEquals(0, record.retryCount);
            assertEquals(0, record.reservedUntil);
        }
        records = room.getAndReserve(StorageRecord.EventLatency_Normal, 1000, 2, 5);
        assertEquals(7, records.length);
        for (StorageRecord record : records) {
            assertEquals(StorageRecord.EventLatency_RealTime, record.latency);
            assertEquals(0, record.retryCount);
            assertEquals(0, record.reservedUntil);
        }
        records = room.getAndReserve(StorageRecord.EventLatency_Normal, 1000, 2, 5);
        assertEquals(10, records.length);
        for (StorageRecord record : records) {
            assertEquals(StorageRecord.EventLatency_Normal, record.latency);
            assertEquals(0, record.retryCount);
            assertEquals(0, record.reservedUntil);
        }
    }

    @Test
    public void ReleaseUnconsumed() {
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        OfflineRoom room = new OfflineRoom(appContext, "OfflineRoomUnit");

        makeTenRecords(room, StorageRecord.EventLatency_Normal, StorageRecord.EventPersistence_Normal);
        StorageRecord[] records = room.getAndReserve(StorageRecord.EventLatency_Normal, 1000, 2,5);
        assertEquals(10, records.length);
        StorageRecord[] nothing = room.getAndReserve(StorageRecord.EventLatency_Normal, 1000, 2,5);
        assertEquals(0, nothing.length);
        room.releaseUnconsumed(records, 5);
        StorageRecord[] released = room.getAndReserve(StorageRecord.EventLatency_Normal, 1000, 2,5);
        assertEquals(5, released.length);
    }

    public String[] collectIds(StorageRecord[] records) {
        if (records == null || records.length == 0) {
            return new String[0];
        }
        String[] ids = new String[records.length];
        for (int i = 0; i < records.length; ++i) {
            ids[i] = records[i].id;
        }
        return ids;
    }

    @Test
    public void RetireRetries() {
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        OfflineRoom room = new OfflineRoom(appContext, "OfflineRoomUnit");

        makeTenRecords(room, StorageRecord.EventLatency_Normal, StorageRecord.EventPersistence_Normal);
        StorageRecord[] records = room.getAndReserve(StorageRecord.EventLatency_Normal, 5, 2,5);
        assertEquals(5, records.length);
        String[] ids = collectIds(records);
        ByTenant[] timedOut = room.releaseRecords(ids, true, 1);
        assertNotNull(timedOut);
        assertEquals(0, timedOut.length);
        records = room.getAndReserve(StorageRecord.EventLatency_Normal, 1000, 2,5);
        assertNotNull(records);
        assertEquals(10, records.length);
        ids = collectIds(records);
        timedOut = room.releaseRecords(ids, true, 1);
        assertNotNull(timedOut);
        assertEquals(5, timedOut.length);
        for (int i = 0; i < 5; ++i) {
            assertNotNull(timedOut[i]);
            assertNotNull(timedOut[i].tenantToken);
            assertEquals(1, timedOut[i].count);
        }
    }
}
