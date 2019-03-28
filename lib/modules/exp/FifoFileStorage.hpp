#include "mat/config.h"
#ifdef HAVE_MAT_FIFOSTORAGE
//
// TODO: [MG] - as discussed, FIFO is going to be killed and replaced by SQLite database-stored properties
//
#pragma once

#include "pal/PAL.hpp"

#include "modules/exp/IDataStorage.hpp"

#include <queue>
#include <vector>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <list>
#include <map>

#ifndef _MSC_VER
#include "linux/sal.h"
#endif

namespace MAT_NS_BEGIN {

    /** \brief An enum to define the flag of the block.
    */
    enum BlockFlag
    {
        END_OF_BLOCK = -3,
        BEG_OF_BLOCK = -2,
        MID_OF_BLOCK = -1   // WHY IS THIS REQUIRED?
    };
    
    /** \brief A structure to define block information
    */
    struct PhysicalBlockInfo
    {
        /** \brief checksum
        */
		size_t  CheckSum;

        /** \brief mark the used size of the block, if 0 means not used
        */
		size_t  SizeInBytes;

        /** \brief next block id.

        if nextBlockId=-1, is the end;
        else list all the blocks is the whole data
        */
		int   NextBlockId;

        /** \brief previous block id.
        */
		int   PreviousBlockId;

        /** \brief block offset in the file
        */
        std::uint64_t  FileOffset;

        /** \brief the utc timestamp of the block used.
        */
        StorageItemKey Key;

		bool IsBlockInUse() const { return SizeInBytes != 0; }
    };

    /** \brief block list
    */
    struct LogicalBlockInfo
    {
        /** \brief total size of buffer
        */
		size_t  SizeInBytes;

        /** \brief start block id
        */
		int  FirstPhysicalBlockIndex;

        /** \brief block numbers, composed by a number of block
        */
		size_t  PhysicalBlockCount;

        StorageItemKey Key;

        //int  end;       ///< finish block id
    };

    struct BlockKey
    {
		size_t BlockId;

        StorageItemKey ItemKey;
    };

    class BlockKeyComparer
    {
    public:
        bool operator() (const BlockKey& b1, const BlockKey& b2) const
        {
            if (b1.ItemKey.Time == b2.ItemKey.Time)
            {
				size_t idResult = strcmp(b1.ItemKey.Id, b2.ItemKey.Id);

                if (idResult == 0)
                {
                    if (b1.ItemKey.Priority == b2.ItemKey.Priority)
                    {
                        return b1.BlockId < b2.BlockId;
                    }
                    else
                    {
                        return b1.ItemKey.Priority < b2.ItemKey.Priority;
                    }
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return b1.ItemKey.Time < b2.ItemKey.Time;
            }
        }
    };

    // [MG] FIXME: below structure is different between 32-bit and 64-bit!
    /** \brief A structure to define the file header.
    */
    struct  FileHeader
    {
        /** \brief magicNumber: (0x1989F64(Magic Number) << 4) & version
        */
        std::uint64_t  magicNumber;

        /** \brief file size
        */
        size_t  fileSize;

        /** \brief block numbers.
        */
		size_t  physicalBlockCount;

        /** \brief block size.
        */
		size_t  blockSize;
    };

    /** \brief An enum to define status of file
    */
    enum FileStatus
    {
        FILE_OPEN,
        FILE_READY, // file is ready to write
        FILE_CLOSE,
        FILE_WRITING,
        FILE_READING
    };
    
    /** \brief An implementation class of FIFO storage.
    */
    class FIFOFileStorage : public IDataStorage
    {
    public:
        /** \brief A constructor,

        block is the store unit in store file, default is 32KB
        */
        FIFOFileStorage();

        /** \brief A destructor.
        */
        virtual ~FIFOFileStorage();

        /** \brief Open storage.

        Open the file in exclusive mode. If the file is existed already, check
        the file's magic number and version. Return error if the magic number is
        not match or the version is not supported.

        \param path storage file path
        \param size file size
        \param block_size_byte block size
        \return 0 if no error
        */
        virtual int Open(const char* path, size_t size,  size_t block_size_byte = DEFAULT_BLOCK_SIZE);

        /** \brief Close storage.

        This function will be used to close the storage.
        */
        virtual void Close();

        /** \brief Save item to storage.

        In this function, we will move the file pointer to the end of the file,
        save the buffer first, and then save the buffer's size.

        \param buffer
        The item data.

        \param size
        The item size.

        \return 0   No error.
        */
        virtual int SaveItem(const char* buffer, size_t size, const StorageItemKey* pStorageItemKey /* Optional */);

        /** \brief Load and remove item from storage.

        In this function, we will move the file pointer to the end of the file.
        Then we will read the previous four bytes which should be the size of
        the item. Then we will read the previous X bytes to buffer. Then shrink
        the file to the current size.

        \param buffer the buffer for the item
        \param size the item size
        */
        virtual int PopNextItem(char** buffer, size_t& size, StorageItemKey* pStorageItemKey /* Optional */);

        virtual DATARV_ERROR FindFirstItem(FindItemInfo* pFindItemInfo);
        virtual DATARV_ERROR FindNextItem(FindItemInfo* pFindItemInfo);

        virtual DATARV_ERROR ReadItem(const FindItemInfo& findItemInfo, size_t readSize, char** ppBuffer, size_t* pBufferSize);

        DATARV_ERROR RemoveItemAndMoveNext(FindItemInfo* pFindItemInfo, bool* pHasMoreItems, char** ppBuffer, size_t* pBufferSize);

        /** \brief flush data to filesystem.
        */
        virtual void Flush();

        /** \brief Free *buffer and set it to NULL.
        */
        virtual void Free(char** buffer);

        /** \brief Delete file with give filePath.

        \param filePath
        \return 0 if no error
        */
        virtual size_t  DeleteFileLocal(const char* filePath);

        /** \brief Get file size.

        \return the size of storage file in use
        */
        virtual std::uint64_t GetFileSize();

        /** \brief Update file stats. Currently only save the last overwritten size to fileStats
        and clear it at the same time.

        \param fileStats File stats will be saved in this object.
        */
        virtual void GetFileStats(FileStats* pFileStats);

        /** \brief Get the size of the first block.

        \return the first blocks's data size; 0 if there is no data in disk
        */
        virtual size_t  GetBufferSize();

        /** \brief Judge if there are items in storage.

        \return bool to indicate whether or not the storage is empty. True indicates empty storage
        */
        virtual bool IsStorageEmpty();

        /** \brief Default file size.
        */
        static const std::uint64_t DEFAULT_FILE_SIZE = 2 * 1024 * 1024;

        /** brief Max file size allowed.
        */
        static const std::uint64_t MAX_FILE_SIZE = 100 * 1024 * 1024;

        /** \brief Magic number to compose magic_num in file header.
        */
        static const std::uint64_t MAGIC_NUM = 0x656c69666f7473ULL; ///< magic num is "stofile", 56bit

        /** \brief checksum of each block
        */
        static const size_t  CHECKSUM_NUM = 0xFC1985AB;

    private:
        std::FILE*  m_FileHandle;
        std::string m_FilePath;
        FileStatus m_FileStatus;
		size_t m_blockSizeInBytes;

        std::vector<PhysicalBlockInfo>  m_PhysicalBlocks;
        std::map<BlockKey, LogicalBlockInfo, BlockKeyComparer>  m_LogicalBlocks;
        typedef std::map<BlockKey, LogicalBlockInfo, BlockKeyComparer>::iterator LogicalBlockIterator;

        FileHeader m_FileHeader;

        /// records the file size parameter in the call of Open(), please refer to m_FileHeader.fileSize for actual file size.
        size_t m_FileOpenSize;

        size_t m_lastOverwrittenSizeInBytes;

        /**
        * check the file is the target file
        */
		int  VerifyFileChecksum();

		int  Write(_In_reads_bytes_(size) const char* buffer, size_t size);
		int  Read(_Out_writes_bytes_ (size) char *buffer, size_t size);

		int  WriteFileHeader();
		int  WriteFileInfo();
		int  ReadFileHeader();
		int  ReadFileInfo();
        void CheckBlockInfo();
        void BuildIndexInfo();
        static size_t CalculateFileSize(size_t fileSize, size_t alignSize);
		int  GenerateFile();
		int  UpdateBlockToDisk(LogicalBlockInfo& idx, const char *buffer, size_t buffSize);
		size_t  FillFileHeader(size_t size, size_t block_size);
        size_t _GetFileContentsSize();
		int RecreateFile(bool closeFile = true);

        LogicalBlockInfo& GetOldestLogicalBlock()
        {
            return m_LogicalBlocks.begin()->second;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        ///  Searches for the next item.
        /// </summary>
        /// <param name="firstItem">
        ///  true if this function is called from FindFirst. If true, this function will reset the iterator
        ///  to point it to the first item of the logical block list.
        /// </param>
        /// <param name="pFindItemInfo">
        ///  [in,out] If non-null, information describing the item found.
        /// </param>
        /// <returns>
        ///  DATARV_ERROR.
        ///  DATARV_ERROR_FILE_NOMOREITEMS if the list is empty or the search has reached the last element 
        ///  of the list
        /// </returns>
        ////////////////////////////////////////////////////////////////////////////////////////////////////
        DATARV_ERROR _FindNextItem(bool firstItem, FindItemInfo* pFindItemInfo);

        ////////////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        ///  Reads an item based on the given logical block information
        /// </summary>
        /// <param name="logicalBlockInfo">
        ///  Information describing the logical block.
        /// </param>
        /// <param name="readSize">
        ///  Number of bytes to be read from the logical block.
        /// </param>
        /// <param name="pStorageItemKey">
        ///  [in,out] If non-null, the file item key that describes the block that's read
        /// </param>
        /// <param name="ppBuffer">
        ///  [in,out] Output buffer. If not null, the caller must release the memory using free or
        ///  FIFOFileStorage::Free
        /// </param>
        /// <param name="pBufferSize">
        ///  [in,out] Number of bytes read.
        /// </param>
        /// <returns>
        ///  The item.
        /// </returns>
        ////////////////////////////////////////////////////////////////////////////////////////////////////

        DATARV_ERROR _ReadItem(
            const LogicalBlockInfo& logicalBlockInfo,
            size_t readSize, 
            StorageItemKey* pStorageItemKey,
            char** ppBuffer, 
            size_t* pBufferSize);

        /*
        * traversal the array and return the free block
        * return -1 no free block, else return the free block position
        */
		int  FindFreePhysicalBlockIndex();
        void ReleaseBlocks(LogicalBlockInfo& idx);
        void Reset();
        /*
        * check the file status is ready. 
        * If it's not ready, adjust the file, then after calling it's ready.
		*
        */
		int  PrepareFileForWrite();
		int  AdjustFileSize(size_t newFileSize);
        static size_t  AlignToPower(size_t size, size_t alignSize);

        // Replica created for find operation
        // std::list<LogicalBlockInfo> m_LogicalBlocksReplica;

        std::map<BlockKey, LogicalBlockInfo, BlockKeyComparer> m_LogicalBlocksReplica;
        LogicalBlockIterator m_findIter;

    };

} MAT_NS_END
#endif
