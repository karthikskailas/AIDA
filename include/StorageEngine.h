#pragma once
#include <string>
#include <cstdint>
#include <cstring>

const size_t BLOCK_SIZE = 4096;     // 4KB block size
const size_t MAX_RECORD_SIZE = 256; // Max size of a single record

// Per-record header (1 byte deleted flag + 2 byte length)
struct RecordHeader
{
    bool deleted;
    uint16_t length;
};

// One database page (fits exactly in BLOCK_SIZE)
struct Page
{
    int pageID;
    int recordCount;
    int freeSpaceOffset;
    char data[BLOCK_SIZE - sizeof(int) * 3];

    Page(int id) : pageID(id), recordCount(0), freeSpaceOffset(0) {}
};

// Metadata stored in block 0
struct Metadata
{
    uint32_t totalPages;
    uint32_t nextPageID;
    uint8_t freePagesBitmap[128]; // supports up to 1024 pages

    Metadata() : totalPages(1), nextPageID(1)
    {
        memset(freePagesBitmap, 0, sizeof(freePagesBitmap));
    }
};

class StorageEngine
{
public:
    // File management
    static void createStorageFile(const std::string &filename, size_t sizeInMB);
    static bool openStorageFile(const std::string &filename);
    static void closeStorageFile();

    // Low-level block I/O
    static void writeBlock(int blockID, const char *data);
    static void readBlock(int blockID, char *buffer);

    // Metadata page (block 0)
    static void loadMetadata(Metadata &meta);
    static void updateMetadata(const Metadata &meta);
    static int allocatePage();

    // Page I/O (blocks 1…N)
    static void writePage(const Page &page);
    static void readPage(int pageID, Page &page);

    // CRUD operations
    static bool insertRecord(int pageID, const char *record, size_t size);
    static bool readRecord(int pageID, int recordIndex, char *buffer, size_t bufferSize);
    static bool deleteRecord(int pageID, int recordIndex);
    static bool updateRecord(int pageID, int recordIndex, const char *newRecord, size_t size);
};
