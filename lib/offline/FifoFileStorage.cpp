//
// TODO: [MG] - as discussed, FIFO is going to be killed and replaced by SQLite database-stored properties
//
#include "pal/PAL.hpp"
#include "utils/Utils.hpp"
#include "FifoFileStorage.hpp"

#define AssertAbort(a) assert(a)
namespace ARIASDK_NS_BEGIN {
	
    FIFOFileStorage::FIFOFileStorage()
        : m_FileHandle(nullptr)
        , m_FileStatus(FILE_CLOSE)
        , m_FileOpenSize(0)
        , m_lastOverwrittenSizeInBytes(0)
    {
        memset((void *)&m_FileHeader, 0, sizeof(m_FileHeader));
    }

    FIFOFileStorage::~FIFOFileStorage()
    {
        if (m_FileStatus != FILE_CLOSE)
        {
            Close();
        }
    }

	int FIFOFileStorage::Open(const char *path, size_t size, size_t blockSizeInBytes)
    {
		
        if (path == nullptr)
        {
            LOG_ERROR("open file failed, path passed is nullptr");
            return RES_ERROR;
        }

        if (m_FileStatus != FILE_CLOSE)
        {
			LOG_ERROR("file already has been opened");
            return RES_ERROR;
        }

        if (size > MAX_FILE_SIZE)
        {
            LOG_ERROR("file size is too big, limit file size is = %u", size);
            return RES_ERROR;
        }

        LOG_TRACE("open file [%s], size = %d, block size = %u", path, size, blockSizeInBytes);

		m_FilePath = path;
		m_blockSizeInBytes = blockSizeInBytes;

        if(size == 0)
        {
            LOG_TRACE("open file with size 0, use default size instead");
            size = DEFAULT_FILE_SIZE;
        }
        m_FileOpenSize = size; // for later file size adjustment

#pragma warning( push )
#pragma warning(disable: 4996)
		m_FileHandle = std::fopen(m_FilePath.c_str(), "r+b");
#pragma warning( pop ) 
		if (nullptr == m_FileHandle)
		{// open for read/write failed, lets open for wrire read
			return RecreateFile(false);
		}

        m_FileStatus = FILE_OPEN;

        if (ReadFileHeader() != RES_SUCC)
        {
            LOG_ERROR("offline data file exist, but load file header failed, truncating...");
		    return RecreateFile();
        }

        if (ReadFileInfo() != RES_SUCC)
        {
            LOG_ERROR("offline data file exist, but load file info failed, truncating...");
			return RecreateFile();
        }

        LOG_TRACE("open file [%s] suc", path);
        return RES_SUCC;
    }

	int  FIFOFileStorage::RecreateFile(bool closeFile)
	{
		if (closeFile)
		{
			std::fclose(m_FileHandle);
		}
#pragma warning( push )
#pragma warning(disable: 4996)
		m_FileHandle = std::fopen(m_FilePath.c_str(), "w+b");
#pragma warning( pop ) 

		if (nullptr != m_FileHandle)
		{
			m_FileStatus = FILE_OPEN;

			if (m_blockSizeInBytes <= 0)
			{
				LOG_ERROR("block size is incorrect, size=[%d]", m_blockSizeInBytes);
				return RES_ERROR;
			}

			if (FillFileHeader(0, m_blockSizeInBytes) != RES_SUCC)
			{
				LOG_ERROR("cannot fill file header");
				Close();
				return RES_ERROR;
			}

			if (WriteFileHeader() != RES_SUCC)
			{
				LOG_ERROR("cannot write file header");
				Close();
				return RES_ERROR;
			}
			return RES_SUCC;
		}
		return RES_ERROR;
	}

    void FIFOFileStorage::Close()
    {
        if (m_FileStatus == FILE_CLOSE)
        {
            LOG_WARN("close file: file already closed.");
            return;
        }

        LOG_TRACE("fileClose begin");
		if (nullptr != m_FileHandle)   std::fclose(m_FileHandle);

        LOG_TRACE("fileClose end");
        m_FileStatus = FILE_CLOSE;

        Reset();
        LOG_TRACE("close file success");
    }

	int FIFOFileStorage::Write(_In_reads_bytes_(size) const char* buffer, size_t size)
    {
		if (nullptr == m_FileHandle)
		{
			return RES_FILE_NOT_OPEN;
		}
        if (buffer == nullptr)
        {
            LOG_ERROR("save file : passed parameter buffer is nullptr");
            return RES_ERROR;
        }

        FileStatus oldStatus = m_FileStatus;
        m_FileStatus = FILE_WRITING;

		size_t bufferPosition = 0;
		size_t bytesToWrite = size;

        while (bytesToWrite > 0)
        {
            size_t bytesWritten = std::fwrite(buffer + bufferPosition, 1, bytesToWrite, m_FileHandle);
            // FIXME: review this code
            if (bytesWritten == 0)
            {
                LOG_ERROR("save offline data failed");
                goto SAVE_FAILED;
            }
            bufferPosition += bytesWritten;
            bytesToWrite -= bytesWritten;
        }

        m_FileStatus = oldStatus;
        return RES_SUCC;

SAVE_FAILED:

        m_FileStatus = oldStatus;
        return RES_ERROR;
    }

	int FIFOFileStorage::Read(_Out_writes_bytes_(size) char* buffer, size_t size)
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN;
		}
        if (buffer == nullptr)
        {
            LOG_ERROR("load file : passed parameter buffer is nullptr");
            return RES_ERROR;
        }

        FileStatus oldStatus = m_FileStatus;
        m_FileStatus = FILE_READING;

		size_t bufferPosition = 0;
		size_t bytesToRead = size;

        while (bytesToRead > 0)
        {
			size_t readSize = std::fread(buffer + bufferPosition, 1, bytesToRead, m_FileHandle);
            if (readSize == 0)
            {
                LOG_ERROR("load offline data failed: empty file? %d bytes remaining", bytesToRead);
                // nullptr-terminate the buffer (empty buffer)
                buffer[0]=0;
                goto LOAD_OK;
            } else
            if (readSize < 0)
            {
                LOG_ERROR("load offline data failed: pos=%d, left=%d", bufferPosition, bytesToRead);
                goto LOAD_FAILED;
            }

            bytesToRead -= readSize;
            bufferPosition += readSize;
        }

LOAD_OK:
        m_FileStatus = oldStatus;
        return RES_SUCC;

LOAD_FAILED:

        m_FileStatus = oldStatus;
        return RES_ERROR;
    }

	int FIFOFileStorage::ReadFileHeader()
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN;
		}
		rewind(m_FileHandle);

		size_t headerLength = sizeof(struct FileHeader);
        if (Read((char *)(&m_FileHeader), headerLength) != RES_SUCC)
        {
            LOG_ERROR("wrie file header failed, headerLength=%d", headerLength);
            return RES_ERROR;
        }

        if (VerifyFileChecksum() != RES_SUCC)
        {
            LOG_ERROR("VerifyFileChecksum failed");
            return RES_ERROR;
        }

        return RES_SUCC;
    }

	int FIFOFileStorage::ReadFileInfo()
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN;
		}
        size_t fileOffset = 0;
        //size_t  checkCount = 0;
        PhysicalBlockInfo block = {};

        AssertAbort(m_FileHeader.physicalBlockCount >= 0 && m_FileHeader.fileSize > 0);

        fileOffset += sizeof(struct FileHeader);

        for (size_t num = 0; num < m_FileHeader.physicalBlockCount; num++)
        {
            if (fseek(m_FileHandle, (long)fileOffset, SEEK_SET) != 0)
            {
                LOG_ERROR("fseek file failed, file offset = %d", fileOffset);
                return RES_ERROR;
            }

            ///< read block info
            if (Read((char *)(&block), sizeof(PhysicalBlockInfo)) != RES_SUCC)
            {
                LOG_ERROR("read block info failed, offset = %d", fileOffset);
                return RES_ERROR;
            }

         /*   if (block.CheckSum != CHECKSUM_NUM)
            {
                LOG_ERROR("verify block checksum number failed, expect=[%u], auctual=[%u]",
                    CHECKSUM_NUM, block.CheckSum);
                return RES_ERROR;
            }*/

            m_PhysicalBlocks.push_back(block);

            fileOffset += m_FileHeader.blockSize + sizeof(PhysicalBlockInfo);

			if (fileOffset > m_FileHeader.fileSize)
			{
				break;
			}
            AssertAbort(fileOffset <= m_FileHeader.fileSize);
        }

//        AssertAbort(fileOffset + m_FileHeader.blockSize + sizeof(PhysicalBlockInfo) == m_FileHeader.fileSize);

        //check for invalid block and reclaim in memory
        CheckBlockInfo();
        BuildIndexInfo();

        return RES_SUCC;
    }

    void FIFOFileStorage::Free(char** buff)
    {
        if (*buff)
        {
            LOG_TRACE(":[DE-ALLOCATE]:%d buffer=%p", __LINE__, *buff);
            delete[] *buff;
            *buff = nullptr;
        }
    }

    /** \brief flush data to filesystem.
    */
    void FIFOFileStorage::Flush()
    {
		if (nullptr == m_FileHandle)
		{
            return;
		}
        if (PrepareFileForWrite() == RES_SUCC)
        {
            LOG_TRACE("FIFOFileStorage::Flush() begin");
            //sync data to disk
            std::fflush(m_FileHandle);
            LOG_TRACE("FIFOFileStorage::Flush() end");
        }
    }

	int FIFOFileStorage::PrepareFileForWrite()
    {
        if (m_FileStatus == FILE_READY)
        {
            return RES_SUCC;
        }

        if (m_FileStatus != FILE_OPEN)
        {
            LOG_ERROR("file has not been opened");
            return RES_FILE_NOT_OPEN;
        }

        if (AdjustFileSize(m_FileOpenSize) != RES_SUCC)
        {
            Close();
            LOG_ERROR("AdjustFileSize failed, file can not be ready for use");
            return RES_FILE_ADJUST_SIZE_FAIL;
        }

        m_FileStatus = FILE_READY;
        return RES_SUCC;
    }

	int FIFOFileStorage::SaveItem(
        const char* buffer, 
        size_t bufferSize,
        const StorageItemKey* pStorageItemKey /* Optional */)
    {
        LOG_TRACE("save buffer begin: size=%d", bufferSize);
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN; 
		}

		int ret = PrepareFileForWrite();
        if (ret != RES_SUCC)
        {
            LOG_ERROR("invoke save failed, file has not been opened or cannot resize");
            return ret;
        }

        if (bufferSize >= m_FileHeader.physicalBlockCount * m_FileHeader.blockSize)
        {
            LOG_ERROR("save buffer size is too big, size = [%d]", bufferSize);
            return RES_DATA_LARGE;
        }

        if (m_LogicalBlocks.size() == 0 && m_PhysicalBlocks.size() == 0)
        {
            LOG_ERROR("Fatal error, block queue is empty");
            return RES_ERROR;
        }

        if (buffer == nullptr || bufferSize == 0)
        {
            LOG_WARN("save buffer to file with empty data, buffer=%p, size=%u", buffer, bufferSize);
            return RES_SUCC;
        }

        size_t  requiredPhysicalBlocks = bufferSize / m_FileHeader.blockSize;
        if (bufferSize % m_FileHeader.blockSize != 0)
        {
            requiredPhysicalBlocks++;
        }
        LOG_TRACE("Current block size is %u, needs %u blocks for data of %d bytes.", m_FileHeader.blockSize, requiredPhysicalBlocks, bufferSize);

        AssertAbort(requiredPhysicalBlocks < 0xFFFF);

        // save one block
        PhysicalBlockInfo* physicalBlockInfo = nullptr;
		size_t remainingBufferSize = bufferSize;
		size_t logicalBlockSizeInBytes = 0;
		int prevBlockId = BEG_OF_BLOCK;
        std::vector<size_t> blockIdList;

        LogicalBlockInfo  logicalBlockInfo = {};
        logicalBlockInfo.FirstPhysicalBlockIndex = -1;

        if (pStorageItemKey != nullptr)
        {
            logicalBlockInfo.Key.CopyFrom(*pStorageItemKey);

            if (logicalBlockInfo.Key.Time == 0)
            {
                logicalBlockInfo.Key.Time = PAL::getUtcSystemTimeMs();
            }
        }
        else
        {
            logicalBlockInfo.Key.Id[0] = '\0';
            logicalBlockInfo.Key.Time = PAL::getUtcSystemTimeMs();
            logicalBlockInfo.Key.Priority = 0;
        }

        for (size_t i = 0; i < requiredPhysicalBlocks; i++)
        {
			int blockId = FindFreePhysicalBlockIndex();

            if (blockId == -1)
            {
                AssertAbort(m_LogicalBlocks.size() >= 0);
                if (m_LogicalBlocks.size() > 0)
                {
                    //if there is no block exist, release one oldest block
                    LOG_TRACE("No free block, overwrite the oldest block.");

                    LogicalBlockInfo firstLogicalBlock = GetOldestLogicalBlock();
                    ReleaseBlocks(firstLogicalBlock);
                    m_lastOverwrittenSizeInBytes += firstLogicalBlock.SizeInBytes;

                    m_LogicalBlocks.erase(m_LogicalBlocks.begin());

                    // Re-run this iteration now that we have found one or more new physical blocks
                    i--;
                    continue;
                }
            }

            AssertAbort(blockId >= 0);
            blockIdList.push_back(blockId);

            if (logicalBlockInfo.FirstPhysicalBlockIndex == -1)
            {
                logicalBlockInfo.FirstPhysicalBlockIndex = blockId;
                logicalBlockInfo.PhysicalBlockCount = 1;
            }
            else
            {
                logicalBlockInfo.PhysicalBlockCount++;
                physicalBlockInfo->NextBlockId = blockId;
            }

            physicalBlockInfo = &(m_PhysicalBlocks[blockId]);

			size_t physicalBlockSizeInBytes = 0;
            if (remainingBufferSize > m_FileHeader.blockSize)
            {
                physicalBlockSizeInBytes = m_FileHeader.blockSize;
                remainingBufferSize -= m_FileHeader.blockSize;
            }
            else
            {
                physicalBlockSizeInBytes = remainingBufferSize;
                remainingBufferSize = 0;
            }

            physicalBlockInfo->SizeInBytes = physicalBlockSizeInBytes;
            physicalBlockInfo->NextBlockId = END_OF_BLOCK;
            physicalBlockInfo->Key.CopyFrom(logicalBlockInfo.Key);

            if (prevBlockId != BEG_OF_BLOCK)
            {
                physicalBlockInfo->PreviousBlockId = prevBlockId;
            }
            else
            {
                physicalBlockInfo->PreviousBlockId = BEG_OF_BLOCK;
            }

            logicalBlockSizeInBytes += physicalBlockSizeInBytes;

            prevBlockId = blockId;
        }

        logicalBlockInfo.SizeInBytes = logicalBlockSizeInBytes;

        if (UpdateBlockToDisk(logicalBlockInfo, buffer, bufferSize) != RES_SUCC)
        {
            //rollback if failed
            for (size_t i = 0; i < blockIdList.size(); i++)
            {
                PhysicalBlockInfo* pBlockInfo = &(m_PhysicalBlocks[blockIdList[i]]);
                pBlockInfo->SizeInBytes = 0;
            }
            LOG_ERROR("save buffer to file fail, offset=%llu, size=%d", physicalBlockInfo->FileOffset, bufferSize);
            return RES_ERROR;
        }

        BlockKey logicalBlockKey = {};
        logicalBlockKey.BlockId = logicalBlockInfo.FirstPhysicalBlockIndex;
        logicalBlockKey.ItemKey.CopyFrom(logicalBlockInfo.Key);

        m_LogicalBlocks.insert(std::make_pair(logicalBlockKey, logicalBlockInfo));

        LOG_TRACE("save buffer to file suc, offset=%llu, size=%d", physicalBlockInfo->FileOffset, bufferSize);

        return RES_SUCC;
    }

    size_t FIFOFileStorage::CalculateFileSize(size_t fileSize, size_t alignSize)
    {
        size_t uSize = fileSize;

        //return ALIGN_TO_POWER(uSize, alignSize);
        return FIFOFileStorage::AlignToPower(uSize, alignSize);
    }

	int FIFOFileStorage::GenerateFile()
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN; 
		}
        //generate file use fileSize
        size_t pos = m_FileHeader.fileSize - 1;
        if (fseek(m_FileHandle, (long)pos, SEEK_SET) != 0)
        {
            LOG_ERROR("file seek failed for generating file, offset=[%d]", m_FileHeader.fileSize);
            return RES_ERROR;
        }

        if (Write(" ", 1) != RES_SUCC)
        {
            LOG_ERROR("generate file failed, please check if the disk space is enough");
            return RES_ERROR;
        }

        return RES_SUCC;
    }

	size_t FIFOFileStorage::FillFileHeader(size_t givenFileSize, size_t blockSizeInBytes)
    {
        //revise the filesize according to the block size
        size_t fileSize = CalculateFileSize(givenFileSize, blockSizeInBytes + sizeof(PhysicalBlockInfo));

        m_FileHeader.magicNumber = (MAGIC_NUM << 4) | (FORMAT_VERSION & 0x0F);
        m_FileHeader.blockSize = blockSizeInBytes;
        m_FileHeader.physicalBlockCount = static_cast<size_t>(fileSize / (m_FileHeader.blockSize + sizeof(PhysicalBlockInfo)));

        AssertAbort(fileSize % (m_FileHeader.blockSize + sizeof(PhysicalBlockInfo)) == 0);

        m_FileHeader.fileSize = fileSize + sizeof(m_FileHeader);

        return RES_SUCC;
    }

	int FIFOFileStorage::VerifyFileChecksum()
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN;
		}
        std::uint64_t magicNum = m_FileHeader.magicNumber >> 4;
		size_t version = m_FileHeader.magicNumber & 0xF;

        if (magicNum != MAGIC_NUM)
        {
            LOG_ERROR("check magic num failed, expect=[%llu], actual=[%llu]",
                MAGIC_NUM, magicNum);
            return RES_ERROR;
        }

        if (FORMAT_VERSION != version)
        {
            LOG_ERROR("check version failed, expect=[%llu], acutal=[%llu]",
                FORMAT_VERSION, version);
            return RES_ERROR;
        }

		fseek(m_FileHandle, 0, SEEK_END);   // non-portable
        size_t size = ftell(m_FileHandle);
		fseek(m_FileHandle, 0, SEEK_SET);   // non-portable
/*		fpos_t pos = EOF;
		std::fsetpos(m_FileHandle, &pos);
		size_t size = std::fgetpos(m_FileHandle);
		pos = 0;
		std::fsetpos(m_FileHandle, &pos);   // non-portable
*/
        if (m_FileHeader.fileSize != size)
        {
            LOG_ERROR("file size is not correct, expect=[%llu], actual=[%llu]",
                m_FileHeader.fileSize, size);
            return RES_ERROR;
        }
        return RES_SUCC;
    }

	int FIFOFileStorage::FindFreePhysicalBlockIndex()
    {
        for (size_t blockIndex = 0; blockIndex < m_PhysicalBlocks.size(); blockIndex++)
        {
            if (!m_PhysicalBlocks[blockIndex].IsBlockInUse())
            {
                return static_cast<int>(blockIndex);
            }
        }

        return -1;
    }

    void FIFOFileStorage::ReleaseBlocks(LogicalBlockInfo& idx)
    {
        AssertAbort( idx.FirstPhysicalBlockIndex >= 0 && 
			         idx.PhysicalBlockCount > 0 && 
			         idx.FirstPhysicalBlockIndex < static_cast<int>(m_FileHeader.physicalBlockCount));

        UpdateBlockToDisk(idx, nullptr, 0);
    }

    void FIFOFileStorage::Reset()
    {
        memset(&m_FileHeader, 0, sizeof(m_FileHeader));
        m_FileHandle = nullptr;
        m_FileStatus = FILE_CLOSE;
        m_FileOpenSize = 0;
        m_PhysicalBlocks.clear();
        m_LogicalBlocks.clear();
    }

    size_t FIFOFileStorage::DeleteFileLocal(const char* filePath)
    {
        // FIXME: [MG] --- this will break on Windows 7 !!!
        std::remove(filePath);
        return RES_SUCC;
    }

    std::uint64_t FIFOFileStorage::GetFileSize()
    {
        return m_FileHeader.fileSize;
    }

    void FIFOFileStorage::GetFileStats(FileStats* pFileStats)
    {
        pFileStats->fileContentsSize = _GetFileContentsSize();
        pFileStats->lastOverWrittenSizeInBytes = m_lastOverwrittenSizeInBytes;

        m_lastOverwrittenSizeInBytes = 0;
    }

    bool FIFOFileStorage::IsStorageEmpty()
    {
        return m_LogicalBlocks.size() == 0;
    }

    size_t FIFOFileStorage::AlignToPower(size_t size, size_t alignSize)
    {
        size_t m = size % alignSize;
        if (m > 0)
        {
            size_t n = size / alignSize;
            // use number larger than size for not attempting to adjust file size next time
            return n * alignSize + alignSize;
        }

        return size;
    }

    size_t FIFOFileStorage::GetBufferSize()
    {
        if (m_LogicalBlocks.size() == 0)
        {
            return 0;
        }

        return GetOldestLogicalBlock().SizeInBytes;
    }

    //////////////////////////////////////////////////////////////////////////
    /// This function ensures that all the blocks are in good state.
    ///
    /// A logical block can be bigger than a physical block (for e.g. predefined size of 32 KB)
    /// in order to store bigger blocks, each block contains next block id and previous block id
    /// and maintains related physical blocks like a doubly linked list
    ///
    /// if the next block id of a physical block is "Not End" and the next block
    /// info does not match the expectation - for e.g. Next Block Info shows that its size is 0 -
    /// then the chain is broken. For that case, we reset the current block's SizeInBytes as 0
    /// and reclaim that block [in other words data is lost]
    ///
    /// We do a similar check for the previous block. 
    ///
    /// Question: We don't seem to reset the block id. Maybe, it's fine because
    /// we determine the block usages using BlockInUse - which uses the 
    /// sizeInBytes field
    ///
    //////////////////////////////////////////////////////////////////////////
    void FIFOFileStorage::CheckBlockInfo()
    {
        for (size_t i = 0; i < m_PhysicalBlocks.size(); i++)
        {
            PhysicalBlockInfo& block = m_PhysicalBlocks[i];

            if (!block.IsBlockInUse())
            {
                //unused block
                continue;
            }

            if (block.NextBlockId != END_OF_BLOCK)
            {
                if (block.NextBlockId < 0 || block.NextBlockId >= static_cast<int>(m_PhysicalBlocks.size()))
                {
                    block.SizeInBytes = 0;
                }
                else 
                {
                    const PhysicalBlockInfo& nextBlock = m_PhysicalBlocks[block.NextBlockId];
                    if (!nextBlock.IsBlockInUse() || nextBlock.PreviousBlockId != static_cast<int>(i))
                    {
                        ///< these two blocks are all broken and need be reclaimed
                        block.SizeInBytes = 0;
                    }
                }
            }

            if (block.PreviousBlockId != BEG_OF_BLOCK)
            {
                if (block.PreviousBlockId < 0 || block.PreviousBlockId >= static_cast<int>(m_PhysicalBlocks.size()))
                {
                    block.SizeInBytes = 0;
                }
                else 
                {
                    const PhysicalBlockInfo& prevBlock = m_PhysicalBlocks[block.PreviousBlockId];
                    if (!prevBlock.IsBlockInUse() || prevBlock.NextBlockId != static_cast<int>(i))
                    {
                        block.SizeInBytes = 0;
                    }
                }
            }
        }
    }

    void FIFOFileStorage::BuildIndexInfo()
    {
		size_t logicalBlockCount = 0;
		size_t indexedBlocksCount = 0;

        std::map<BlockKey, PhysicalBlockInfo*, BlockKeyComparer>  blockMap;       

        // Read the first physical block of each logical blocks. There's no 
        // notion of logical blocks in the file itself except it's done with the 
        // help of physical block properties. 
        for (size_t i = 0; i < m_PhysicalBlocks.size(); i++)
        {
            PhysicalBlockInfo* pCurrentBlock = &(m_PhysicalBlocks[i]);

            // Count only the first physical block as a valid block. for e.g. if there 
            // are three physical block constituting a block { block 10 --> block 23 --> block 45 } 
            // in this case except for block 10, both block 23 and block 45 will be pointing to
            // other blocks. Hence don't count them towards overall count
            if (!pCurrentBlock->IsBlockInUse() || pCurrentBlock->PreviousBlockId != BEG_OF_BLOCK)
            {
                continue;
            }

            logicalBlockCount++;

            BlockKey physicalBlockKey = {};
            physicalBlockKey.BlockId = i;
            physicalBlockKey.ItemKey.CopyFrom(pCurrentBlock->Key);

            blockMap.insert(std::make_pair(physicalBlockKey, pCurrentBlock));
        }
        // Build logical blocks from physical blocks
        for (std::map<BlockKey, PhysicalBlockInfo*, BlockKeyComparer>::iterator iter = blockMap.begin(); iter != blockMap.end(); ++iter)
        {
            LogicalBlockInfo logicalBlockInfo = {};

            logicalBlockInfo.SizeInBytes = 0;
            logicalBlockInfo.FirstPhysicalBlockIndex = static_cast<int>((iter->first).BlockId);
            logicalBlockInfo.PhysicalBlockCount = 0;
            logicalBlockInfo.Key.CopyFrom(iter->first.ItemKey);

            PhysicalBlockInfo* pCurrentBlock = iter->second;
            bool loopDetected = false;

            while (1)
            {
                logicalBlockInfo.SizeInBytes += pCurrentBlock->SizeInBytes;
                logicalBlockInfo.PhysicalBlockCount++;
                indexedBlocksCount++;

                if (logicalBlockInfo.PhysicalBlockCount > m_PhysicalBlocks.size())
                {
                    loopDetected = true;

                    // Reclaim the entire logical block because the chain is broken
                    indexedBlocksCount -= logicalBlockInfo.PhysicalBlockCount;
                    LOG_WARN("loop detected when building index info");

                    pCurrentBlock = &m_PhysicalBlocks[logicalBlockInfo.FirstPhysicalBlockIndex];
                    while (pCurrentBlock->IsBlockInUse()) 
                    {
                        pCurrentBlock->SizeInBytes = 0;
                        pCurrentBlock = &(m_PhysicalBlocks[pCurrentBlock->NextBlockId]);
                    }
                    break;
                }

                if (pCurrentBlock->NextBlockId == END_OF_BLOCK)
                {
                    break;
                }
                pCurrentBlock = &(m_PhysicalBlocks[pCurrentBlock->NextBlockId]);
            }

            if (!loopDetected)
            {
                BlockKey logicalBlockKey = {};
                logicalBlockKey.BlockId = logicalBlockInfo.FirstPhysicalBlockIndex;
                logicalBlockKey.ItemKey.CopyFrom(logicalBlockInfo.Key);

                m_LogicalBlocks.insert(std::make_pair(logicalBlockKey, logicalBlockInfo));
            }
        }

        if (logicalBlockCount != indexedBlocksCount)
        {
            std::vector<bool> isPhysicalBlockIndexed(m_FileHeader.physicalBlockCount);
            for (LogicalBlockIterator iter = m_LogicalBlocks.begin(); iter != m_LogicalBlocks.end(); ++iter)
            {
                LogicalBlockInfo logicalBlockInfo = iter->second;
				size_t physicalBlockIndex = logicalBlockInfo.FirstPhysicalBlockIndex;
                const PhysicalBlockInfo* pPhysicalBlockInfo = &m_PhysicalBlocks[physicalBlockIndex];

                while (1)
                {
                    isPhysicalBlockIndexed[physicalBlockIndex] = true;

                    if (pPhysicalBlockInfo->NextBlockId == END_OF_BLOCK) 
                    {
                        break;
                    }

                    physicalBlockIndex = pPhysicalBlockInfo->NextBlockId;
                    pPhysicalBlockInfo = &(m_PhysicalBlocks[physicalBlockIndex]);
                }
            }

            // Visit each physical block and reset the ones that are not referenced
            // by logical blocks
            for (size_t i = 0; i < m_PhysicalBlocks.size(); i++)
            {
                if (m_PhysicalBlocks[i].SizeInBytes != 0 && isPhysicalBlockIndexed[i] == false)
                {
                    m_PhysicalBlocks[i].SizeInBytes = 0;
                }
            }
        }
    }

	int FIFOFileStorage::WriteFileHeader()
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN;
		}
		rewind(m_FileHandle);
		
        if (Write((char *)&m_FileHeader, sizeof(FileHeader)) != RES_SUCC)
        {
            LOG_ERROR("save file header failed");
            return RES_ERROR;
        }
        return RES_SUCC;
    }

	int FIFOFileStorage::WriteFileInfo()
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN;
		}
        for (size_t i = 0; i < m_PhysicalBlocks.size(); i++)
        {
            size_t pos = (size_t)(m_PhysicalBlocks[i].FileOffset);
            if (fseek(m_FileHandle, (long)pos, SEEK_SET) != 0)
            {
                LOG_ERROR("seek file failed, offset=%llu", m_PhysicalBlocks[i].FileOffset);
                return RES_ERROR;
            }

            if (Write((char *)&(m_PhysicalBlocks[i]), sizeof(PhysicalBlockInfo)) != RES_SUCC)
            {
                LOG_ERROR("save init file info (block) failed, offset=%llu", m_PhysicalBlocks[i].FileOffset);
                return RES_ERROR;
            }
        }
        return RES_SUCC;
    }

	int FIFOFileStorage::UpdateBlockToDisk(LogicalBlockInfo& idx, const char* buffer, size_t buffSize)
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN;
        }
        AssertAbort(idx.FirstPhysicalBlockIndex >= 0 && idx.PhysicalBlockCount > 0 && idx.FirstPhysicalBlockIndex < static_cast<int>(m_FileHeader.physicalBlockCount));

        PhysicalBlockInfo* pPhysicalBlockInfo = &(m_PhysicalBlocks[idx.FirstPhysicalBlockIndex]);

		size_t offset = 0;
		size_t checkCount = 0;

        while (pPhysicalBlockInfo != nullptr)
        {
            checkCount++;
            size_t pos = (size_t)(pPhysicalBlockInfo->FileOffset);

            if (fseek(m_FileHandle, (long)pos, SEEK_SET) != 0)
            {
                LOG_ERROR("file seek failed, block offset in file is [%llu]", pPhysicalBlockInfo->FileOffset);
                return RES_ERROR;
            }

            if (buffer == nullptr || buffSize == 0)
            {
                pPhysicalBlockInfo->SizeInBytes = 0;
            }

            ///< write block header
            if (Write((char *)pPhysicalBlockInfo, sizeof(PhysicalBlockInfo)) != RES_SUCC)
            {
                LOG_ERROR("save block info failed, blockOffset=%llu, buffer size=%d",
                    pPhysicalBlockInfo->FileOffset, buffSize);
                return RES_ERROR;
            }

            if (buffer != nullptr && buffSize != 0)
            {
                if (Write(buffer + offset, pPhysicalBlockInfo->SizeInBytes) != RES_SUCC)
                {
                    LOG_ERROR("save buffer to file failed, blockOffset=%llu, buffer size=%d",
                        pPhysicalBlockInfo->FileOffset, buffSize);
                    return RES_ERROR;
                }

                offset += pPhysicalBlockInfo->SizeInBytes;
            }

            if (pPhysicalBlockInfo->NextBlockId == END_OF_BLOCK)
            {
                break;
            }

            pPhysicalBlockInfo = &(m_PhysicalBlocks[pPhysicalBlockInfo->NextBlockId]);
        }

        AssertAbort(checkCount == idx.PhysicalBlockCount);
        AssertAbort(buffSize == offset);
        //AssertAbort(offset == idx.SizeInBytes);

        return RES_SUCC;
    }

	int  FIFOFileStorage::AdjustFileSize(size_t newFileSize)
    {
		if (nullptr == m_FileHandle)
		{
            return RES_FILE_NOT_OPEN;
		}
        if (m_FileHeader.fileSize - sizeof(m_FileHeader) >= newFileSize)
        {
            LOG_TRACE("new file size is equal or smaller than old file, use the old file");
            return RES_SUCC;
        }

        LOG_TRACE("AdjustFileSize from %lld to %d begin", m_FileHeader.fileSize, newFileSize);

        std::uint64_t offset = m_FileHeader.fileSize;
        size_t incrFileSize = CalculateFileSize(newFileSize - m_FileHeader.fileSize + sizeof(m_FileHeader),
			                                     m_FileHeader.blockSize + sizeof(PhysicalBlockInfo));

		size_t incrBlockNum = static_cast<size_t>(incrFileSize / (m_FileHeader.blockSize + sizeof(PhysicalBlockInfo)));
        m_FileHeader.physicalBlockCount += incrBlockNum;

        AssertAbort(incrFileSize % (m_FileHeader.blockSize + sizeof(PhysicalBlockInfo)) == 0);

        m_FileHeader.fileSize += incrFileSize;

        if (GenerateFile() != RES_SUCC)
        {
            LOG_ERROR("extend file failed, new file size = %llu, old file size = %llu",
                m_FileHeader.fileSize, m_FileHeader.fileSize - incrFileSize);
            m_FileHeader.fileSize -= incrFileSize;
            m_FileHeader.physicalBlockCount -= incrBlockNum;
            return RES_FILE_ADJUST_SIZE_FAIL;
        }

        size_t  count = 0;
        PhysicalBlockInfo  blockInfo ={};

        while (count++ < incrBlockNum)
        {
            blockInfo.CheckSum = CHECKSUM_NUM;
            blockInfo.FileOffset = offset;
            blockInfo.SizeInBytes = 0;
            blockInfo.Key.Time = 0;
            blockInfo.NextBlockId = END_OF_BLOCK;
            blockInfo.PreviousBlockId = BEG_OF_BLOCK;
            m_PhysicalBlocks.push_back(blockInfo);
            offset += m_FileHeader.blockSize + sizeof(PhysicalBlockInfo);
        }

 //       AssertAbort(offset + m_FileHeader.blockSize + sizeof(PhysicalBlockInfo) ==  m_FileHeader.fileSize);

        //flush the file info to disk
        if (WriteFileInfo() == RES_SUCC && WriteFileHeader() == RES_SUCC)
        {
            LOG_TRACE("AdjustFileSize success");
        }
        else
        {
            //reverse 
            m_FileHeader.fileSize -= incrFileSize;
            m_FileHeader.physicalBlockCount -= incrBlockNum;
            for (size_t i = 0; i < incrBlockNum; i++)
            {
                m_PhysicalBlocks.pop_back();
            }
            RecreateFile();
            LOG_ERROR("update file info to disk failed");
            return RES_FILE_ADJUST_SIZE_FAIL;
        }

        //sync file structure to disk
        LOG_TRACE("flush file structure to disk in AdjustFileSize");
        std::fflush(m_FileHandle);
        LOG_TRACE("flush finished in AdjustFileSize");
        return RES_SUCC;
    }

    DATARV_ERROR FIFOFileStorage::_FindNextItem(bool firstItem, FindItemInfo* pFindItemInfo)
    {
		if (pFindItemInfo == nullptr)
		{
			return DATARV_ERROR_INVALID_ARG;
		}
		if (!(m_FileStatus == FILE_OPEN || m_FileStatus == FILE_READY))
		{
			return  DATARV_ERROR_FILE_NOTOPEN;
		}

        /* Avoid triggering error in the log if storage is empty or no more items to be found */
        if (m_LogicalBlocks.size() == 0)
        {
            return DATARV_ERROR_FILE_NOMOREITEMS;
        }

        if (firstItem)
        {
            m_findIter = m_LogicalBlocks.begin();
        }
        else
        {
            ++m_findIter;
        }

        if (m_findIter == m_LogicalBlocks.end())
        {
            return DATARV_ERROR_FILE_NOMOREITEMS;
        }

        LogicalBlockInfo logicalBlockInfo = m_findIter->second;
        pFindItemInfo->ItemSize = logicalBlockInfo.SizeInBytes;
        pFindItemInfo->Key.CopyFrom(logicalBlockInfo.Key);

        return DATARV_ERROR_OK;
    }

    DATARV_ERROR FIFOFileStorage::FindFirstItem(FindItemInfo* pFindItemInfo)
    {
        return _FindNextItem(true, pFindItemInfo);
    }

    DATARV_ERROR FIFOFileStorage::FindNextItem(FindItemInfo* pFindItemInfo)
    {
        return _FindNextItem(false, pFindItemInfo);
    }

    DATARV_ERROR FIFOFileStorage::RemoveItemAndMoveNext(
        FindItemInfo* pFindItemInfo,
        bool* pHasMoreItems,
        char** ppBuffer, 
        size_t* pBufferSize)
    {
		if (pFindItemInfo == nullptr)
		{
			return DATARV_ERROR::DATARV_ERROR_INVALID_ARG;
		}

		if (!(m_FileStatus == FILE_OPEN || m_FileStatus == FILE_READY))
		{
			return DATARV_ERROR_FILE_NOTOPEN;
		}
		if (!(m_LogicalBlocks.size() > 0))
		{
			return  DATARV_ERROR_FILE_NOMOREITEMS;
		}
		if(!(m_findIter != m_LogicalBlocks.end()))
		{
			return  DATARV_ERROR_FILE_NOMOREITEMS;
		}

        LogicalBlockInfo logicalBlockInfo = m_findIter->second;

        std::unique_ptr<char> autoBuffer;
        size_t bytesRead = 0;
        char* pBuffer = nullptr;

        if ((ppBuffer != nullptr) && (pBufferSize != nullptr))
        {
			DATARV_ERROR error = _ReadItem(
                                    logicalBlockInfo, 
                                    logicalBlockInfo.SizeInBytes,
                                    nullptr, /* we already have file item info */
                                    &pBuffer,
                                    &bytesRead);

			if (DATARV_ERROR_OK != error)
			{
				return error;
			}

            autoBuffer.reset(pBuffer);
            pBuffer = nullptr;
        }

		if (UpdateBlockToDisk(logicalBlockInfo, nullptr, 0) != RES_SUCC)
		{
			return  DATARV_ERROR_FAIL;
		}

        // Store the current item, move to item and delete the saved item to not corrupt the iterator
        LogicalBlockIterator itemToRemove = m_findIter;

        *pHasMoreItems = _FindNextItem(false, pFindItemInfo) == DATARV_ERROR_OK;
        m_LogicalBlocks.erase(itemToRemove);

        if ((ppBuffer != nullptr) && (bytesRead != 0))
        {
            *ppBuffer = autoBuffer.release();
            *pBufferSize = bytesRead;
        }

        return DATARV_ERROR_OK;
    }

    DATARV_ERROR FIFOFileStorage::ReadItem(
        const FindItemInfo& findItemInfo, 
        size_t readSize, 
        char** ppBuffer, 
        size_t* pBufferSize)
    {
		UNREFERENCED_PARAMETER(findItemInfo);
		if (readSize == 0 ||
			ppBuffer == nullptr ||
			pBufferSize == nullptr)
		{
			return DATARV_ERROR_INVALID_ARG;
		}

		if (!(m_FileStatus == FILE_OPEN ||	m_FileStatus == FILE_READY))
		{
			return DATARV_ERROR_FILE_NOTOPEN;
		}

		if (m_findIter == m_LogicalBlocks.end())
		{
			return DATARV_ERROR_FILE_NOMOREITEMS;
		}

        // Reconstruct LogicalBlockIndexInfo from FindItemInfo
        LogicalBlockInfo logicalBlockInfo = m_findIter->second;
		
		return _ReadItem(logicalBlockInfo, readSize, nullptr, ppBuffer, pBufferSize);
    }

    DATARV_ERROR FIFOFileStorage::_ReadItem(
        const LogicalBlockInfo& logicalBlockInfo,
        size_t readSize, 
        StorageItemKey* pStorageItemKey,
        char** ppBuffer, 
        size_t* pBufferSize)
    {
		if (nullptr == m_FileHandle)
		{
			return DATARV_ERROR_FILE_NOTOPEN;
		}

		if (readSize == 0 ||
			ppBuffer == nullptr ||
			pBufferSize == nullptr)
		{
			return DATARV_ERROR_INVALID_ARG;
		}
		
		if (!(m_FileStatus == FILE_OPEN || m_FileStatus == FILE_READY))
		{
			return DATARV_ERROR_FILE_NOTOPEN;
		}
		        
		if (!(logicalBlockInfo.FirstPhysicalBlockIndex < static_cast<int>(m_PhysicalBlocks.size())))
		{
			return DATARV_ERROR_FAIL;
		}
        PhysicalBlockInfo blockInfo = m_PhysicalBlocks[logicalBlockInfo.FirstPhysicalBlockIndex];

        std::unique_ptr<char> pBuffer(new char[readSize]);
		if (nullptr == pBuffer.get())
		{
			return DATARV_ERROR_OUTOFMEMORY;
		}

        size_t count = 0;
		size_t bytesRead = 0;
        while (count++ <= m_FileHeader.physicalBlockCount)
        {
            size_t pos = (size_t)(blockInfo.FileOffset + sizeof(PhysicalBlockInfo));
		
            size_t seekResult = fseek(m_FileHandle, (long)pos, SEEK_SET);
			
			if (!(seekResult == RES_SUCC))
			{
				return DATARV_ERROR_FAIL;
			}

			size_t readResult = Read(pBuffer.get() + bytesRead, blockInfo.SizeInBytes);
			if (!(readResult == RES_SUCC))
			{
				return  DATARV_ERROR_FAIL;
			}

            bytesRead += blockInfo.SizeInBytes;

            if (blockInfo.NextBlockId == END_OF_BLOCK)
            {
                break;
            }
            else
            {
                blockInfo = m_PhysicalBlocks[blockInfo.NextBlockId];
            }
        }

        if (pStorageItemKey != nullptr)
        {
            pStorageItemKey->CopyFrom(logicalBlockInfo.Key);
        }

        *ppBuffer = pBuffer.release();
        *pBufferSize = bytesRead;

        return DATARV_ERROR_OK;
    }

    /**
    * Return the actual item size.
    * return less than 0 - error
    * return 0 storage is empty
    */
	int FIFOFileStorage::PopNextItem(char** buffer, size_t& size, StorageItemKey* pStorageItemKey /* Optional */)
    {
		if (!(m_FileStatus == FILE_OPEN || m_FileStatus == FILE_READY))
		{
			LOG_ERROR("invoke load failed, file has not been opened");
			return RES_FILE_NOT_OPEN;
		}

        if (m_LogicalBlocks.size() == 0)
        {
            LOG_TRACE("there is no data in offline storage");
            return 0;
        }

        //size_t offset = 0;
        LogicalBlockInfo logicalBlockInfo = GetOldestLogicalBlock();

        size_t bytesRead = 0;
        char* pBuffer = nullptr;
        DATARV_ERROR status = _ReadItem(logicalBlockInfo, logicalBlockInfo.SizeInBytes, pStorageItemKey, &pBuffer, &bytesRead);
        if (status != DATARV_ERROR_OK)
        {
            LOG_ERROR("ReadItem failed with error=%d", status);
            return RES_ERROR;
        }

        std::unique_ptr<char> autoBuffer(pBuffer);
        pBuffer = nullptr;

        if (UpdateBlockToDisk(logicalBlockInfo, nullptr, 0) != RES_SUCC)
        {
            LOG_ERROR("update block to disk failed");
            return RES_ERROR;
        }

        m_LogicalBlocks.erase(m_LogicalBlocks.begin());

        *buffer = autoBuffer.release();
        size = bytesRead;
        return static_cast<int>(bytesRead);
    }

    size_t FIFOFileStorage::_GetFileContentsSize()
    {
        size_t contentsSize = 0;

        for (LogicalBlockIterator iter = m_LogicalBlocks.begin(); iter != m_LogicalBlocks.end(); ++iter)
        {
            const LogicalBlockInfo& logicalBlockInfo = iter->second;
            contentsSize += logicalBlockInfo.SizeInBytes;
        }
            
        return contentsSize;
    }
} ARIASDK_NS_END

