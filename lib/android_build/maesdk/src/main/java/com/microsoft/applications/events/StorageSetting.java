package com.microsoft.applications.events;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Keep
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
