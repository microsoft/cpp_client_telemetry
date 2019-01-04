// Copyright (c) Microsoft. All rights reserved .

#include "common/Common.hpp"
//#include "common/MockIStorage.hpp"
#include "offline/FIFOFileStorage.hpp"

using namespace testing;
using namespace ARIASDK_NS;
using namespace std;

class FIFOOfflineStorageTests : public StrictMock<Test> {
protected:

protected:
    FIFOOfflineStorageTests()
    {
    }
};


/** \brief A unit test for FIFOFileStorageStorage::Open, FIFOFileStorageStorage::Save, FIFOFileStorageStorage::Load, FIFOFileStorageStorage::Close
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageBaseUnitTest)
{
    int ret;

    FIFOFileStorage  storeFile;
    int blockCount = 10;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    char filename[] = "OfflineStorageBaseUnitTest1.dat";
    char str[] = "aaaaaaaaaaaaaaaaaaaa";
    char *buf;
    size_t  size;

    storeFile.DeleteFileLocal(filename);

    ret = storeFile.PopNextItem(&buf, size, NULL);
    EXPECT_EQ(RES_FILE_NOT_OPEN, ret);
    ret = storeFile.SaveItem(str, strlen(str), NULL);
    EXPECT_EQ(RES_FILE_NOT_OPEN, ret);

    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);


    ret = storeFile.SaveItem(str, strlen(str), NULL);
    EXPECT_EQ(0, ret);


    ret = storeFile.PopNextItem(&buf, size, NULL);
    EXPECT_GT(ret, 0);
    EXPECT_EQ(static_cast<size_t>(ret), size);
    //EXPECT_STREQ(buf, str);
    EXPECT_EQ(0, std::memcmp(buf, str, size));
    storeFile.Free(&buf);

    //intentionally close twice
    storeFile.Close();
    storeFile.Close();

    //----------------------------------------------------

    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);
    ret = storeFile.SaveItem(str, strlen(str), NULL);
    EXPECT_EQ(0, ret);
    storeFile.Close();

    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);
    ret = storeFile.PopNextItem(&buf, size, NULL);
    EXPECT_GT(ret, 0);
    EXPECT_EQ(static_cast<size_t>(ret), size);
    //EXPECT_STREQ(str, buf);
    EXPECT_EQ(0, std::memcmp(buf, str, size));
    storeFile.Close();

    storeFile.DeleteFileLocal(filename);
}


/** \brief This is a unit test to verify the logic of FIFOFileStorage of saving data, whose size
is larger than file size, saving data, whose size is smaller than file size, and
loading data.
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageOverwriteUnitTest)
{
    char *buff = NULL;
    size_t  size;

    char fileName[] = "OfflineStorageOverwriteUnitTest.dat";
    char content[] = "aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeee";
    int  contLen = static_cast<int>(strlen(content));

    FIFOFileStorage storeFile;
    storeFile.DeleteFileLocal(fileName);

    size_t  smallFileSize = sizeof(PhysicalBlockInfo);
    EXPECT_EQ(0, storeFile.Open(
        fileName,
        smallFileSize, // Not leaving space for user data; Make sure that this is less than sufficient to write a block
        10 /* Less than the content */));

    //expect failed, because it larger than the file;
    EXPECT_EQ(RES_DATA_LARGE, storeFile.SaveItem(content, strlen(content), NULL));
    //EXPECT_EQ(0, storeFile.DeleteFileLocal(fileName));
    storeFile.Close();

    int blockCount = 10;
    int blockSize = 64;
    int fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;;

    EXPECT_EQ(0, storeFile.Open(fileName, fileSize, blockSize));
    EXPECT_EQ(0, storeFile.SaveItem(content, contLen, NULL));
    EXPECT_EQ(static_cast<size_t>(contLen), storeFile.GetBufferSize());
    //EXPECT_EQ(5, storeFile.GetBlockNum());
    EXPECT_EQ(0, storeFile.SaveItem(content, contLen, NULL));
    EXPECT_EQ(static_cast<size_t>(contLen), storeFile.GetBufferSize());
    EXPECT_EQ(0, storeFile.SaveItem(content, contLen, NULL));
    EXPECT_EQ(static_cast<size_t>(contLen), storeFile.GetBufferSize());
    //EXPECT_EQ(5, storeFile.GetBlockNum());
    EXPECT_EQ(contLen, storeFile.PopNextItem(&buff, size, NULL));

    //EXPECT_STREQ(buff, content);
    EXPECT_EQ(0, std::memcmp(buff, content, size));

    storeFile.Free(&buff);
    storeFile.Close();

    EXPECT_EQ(0, storeFile.Open(fileName, fileSize, blockSize));
    EXPECT_EQ(0, storeFile.SaveItem(content, contLen, NULL));
    EXPECT_EQ(static_cast<size_t>(contLen), storeFile.GetBufferSize());
    //EXPECT_EQ(5, storeFile.GetBlockNum());
    EXPECT_EQ(0, storeFile.SaveItem(content, contLen, NULL));
    EXPECT_EQ(static_cast<size_t>(contLen), storeFile.GetBufferSize());
    EXPECT_EQ(0, storeFile.SaveItem(content, contLen, NULL));
    EXPECT_EQ(static_cast<size_t>(contLen), storeFile.GetBufferSize());
    //EXPECT_EQ(5, storeFile.GetBlockNum());
    EXPECT_EQ(contLen, storeFile.PopNextItem(&buff, size, NULL));

    //EXPECT_STREQ(buff, content);
    EXPECT_EQ(0, std::memcmp(buff, content, size));

    storeFile.Free(&buff);
    storeFile.Close();
    storeFile.DeleteFileLocal(fileName);
}

/** \brief This is a unit test to verify the logic of FIFOFileStorage of saving to multiple blocks
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageMultiBlocksUnitTest)
{
    char *buff;
    size_t  size;
    int blockCount = 10;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    char filename[] = "OfflineStorageMultiBlocksUnitTest.dat";
    char content[] = "aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeee";
    int  contLen = static_cast<int>(strlen(content));

    FIFOFileStorage  storeFile;

    EXPECT_EQ(0, storeFile.Open(filename, fileSize, blockSize));
    EXPECT_EQ(0, storeFile.SaveItem(content, strlen(content), NULL));
    int ret = storeFile.PopNextItem(&buff, size, NULL);
    //EXPECT_STREQ(content, buff);
    EXPECT_EQ(0, std::memcmp(buff, content, size));
    EXPECT_EQ(static_cast<size_t>(ret), size);
    EXPECT_EQ(ret, contLen);
    EXPECT_EQ(0, storeFile.SaveItem(content, strlen(content), NULL));
    EXPECT_EQ(static_cast<size_t>(contLen), storeFile.GetBufferSize());
    storeFile.Free(&buff);
    storeFile.Close();

    EXPECT_EQ(0, storeFile.Open(filename, fileSize, blockSize));
    EXPECT_EQ(contLen, storeFile.PopNextItem(&buff, size, NULL));
    EXPECT_EQ(size, static_cast<size_t>(contLen));
    //EXPECT_STREQ(content, buff);
    EXPECT_EQ(0, std::memcmp(buff, content, size));
    storeFile.Free(&buff);
    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify the logic of FIFOFileStorage of saving data
and loading data.
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageMultiUnitTest)
{
    int ret;

    FIFOFileStorage  storeFile;
    int blockCount = 10;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    char filename[] = "OfflineStorageMultiUnitTest.dat";
    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);
    char data[5][21] = { "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbbbbb",
        "cccccccccccccccccccc",
        "dddddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeeee" };

    EXPECT_EQ(0, storeFile.SaveItem(data[0], strlen(data[0]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[1], strlen(data[1]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[2], strlen(data[2]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[3], strlen(data[3]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[4], strlen(data[4]), NULL));

    EXPECT_EQ(strlen(data[0]), storeFile.GetBufferSize());

    char *buf;
    size_t  size;

    for (size_t i = 0; storeFile.PopNextItem(&buf, size, NULL) > 0; i++)
    {
        //EXPECT_STREQ(buf, data[i]);
        EXPECT_EQ(0, std::memcmp(buf, data[i], size));
        storeFile.Free(&buf);
    }

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify the logic of FIFOFileStorage when the last block overwrites
the first block.
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageOverBlockUnitTest)
{
    int ret;

    FIFOFileStorage  storeFile;

    size_t  blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * 4;

    char filename[] = "OfflineStorageOverBlockUnitTest.dat";
    storeFile.DeleteFileLocal(filename);
    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);
    char data[5][21] = { "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbbbbb",
        "cccccccccccccccccccc",
        "dddddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeeee" };
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(0, storeFile.SaveItem(data[i], strlen(data[i]), NULL));
        PAL::sleep(10);// make sure in time differs
    }

    EXPECT_EQ(strlen(data[0]), storeFile.GetBufferSize());

    char *buf;
    size_t  size;

    storeFile.Close();
    EXPECT_EQ(0, storeFile.Open(filename, fileSize, blockSize));

    for (size_t i = 0; storeFile.PopNextItem(&buf, size, NULL) > 0; i++)
    {
        //EXPECT_STREQ(data[i + 1], buf);
        EXPECT_EQ(0, std::memcmp(buf, data[i + 1], size));
        storeFile.Free(&buf);
    }

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify the logic of FIFOFileStorage when reopening an
existing file with smaller file size specified.
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageSmallerFileUnitTest)
{
    size_t  size;
    char *buf = NULL;
    FIFOFileStorage  storeFile;
    char filename[] = "OfflineStorageSmallerFileUnitTest.dat";

    int blockCount = 10;
    int oldBlockSize = 128;
    int newBlockSize = 128;// 64;

    size_t  oldFileSize = (oldBlockSize + sizeof(PhysicalBlockInfo)) * blockCount;
    size_t  newFileSize = (newBlockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    storeFile.DeleteFileLocal(filename);

    EXPECT_EQ(0, storeFile.Open(filename, oldFileSize, oldBlockSize));

    char data[5][21] = { "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbb",
        "ccccccccccccccc",
        "dddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeee" };

    EXPECT_EQ(0, storeFile.SaveItem(data[0], strlen(data[0]), NULL));
    EXPECT_EQ(strlen(data[0]), storeFile.GetBufferSize());
    EXPECT_EQ(0, storeFile.SaveItem(data[1], strlen(data[1]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[2], strlen(data[2]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[3], strlen(data[3]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[4], strlen(data[4]), NULL));
    oldFileSize = (int)storeFile.GetFileSize();
    storeFile.Close();

    EXPECT_EQ(0, storeFile.Open(filename, newFileSize, newBlockSize));
    for (size_t i = 0; i < 5; i++)
    {
        EXPECT_EQ(strlen(data[i]), storeFile.GetBufferSize());
        EXPECT_GT(storeFile.PopNextItem(&buf, size, NULL), 0);
        // EXPECT_STREQ(buf, data[i]);
        EXPECT_EQ(0, std::memcmp(buf, data[i], size));
        storeFile.Free(&buf);
    }

    EXPECT_EQ(oldFileSize, storeFile.GetFileSize());

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify the logic of FIFOFileStorage when reopening an
existing file with smaller file size specified.
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageLargerFileUnitTest)
{
    size_t  size;
    char *buf = NULL;
    FIFOFileStorage  storeFile;
    char filename[] = "OfflineStorageLargerFileUnitTest.dat";

    int oldBlockCount = 64;
    //	int newBlockCount = 80;

    int oldBlockSize = 128;
    //	int newBlockSize = 64;

    size_t  oldFileSize = (oldBlockSize + sizeof(PhysicalBlockInfo)) * oldBlockCount;
    //	size_t  newFileSize = (newBlockSize + sizeof(PhysicalBlockInfo)) * newBlockCount;

    storeFile.DeleteFileLocal(filename);

    EXPECT_EQ(0, storeFile.Open(filename, oldFileSize, oldBlockSize));

    char data[5][21] = { "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbb",
        "ccccccccccccccc",
        "dddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeee" };

    EXPECT_EQ(0, storeFile.SaveItem(data[0], strlen(data[0]), NULL));
    EXPECT_EQ(strlen(data[0]), storeFile.GetBufferSize());
    EXPECT_EQ(0, storeFile.SaveItem(data[1], strlen(data[1]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[2], strlen(data[2]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[3], strlen(data[3]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[4], strlen(data[4]), NULL));
    oldFileSize = (int)storeFile.GetFileSize();
    storeFile.Close();

    EXPECT_EQ(0, storeFile.Open(filename, oldFileSize, oldBlockSize));
    for (size_t i = 0; i < 5; i++)
    {
        EXPECT_EQ(strlen(data[i]), storeFile.GetBufferSize());
        EXPECT_GT(storeFile.PopNextItem(&buf, size, NULL), 0);
        //EXPECT_STREQ(buf, data[i]);
        EXPECT_EQ(0, std::memcmp(buf, data[i], size));
        storeFile.Free(&buf);
    }
    EXPECT_EQ(oldFileSize, storeFile.GetFileSize());

    EXPECT_EQ(0, storeFile.SaveItem(data[0], strlen(data[0]), NULL));
    EXPECT_NE(oldFileSize, storeFile.GetFileSize());

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify the logic of FIFOFileStorage delay Creation Logic.
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageDelayCreationUnitTest)
{
    size_t  size;
    char *buf = NULL;
    FIFOFileStorage  storeFile;
    char filename[] = "OfflineStorageDelayCreationUnitTest.dat";

    int blockSize = 64;
    int blockCount = 10;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    storeFile.DeleteFileLocal(filename);

    EXPECT_EQ(0, storeFile.Open(filename, fileSize, blockSize));
    EXPECT_EQ(0ul, storeFile.GetBufferSize());
    EXPECT_EQ(0, storeFile.PopNextItem(&buf, size, NULL));
    EXPECT_EQ(sizeof(FileHeader), storeFile.GetFileSize());
    storeFile.Close();
    EXPECT_EQ(0ul, storeFile.GetFileSize());
    //	EXPECT_EQ(sizeof(FileHeader), StorageHelper::GetFileSize(filename));

    EXPECT_EQ(0, storeFile.Open(filename, fileSize, blockSize));
    char data[5][21] = { "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbb",
        "ccccccccccccccc",
        "dddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeee" };

    EXPECT_EQ(0, storeFile.SaveItem(data[0], strlen(data[0]), NULL));
    EXPECT_EQ(strlen(data[0]), storeFile.GetBufferSize());

    EXPECT_NE(sizeof(FileHeader), storeFile.GetFileSize());
    fileSize = (int)storeFile.GetFileSize();

    EXPECT_EQ(0, storeFile.SaveItem(data[1], strlen(data[1]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[2], strlen(data[2]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[3], strlen(data[3]), NULL));
    EXPECT_EQ(0, storeFile.SaveItem(data[4], strlen(data[4]), NULL));
    EXPECT_EQ(fileSize, storeFile.GetFileSize());

    for (size_t i = 0; i < 5; i++)
    {
        EXPECT_EQ(strlen(data[i]), storeFile.GetBufferSize());
        EXPECT_GT(storeFile.PopNextItem(&buf, size, NULL), 0);
        //EXPECT_STREQ(buf, data[i]);
        EXPECT_EQ(0, std::memcmp(buf, data[i], size));
        storeFile.Free(&buf);
    }

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

TEST_F(FIFOOfflineStorageTests, OfflineStorageInvalidBlockUnitTest)
{
    char *buf = NULL;
    size_t size;
    FIFOFileStorage  storeFile;
    char filename[] = "OfflineStorageInvalidBlockUnitTest.dat";
    int blockCount = 10;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    storeFile.DeleteFileLocal(filename);

    EXPECT_EQ(0, storeFile.Open(filename, fileSize, blockSize));
    EXPECT_EQ(0, storeFile.SaveItem(NULL, 0, NULL)); //adjust file size, create block structure
/*
    FriendStorage ft(&storeFile);
    vector<PhysicalBlockInfo>& blocks = ft.Blocks();
    ASSERT_EQ(blockCount, blocks.size());
    for (int i = 0; i < blockCount; ++i)
    {
        blocks[i].SizeInBytes = blockSize;
        blocks[i].Key.Time = PAL::getUtcSystemTimeMs();
    }

    //invalid blocks
    blocks[0].NextBlockId = -1;
    blocks[1].NextBlockId = blockCount + 1;
    blocks[2].PreviousBlockId = -1;
    blocks[3].PreviousBlockId = blockCount + 1;
    //loop without header
    blocks[4].NextBlockId = 5;
    blocks[4].PreviousBlockId = 5;
    blocks[5].NextBlockId = 4;
    blocks[5].PreviousBlockId = 4;
    //loop with a header
    blocks[6].NextBlockId = 7;
    blocks[7].PreviousBlockId = 6;
    blocks[7].NextBlockId = 8;
    blocks[8].PreviousBlockId = 7;
    blocks[8].NextBlockId = 7;

    ft.WriteFileInfo();
    storeFile.Close();

    EXPECT_EQ(0, storeFile.Open(filename, fileSize, blockSize));
    ft.Blocks();
    for (int i = 0; i < blockCount - 1; ++i)
    {
        EXPECT_EQ(0, blocks[i].SizeInBytes) << "Invalid block " << i << " cannot be detected" << endl;
    }
    EXPECT_EQ(blockSize, blocks[blockCount - 1].SizeInBytes);

    std::uint64_t oldSize = ft.FileHeaderRef().fileSize;
    ft.FileHeaderRef().fileSize = oldSize + 1000;
    EXPECT_EQ(RES_SUCC, ft.GenerateFile());
    ft.FileHeaderRef().fileSize = oldSize;
    EXPECT_EQ(RES_SUCC, ft.TruncateFile());
*/
    storeFile.Close();

    EXPECT_EQ(RES_SUCC, storeFile.Open(filename, fileSize, blockSize));
    //	EXPECT_EQ(blockSize, ft.Blocks()[blockCount - 1].SizeInBytes);

    EXPECT_EQ(0, storeFile.PopNextItem(&buf, size, NULL));
    storeFile.Free(&buf);
    storeFile.Close();

    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify Find First and Find N
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageFindFirstFindNextUnitTest)
{
    int ret;

    int blockCount = 10;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    char filename[] = "OfflineStorageFindFirstFindNextUnitTest.dat";

    // Delete the file before starting the test
    FIFOFileStorage  storeFile;
    storeFile.DeleteFileLocal(filename);

    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);

    char data[5][21] =
    {
        "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbbbbb",
        "cccccccccccccccccccc",
        "dddddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeeee"
    };

    // Save each item with a different priority
    for (int itemIndex = 0; itemIndex < 5; itemIndex++)
    {
        StorageItemKey fileItemInfo = {};
        fileItemInfo.Priority = itemIndex;

        // Insert the same item twice with the same priority
        for (int i = 0; i < 2; i++)
        {
            EXPECT_EQ(0, storeFile.SaveItem(data[itemIndex], strlen(data[itemIndex]), &fileItemInfo));
        }
    }

    // Go through each item to make sure that it's priority is correctly set
    FindItemInfo findItemInfo = {};
    bool beginCalled = false;

    for (int i = 0; i < 10; i++)
    {
        char* pBlockBuffer = NULL;
        size_t bytesRead = 0;

        if (beginCalled == false)
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindFirstItem(&findItemInfo));
            beginCalled = true;
        }
        else
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindNextItem(&findItemInfo));
        }

        int inputArrayIndex = i / 2;

        EXPECT_EQ(static_cast<uint32_t>(inputArrayIndex), findItemInfo.Key.Priority);
        EXPECT_EQ(strlen(data[inputArrayIndex]), findItemInfo.ItemSize);

        storeFile.ReadItem(findItemInfo, findItemInfo.ItemSize, &pBlockBuffer, &bytesRead);
        EXPECT_EQ(0, std::memcmp(pBlockBuffer, data[inputArrayIndex], findItemInfo.ItemSize));
        storeFile.Free(&pBlockBuffer);
    }

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify Find First and Find N
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageFindFirstFindNextWithStringKeyUnitTest)
{
    int ret;

    int blockCount = 10;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    char filename[] = "OfflineStorageFindFirstFindNextWithStringKeyUnitTest.dat";

    // Delete the file before starting the test
    FIFOFileStorage  storeFile;
    storeFile.DeleteFileLocal(filename);

    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);

    struct InputData
    {
        StorageItemKey Key;
        const char Data[32];
    };

    InputData inputSet[] =
    {
        // { Id, Priority, Time }, Data
        { { "Id1", 1, 100 } , "aaaaaaaaaaaaaaaaaaaa" },
        { { "Id2", 2, 200 } , "bbbbbbbbbbbbbbbbbbbb" },
        { { "Id3", 1, 300 } , "cccccccccccccccccccc" },
        { { "Id4", 100, 400 } , "dddddddddddddddddddd" },
        { { "Id5", 22, 500 } , "eeeeeeeeeeeeeeeeeeee" }
    };

    const size_t c_inputSize = sizeof(inputSet) / sizeof(inputSet[0]);

    // Save each item with a different priority
    for (int itemIndex = 0; itemIndex < c_inputSize; itemIndex++)
    {
        EXPECT_EQ(0, storeFile.SaveItem(inputSet[itemIndex].Data, strlen(inputSet[itemIndex].Data), &inputSet[itemIndex].Key));
    }

    FindItemInfo findItemInfo = {};
    bool beginCalled = false;

    for (int i = 0; i < c_inputSize; i++)
    {
        char* pBlockBuffer = NULL;
        size_t bytesRead = 0;

        if (beginCalled == false)
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindFirstItem(&findItemInfo));
            beginCalled = true;
        }
        else
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindNextItem(&findItemInfo));
        }

        EXPECT_EQ(inputSet[i].Key.Time, findItemInfo.Key.Time);
        EXPECT_EQ(inputSet[i].Key.Priority, findItemInfo.Key.Priority);
        EXPECT_EQ(strcmp(inputSet[i].Key.Id, findItemInfo.Key.Id), 0);
        EXPECT_EQ(strlen(inputSet[i].Data), findItemInfo.ItemSize);

        storeFile.ReadItem(findItemInfo, findItemInfo.ItemSize, &pBlockBuffer, &bytesRead);
        EXPECT_EQ(0, std::memcmp(pBlockBuffer, inputSet[i].Data, findItemInfo.ItemSize));
        storeFile.Free(&pBlockBuffer);
    }

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify Find First and Find N
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageRemoveItemAndMoveNextUnitTest)
{
    int ret;

    int blockCount = 10;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;

    char filename[] = "OfflineStorageRemoveItemAndMoveNextUnitTest.dat";

    // Delete the file before starting the test
    FIFOFileStorage  storeFile;
    storeFile.DeleteFileLocal(filename);

    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);

    char data[5][21] =
    {
        "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbbbbb",
        "cccccccccccccccccccc",
        "dddddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeeee"
    };

    // Save each item with a different priority
    for (int itemIndex = 0; itemIndex < 5; itemIndex++)
    {
        //std::string temp = std::to_string(itemIndex);
        //char temp = itemIndex;
        StorageItemKey fileItemInfo("ID", itemIndex, 0);


        EXPECT_EQ(0, storeFile.SaveItem(data[itemIndex], strlen(data[itemIndex]), &fileItemInfo));
    }

    // Go through each item to make sure that it's priority is correctly set
    FindItemInfo findItemInfo = {};
    bool beginCalled = false;

    for (int i = 0; i < 5; i++)
    {
        if (beginCalled == false)
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindFirstItem(&findItemInfo));
            beginCalled = true;
        }

        int inputArrayIndex = i;

        EXPECT_EQ(static_cast<uint32_t>(inputArrayIndex), findItemInfo.Key.Priority);
        EXPECT_EQ(strlen(data[inputArrayIndex]), findItemInfo.ItemSize);

        char* pBlockBuffer = NULL;
        size_t bytesRead = 0;

        bool hasMoreItems = false;
        DATARV_ERROR errorCode = storeFile.RemoveItemAndMoveNext(&findItemInfo, &hasMoreItems, &pBlockBuffer, &bytesRead);
        EXPECT_EQ(DATARV_ERROR_OK, errorCode);

        // Except for the last iteration, we should have more items
        if (i != 4)
        {
            EXPECT_EQ(true, hasMoreItems);
        }

        EXPECT_EQ(0, std::memcmp(pBlockBuffer, data[inputArrayIndex], findItemInfo.ItemSize));
        storeFile.Free(&pBlockBuffer);
    }

    // Doing a find first should return error
    FindItemInfo findItemInfo2 = {};
    EXPECT_EQ(DATARV_ERROR_FILE_NOMOREITEMS, storeFile.FindFirstItem(&findItemInfo2));

    // Doing PopNextItem should return error
    char* pBlockBuffer = NULL;
    size_t bytesRead = 0;
    EXPECT_EQ(0, storeFile.PopNextItem(&pBlockBuffer, bytesRead, NULL));

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief This is a unit test to verify that we can remove items by time filter
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageRemoveItemsByTimeUnitTest)
{
    //	int blockCount = 10;
    //	int blockSize = 64;
        //size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;
    char filename[] = "OfflineStorageRemoveItemsByTimeUnitTest.dat";

    // Delete the file before starting the test
    FIFOFileStorage  storeFile;
    storeFile.DeleteFileLocal(filename);

    Microsoft::Applications::Events::IStorage* pOfflineStorage = new FIFOFileStorage();
    EXPECT_TRUE(pOfflineStorage != NULL);

    EXPECT_TRUE(pOfflineStorage->Open(filename, 1000 * (32 * 1024)) == 0);

    char data[5][21] =
    {
        "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbbbbb",
        "cccccccccccccccccccc",
        "dddddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeeee"
    };

    int totalItems = 10;
    int midIndex = totalItems / 2;

    // Save each item with a different priority
    for (int itemIndex = 0; itemIndex < totalItems; itemIndex++)
    {
        StorageItemKey fileItemInfo("ID", itemIndex, (itemIndex + 1) * 10000);

        EXPECT_EQ(0, pOfflineStorage->SaveItem(data[itemIndex % 5], strlen(data[itemIndex % 5]), &fileItemInfo));
    }

    // Check the count

    FindItemInfo findItemInfo = {};

    for (int i = 0; i < midIndex; i++)
    {
        char* pBlockBuffer = NULL;
        size_t bytesRead = 0;

        EXPECT_EQ(DATARV_ERROR_OK, pOfflineStorage->FindFirstItem(&findItemInfo));
        int inputArrayIndex = i % midIndex;

        EXPECT_EQ(inputArrayIndex, static_cast<int>(findItemInfo.Key.Priority));
        EXPECT_EQ(strlen(data[inputArrayIndex]), findItemInfo.ItemSize);

        bool hasMoreItems = false;
        DATARV_ERROR errorCode = pOfflineStorage->RemoveItemAndMoveNext(&findItemInfo, &hasMoreItems, &pBlockBuffer, &bytesRead);
        EXPECT_EQ(DATARV_ERROR_OK, errorCode);

        EXPECT_EQ(0, std::memcmp(pBlockBuffer, data[inputArrayIndex], findItemInfo.ItemSize));
        delete[] pBlockBuffer;
    }

    EXPECT_EQ(DATARV_ERROR_OK, pOfflineStorage->FindNextItem(&findItemInfo));


    for (int i = midIndex; i < totalItems; i++)
    {
        char* pBlockBuffer = NULL;
        size_t bytesRead = 0;

        EXPECT_EQ(DATARV_ERROR_OK, pOfflineStorage->FindFirstItem(&findItemInfo));

        int inputArrayIndex = i % 5;

        EXPECT_EQ(static_cast<uint32_t>(i), findItemInfo.Key.Priority);
        EXPECT_EQ(strlen(data[inputArrayIndex]), findItemInfo.ItemSize);

        bool hasMoreItems = false;
        DATARV_ERROR errorCode = pOfflineStorage->RemoveItemAndMoveNext(&findItemInfo, &hasMoreItems, &pBlockBuffer, &bytesRead);

        EXPECT_EQ(DATARV_ERROR_OK, errorCode);

        EXPECT_EQ(0, std::memcmp(pBlockBuffer, data[inputArrayIndex], findItemInfo.ItemSize));
        delete[] pBlockBuffer;
    }

    EXPECT_EQ(DATARV_ERROR_FILE_NOMOREITEMS, pOfflineStorage->FindNextItem(&findItemInfo));

    // Now delete the only item that's left
    EXPECT_EQ(DATARV_ERROR_FILE_NOMOREITEMS, pOfflineStorage->FindFirstItem(&findItemInfo));

    delete pOfflineStorage;
    storeFile.DeleteFileLocal(filename);
}


/** \brief Ensures that items with oldest "in time" gets retrieved fist.
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageRetrievesItemsInChronologicalOrderUnitTest)
{
    int ret;

    int blockCount = 10;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;
    char filename[] = "OfflineStorageRetrievesItemsInChronologicalOrderUnitTest.dat";

    // Delete the file before starting the test
    FIFOFileStorage  storeFile;
    storeFile.DeleteFileLocal(filename);

    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);

    char data[5][21] =
    {
        "aaaaaaaaaaaaaaaaaaaa",
        "bbbbbbbbbbbbbbbbbbbb",
        "cccccccccccccccccccc",
        "dddddddddddddddddddd",
        "eeeeeeeeeeeeeeeeeeee"
    };

    // Save items in newest to oldest order
    for (int itemIndex = 0; itemIndex < 5; itemIndex++)
    {
        StorageItemKey fileItemInfo = {};

        fileItemInfo.Time = 10000 - itemIndex;
        fileItemInfo.Priority = itemIndex;
        EXPECT_EQ(0, storeFile.SaveItem(data[itemIndex], strlen(data[itemIndex]), &fileItemInfo));
    }

    FindItemInfo findItemInfo = {};
    bool beginCalled = false;

    // Items should be given to us in reverse order
    for (int i = 4; i >= 0; i--)
    {
        char* pBlockBuffer = NULL;
        size_t bytesRead = 0;

        if (beginCalled == false)
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindFirstItem(&findItemInfo));
            beginCalled = true;
        }
        else
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindNextItem(&findItemInfo));
        }

        int inputArrayIndex = i;

        EXPECT_EQ(static_cast<uint32_t>(inputArrayIndex), findItemInfo.Key.Priority);
        EXPECT_EQ(strlen(data[inputArrayIndex]), findItemInfo.ItemSize);

        storeFile.ReadItem(findItemInfo, findItemInfo.ItemSize, &pBlockBuffer, &bytesRead);
        EXPECT_EQ(0, std::memcmp(pBlockBuffer, data[inputArrayIndex], findItemInfo.ItemSize));
        storeFile.Free(&pBlockBuffer);
    }

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}

/** \brief Ensures that items with oldest "in time" gets overwritten when the storage is full.
*/
TEST_F(FIFOOfflineStorageTests, OfflineStorageOverwriteOldestItemUnitTest)
{
    int ret;

    int blockCount = 100;
    int blockSize = 64;
    size_t  fileSize = (blockSize + sizeof(PhysicalBlockInfo)) * blockCount;
    char filename[] = "OfflineStorageOverwriteOldestItemUnitTest.dat";

    // Delete the file before starting the test
    FIFOFileStorage  storeFile;
    storeFile.DeleteFileLocal(filename);

    ret = storeFile.Open(filename, fileSize, blockSize);
    EXPECT_EQ(0, ret);

    char data[32];

    size_t writeCount = blockCount + 100;

    // Save items in newest to oldest order
    for (size_t itemIndex = 1; itemIndex <= writeCount; itemIndex++)
    {
        sprintf_s(data, "Data Item %d", static_cast<int>(itemIndex));
        StorageItemKey fileItemInfo = {};

        fileItemInfo.Time = itemIndex;
        fileItemInfo.Priority = 1;
        EXPECT_EQ(0, storeFile.SaveItem(data, strlen(data), &fileItemInfo));
    }

    FindItemInfo findItemInfo = {};
    bool beginCalled = false;

    // Irrespective of how many items have been written, we should only have 
    // blockCount items which are the most recent items
    for (int i = 1; i <= blockCount; i++)
    {
        char* pBlockBuffer = NULL;
        size_t bytesRead = 0;

        if (beginCalled == false)
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindFirstItem(&findItemInfo));
            beginCalled = true;
        }
        else
        {
            EXPECT_EQ(DATARV_ERROR_OK, storeFile.FindNextItem(&findItemInfo));
        }

        int itemIndex = static_cast<int>(writeCount - (blockCount - i));
        printf("Item Index %d", itemIndex);

        sprintf_s(data, "Data Item %d", itemIndex);

        EXPECT_EQ(1u, findItemInfo.Key.Priority);
        EXPECT_EQ(strlen(data), findItemInfo.ItemSize);

        storeFile.ReadItem(findItemInfo, findItemInfo.ItemSize, &pBlockBuffer, &bytesRead);
        EXPECT_EQ(0, std::memcmp(pBlockBuffer, data, findItemInfo.ItemSize));
        storeFile.Free(&pBlockBuffer);
    }

    storeFile.Close();
    storeFile.DeleteFileLocal(filename);
}
