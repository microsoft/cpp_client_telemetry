package com.microsoft.applications.events;

import androidx.annotation.NonNull;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Entity
public class StorageSetting {
    @PrimaryKey
    @NonNull
    String name;
    @NonNull
    String value;

    StorageSetting(@NonNull String name, @NonNull String value)
    {
        this.name = name;
        this.value = value;
    }
}
