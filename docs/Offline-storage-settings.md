# Offline storage configurations

There are several configurations that can alter the offline storage handler behaviour.

## Configurations

| Configuration | Type | Default value | Description |
| ------------- | ---- | ------------- | -------------|
| CFG_INT_CACHE_FILE_SIZE | int | 3145728 | Sets size limit for the cache file.
| CFG_INT_STORAGE_FULL_PCT | int | 75 | Sets the notification threshold (percentage) for storage full notifications. If the cache file size excceds CFG_INT_STORAGE_FULL_PCT percent, an EVT_STORAGE_FULL debug event will be fired.
| CFG_INT_STORAGE_FULL_CHECK_TIME | int | 5000 | Sets the minimum time (ms) between storage full notifications.
| CFG_BOOL_ENABLE_DB_DROP_IF_FULL | bool | false | When set to true, trim events if cache size reaches CFG_INT_CACHE_FILE_SIZE
| CFG_STR_CACHE_FILE_PATH | string | %TEMP% | Sets the path for the cache file

## Deprecated configurations

| Configuration |
| ------------- |
| CFG_BOOL_ENABLE_DB_COMPRESS |
| CFG_BOOL_ENABLE_WAL_JOURNAL |
| CFG_INT_RAM_QUEUE_BUFFERS |
| CFG_STR_PRAGMA_JOURNAL_MODE |
| CFG_STR_PRAGMA_SYNCHRONOUS |
