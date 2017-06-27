#pragma once

#include <Version.hpp>
#include <stddef.h>
#include <cstdint>
#include <stdio.h>
#include <string.h>


#define MAX_EVENT_PRIORITY      4

namespace ARIASDK_NS_BEGIN {

	enum DATARV_ERROR
	{
		DATARV_ERROR_OK = 0,
		DATARV_ERROR_FAIL,
		DATARV_ERROR_NOT_IMPLEMENTED,
		DATARV_ERROR_INVALID_ARG,
		DATARV_ERROR_INVALID_CONFIG,
		DATARV_ERROR_INVALID_DEPENDENCIES,
		DATARV_ERROR_INVALID_HTTPSTACK,
		DATARV_ERROR_INVALID_STATUS,
		DATARV_ERROR_INVALID_EVENT,
		DATARV_ERROR_INVALID_EVENT_VERSION,
		DATARV_ERROR_DISABLED,
		DATARV_ERROR_OUTOFMEMORY,
		DATARV_ERROR_UNEXPECTED,
		DATARV_ERROR_EVENT_BANNED,
		DATARV_ERROR_EVENT_SIZE_LIMIT_EXCEEDED,
		DATARV_ERROR_CREATE_TIMER_FAILED,
		DATARV_ERROR_INIT_OFFLINESTORAGE_FAILED,
		DATARV_ERROR_START_OFFLINESTORAGE_FAILED,
		DATARV_ERROR_SMALL_OFFLINESTORAGE_SIZE,
		DATARV_ERROR_OFFLINESTORAGE_DISABLED,
		DATARV_ERROR_FILE_DATA_TOOLARGE,
		DATARV_ERROR_FILE_NOTOPEN,
		DATARV_ERROR_FILE_DOESNOTEXIST,
		DATARV_ERROR_FILE_EMPTY,
		DATARV_ERROR_FILE_NOMOREITEMS,
		DATARV_ERROR_DATA_SERIALIZATION_FAILED
		//should update jniwrapper/DataRVErrorCode.java if there is ErrorCode changed
	};

/** \file IStorage.hpp

IStorage is an abstract of physical storage (e.g. file, database). It will
support save/load item to/from the physical storage. Due to this interface
is an abstract of physical device. So, operations on this interface should
be treated as taking the same operations on the physical device. The user
should consider the performance impact and do some cache or batch work if
necessary.

\todo It will also make sure the physical storage is in correct state. If
there are some corruption cases, it will take the charge of handling them.
*/

/** \brief An enum to define error codes of operating the storage.
*/
enum RES_VALUE
{
    RES_FILE_ADJUST_SIZE_FAIL = -6,
    RES_DATA_LARGE = -5,
    RES_FILE_NOT_OPEN = -4,
    RES_FILE_EXIST = -3,
    RES_OPEN_FAIL = -2,
    RES_ERROR = -1,
    RES_SUCC = 0,
    RES_OTHER_ERROR
};

/** \brief A structure to hold the file stats, which is part of telemetry stats.
*/
struct FileStats
{
    /// total overwritten size in bytes
    size_t lastOverWrittenSizeInBytes;
    size_t fileContentsSize;
    size_t eventCounts[MAX_EVENT_PRIORITY];
};

/** \brief An enum to define different storage types.
*/
enum StorageType
{
    STORAGE_TYPE_FIFO = 0
};

const int STORAGE_KEY_ID_AND_PROPERTY_SIZE = 128;
////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
///  Storage item key. This key is used for sorting and searching.
///  User of IStorage can pass an object of this type to SaveItem function to provide Id, Priority
///  and Time for the item being stored. Items are retrieved in FIFO basis hence Time field plays
///  a key role in determining the order of logical blocks. If items have the same time value,
///  Id and Priority will be used for determining the order.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////
class StorageItemKey
{
public:
    // DONOT CHANGE THIS ORDER - Client Agnostic Sorting Key
    char Id[STORAGE_KEY_ID_AND_PROPERTY_SIZE];
    std::uint32_t Priority;
    std::int64_t Time;
    std::uint32_t Version;

    // Use these custom properties for more granularity
    std::uint32_t CustomProperty1;
    std::uint32_t CustomProperty2;
    char CustomProperty3[STORAGE_KEY_ID_AND_PROPERTY_SIZE];

    StorageItemKey(char *Id, std::uint32_t Priority, std::int64_t Time) {
    	clear();
    	memcpy(&(this->Id[0]), (void *)Id, STORAGE_KEY_ID_AND_PROPERTY_SIZE -1);
    	this->Priority = Priority;
    	this->Time = Time;
    }

    StorageItemKey() {
    	clear();
    }

    void CopyFrom(const StorageItemKey& rhs)
    {
        memcpy(Id, rhs.Id, sizeof(Id) / sizeof(Id[0]));

        Priority = rhs.Priority;
        Time = rhs.Time;
        Version = rhs.Version;

        CustomProperty1 = rhs.CustomProperty1;
        CustomProperty2 = rhs.CustomProperty2;
        memcpy(CustomProperty3, rhs.CustomProperty3, sizeof(CustomProperty3) / sizeof(CustomProperty3[0]));
    }

    StorageItemKey& operator=(const StorageItemKey& rhs)
    {
        CopyFrom(rhs);
        return (*this);
    }

    bool operator==(const StorageItemKey& rhs) const
    {
        const StorageItemKey* ptr = dynamic_cast<const StorageItemKey*>(&rhs);
        return (this==ptr);
    }

    bool operator==(const StorageItemKey *ptr) {
        if (ptr == this) {
        	return true;
        }
        if (ptr != nullptr) {
        	return (0==strncmp((const char *)(&ptr->Id[0]), (const char *)(&Id[0]), sizeof(Id)));
        }
        return false;
    }

    void clear() {
        memset(&Id, 0, sizeof(Id));
        Priority = 0;
        Time = 0;
        Version = 0;
        CustomProperty1 = 0;
        CustomProperty2 = 0;
        memset(&CustomProperty3, 0, sizeof(CustomProperty3));
    }

};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
///  Information about the item returned from FindFirst and FindNext functions. 
///  Key helps retrieve Id, Priority and Time properties of the item whereas
///  ItemSize provides information about the block size
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////
struct FindItemInfo
{
    StorageItemKey Key;

    // Block size in bytes
    size_t ItemSize;
};

/** \brief This class is an abstraction of physical storage.
*/
class IStorage
{
public:
    /** \brief A destructor
    */
    virtual ~IStorage() { }

    /** \brief Open storage.

        This function will be used to open the storage in exclusive mode. That
        means if the storage is opened already, other open operation will return
        error.

        \param path
        The storage path (e.g. file path for file storage, or database
        connection string for database storage).

        \param size
        The storage size limitation. If this value is bigger than 0, it will
        create the storage if the storage is not existed. If the storage is
        existed, there are three cases:
        1. The value equals to the old value:
        Do nothing.
        2. The value is bigger than the old value:
        Use this value as the new size limitation.
        3. The value is less than the old value:
        User can only load the items until the file size is less
        than this value. And shrink the file size to this value
        in Close() if the current file size is bigger than the
        value.
        \param block_size_byte
        \return 0 if no error.
    */
    virtual int Open(const char* path, size_t size = 0, size_t block_size_byte = DEFAULT_BLOCK_SIZE) = 0;

    /** \brief Close storage.

    This function will be used to close the storage..
    */
    virtual void Close() = 0;

    /** \brief Get the size of the first block.

        \return the first blocks's data size; 0 if there is no data in disk
    */
    virtual size_t GetBufferSize() = 0;

    /** \brief Save item to storage.

        This function will save the item to storage. If the storage is full,
        an error will be return.

        \param item
        The item data.

        \param size
        The item size.

        \return 0   No error.
    */
    virtual int SaveItem(const char* item, size_t size, const StorageItemKey* pFileItemKey /* Optional */) = 0;

    /** \brief Load and remove item (logical block) from storage.

        This function will load the item form the storage and the item will be
        removed from the storage.

        \param item
        The buffer for the item. If NULL, this function will return the item's
        size.

        \param size
        The item size.

        \return
        Return the actual item size. If the return value is less than or
        equal to the size, that means, the item has been loaded. If the
        return value is larger than the size, that means, the buffer is too
        small, so the item hasn't been loaded. You should create a bigger
        buffer to receive the item.

        If the return value is 0, means no item to read, storage is empty.

        If the return is less than 0, means error occurs.
    */
    virtual int PopNextItem(char** item, size_t& size, StorageItemKey* pFileItemKey /* Optional */) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///  Searches for the first item.
    /// </summary>
    /// <param name="pFindInfo">
    ///  [in,out] If non-null, information describing the item. This item can be used in other Find
    ///  category functions such as FindNextItem, ReadItem, RemoveItemAndMoveNext. 
    ///  Note that simultaneous, multiple calls to FindFirstItem is not supported.
    /// </param>
    /// <returns>
    ///  DATARV_ERROR_FILE_NOMOREITEMS - If the storage is empty or the "find" pointer has reached the
    ///  end of the list.  
    /// </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual DATARV_ERROR FindFirstItem(FindItemInfo* pFindInfo) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///  Searches for the next item.
    /// </summary>
    /// <param name="pFindInfo">
    ///  [in] FindItemInfo object returned in FindFirstItem.
    /// </param>
    /// <returns>
    ///  DATARV_ERROR_FILE_NOMOREITEMS - If the storage is empty or the "find" pointer has reached the
    ///  end of the list.
    /// </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual DATARV_ERROR FindNextItem(FindItemInfo* pFindInfo) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///  Reads the item pointed by FindItemInfo. 
    /// </summary>
    /// <param name="pFindInfo">
    ///  [in] FindItemInfo object returned by FindFirstItem / FindNextItem / RemoveItemAndMoveNext.
    /// </param>
    /// <returns>
    ///  DATARV_ERROR_FILE_NOMOREITEMS - If the storage is empty or the "find" pointer has reached the
    ///  end of the list. 
    /// </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual DATARV_ERROR ReadItem(
        const FindItemInfo& findInfo, 
        size_t readSize, 
        char** ppBuffer, 
        size_t* pBufferSize) = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///  Reads the item pointed by FindItemInfo and removes the same from the storage. After removing
    ///  it advances the "find" pointer to the next item in the list.
    /// </summary>
    /// <param name="pFindInfo">
    ///  [in] FindItemInfo object returned by FindFirstItem / FindNextItem / RemoveItemAndMoveNext.
    /// </param>
    /// <param name="pHasMoreItems">
    ///  [in,out] Indicates whether or not the list has more items to be visited (Find)
    /// </param>
    /// <param name="ppBuffer">
    ///  [in,out] Output buffer.
    /// </param>
    /// <param name="pBufferSize">
    ///  [in,out] Buffer Size
    /// </param>
    /// <returns>
    ///  DATARV_ERROR_FILE_NOMOREITEMS - If the storage is empty or the "find" pointer has reached the
    ///  end of the list.
    /// </returns>
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual DATARV_ERROR RemoveItemAndMoveNext(
        FindItemInfo* pFindInfo,
        bool* pHasMoreItems,
        char** ppBuffer, 
        size_t* pBufferSize) = 0;

    /** \brief flush data to file system.
    */
    virtual void Flush() = 0;

    /** \brief Update file stats.
        Currently only save the last overwritten size to fileStats and clear it at the same time.
        \param fileStats File stats will be saved in this object.
    */
    virtual void GetFileStats(FileStats* pFileStats) = 0;

    /** \brief Default block size.
     */
    static const size_t DEFAULT_BLOCK_SIZE = 32 * 1024;

    /** \brief Storage format version.
     */
    static const unsigned int FORMAT_VERSION = 1;

    /** \brief Current storage format.
     */
    static const StorageType STORAGE_FORMAT = STORAGE_TYPE_FIFO;
};

} ARIASDK_NS_END
