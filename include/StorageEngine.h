#pragma once
#include <string>

const size_t BLOCK_SIZE = 4096;     // 4KB block size
const size_t MAX_RECORD_SIZE = 256; // Max size of a single record

struct RecordHeader
{
    bool deleted;    // 1 byte
    uint16_t length; // 2 bytes
};

struct Page
{
    int pageID;
    int recordCount;
    int freeSpaceOffset;
    char data[BLOCK_SIZE - sizeof(int) * 3];

    Page(int id) : pageID(id), recordCount(0), freeSpaceOffset(0) {}
};

class StorageEngine
{
public:
    static void createStorageFile(const std::string &filename, size_t sizeInMB);
    static bool openStorageFile(const std::string &filename);
    static void closeStorageFile();
    static void writeBlock(int blockID, const char *data);
    static void readBlock(int blockID, char *buffer);
    static void writePage(const Page &page);
    static void readPage(int pageID, Page &page);

    static bool insertRecord(int pageID, const char *record, size_t size);
    static bool readRecord(int pageID, int recordIndex, char *buffer, size_t bufferSize);
    static bool deleteRecord(int pageID, int recordIndex);
    static bool updateRecord(int pageID, int recordIndex, const char *newRecord, size_t size);
};
