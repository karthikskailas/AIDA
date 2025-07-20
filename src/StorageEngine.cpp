#include "StorageEngine.h"
#include <fstream>
#include <iostream>
#include <cstring>

std::fstream file;

void StorageEngine::createStorageFile(const std::string &filename, size_t sizeInMB)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error creating file!" << std::endl;
        return;
    }
    size_t totalSize = sizeInMB * 1024 * 1024;
    file.seekp(totalSize - 1);
    file.write("", 1);
    file.close();
    std::cout << "Created storage file: " << filename << " (" << sizeInMB << " MB)" << std::endl;
}

bool StorageEngine::openStorageFile(const std::string &filename)
{
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    std::cout << "Opened storage file: " << filename << std::endl;
    return true;
}

void StorageEngine::closeStorageFile()
{
    if (file.is_open())
    {
        file.close();
        std::cout << "Closed storage file." << std::endl;
    }
}

void StorageEngine::writeBlock(int blockID, const char *data)
{
    if (!file.is_open())
        return;
    file.seekp(blockID * BLOCK_SIZE);
    file.write(data, BLOCK_SIZE);
    file.flush();
    std::cout << "Wrote to block #" << blockID << std::endl;
}

void StorageEngine::readBlock(int blockID, char *buffer)
{
    if (!file.is_open())
        return;
    file.seekg(blockID * BLOCK_SIZE);
    file.read(buffer, BLOCK_SIZE);
    std::cout << "Read from block #" << blockID << std::endl;
}

void StorageEngine::writePage(const Page &page)
{
    char buffer[BLOCK_SIZE] = {};
    size_t offset = 0;

    memcpy(buffer + offset, &page.pageID, sizeof(page.pageID));
    offset += sizeof(page.pageID);
    memcpy(buffer + offset, &page.recordCount, sizeof(page.recordCount));
    offset += sizeof(page.recordCount);
    memcpy(buffer + offset, &page.freeSpaceOffset, sizeof(page.freeSpaceOffset));
    offset += sizeof(page.freeSpaceOffset);
    memcpy(buffer + offset, page.data, sizeof(page.data));

    std::cout << "[DEBUG] Writing Page #" << page.pageID << " to block #" << page.pageID << std::endl;
    writeBlock(page.pageID, buffer);
}

void StorageEngine::readPage(int pageID, Page &page)
{
    char buffer[BLOCK_SIZE] = {};
    readBlock(pageID, buffer);

    // Check if block is empty (all zeros)
    bool isEmpty = true;
    for (size_t i = 0; i < BLOCK_SIZE; ++i)
    {
        if (buffer[i] != 0)
        {
            isEmpty = false;
            break;
        }
    }

    if (isEmpty)
    {
        std::cout << "[DEBUG] Block #" << pageID << " is empty. Initializing new page." << std::endl;
        page.pageID = pageID;
        page.recordCount = 0;
        page.freeSpaceOffset = 0;
        memset(page.data, 0, sizeof(page.data));
        return;
    }

    size_t offset = 0;
    memcpy(&page.pageID, buffer + offset, sizeof(page.pageID));
    offset += sizeof(page.pageID);
    memcpy(&page.recordCount, buffer + offset, sizeof(page.recordCount));
    offset += sizeof(page.recordCount);
    memcpy(&page.freeSpaceOffset, buffer + offset, sizeof(page.freeSpaceOffset));
    offset += sizeof(page.freeSpaceOffset);

    memcpy(page.data, buffer + offset, sizeof(page.data));

    std::cout << "[DEBUG] Page Metadata - pageID: " << page.pageID
              << ", recordCount: " << page.recordCount
              << ", freeSpaceOffset: " << page.freeSpaceOffset << std::endl;
}

bool StorageEngine::insertRecord(int pageID, const char *record, size_t size)
{
    if (size > MAX_RECORD_SIZE)
    {
        std::cerr << "Record too large!" << std::endl;
        return false;
    }

    Page page(pageID);
    page.pageID = pageID;

    readPage(pageID, page);

    size_t totalSize = sizeof(RecordHeader) + size;
    if (page.freeSpaceOffset + totalSize > sizeof(page.data))
    {
        std::cerr << "No space left in page #" << pageID << std::endl;
        return false;
    }

    RecordHeader header = {false, static_cast<uint16_t>(size)};
    memcpy(page.data + page.freeSpaceOffset, &header, sizeof(header));
    memcpy(page.data + page.freeSpaceOffset + sizeof(header), record, size);

    page.freeSpaceOffset += totalSize;
    page.recordCount++;

    writePage(page);
    std::cout << "Inserted record into page #" << pageID << std::endl;
    return true;
}

bool StorageEngine::readRecord(int pageID, int recordIndex, char *buffer, size_t bufferSize)
{
    Page page(pageID);
    readPage(pageID, page);

    size_t offset = 0;
    int currentIndex = 0;

    while (offset < page.freeSpaceOffset)
    {
        RecordHeader header;
        memcpy(&header, page.data + offset, sizeof(header));

        if (currentIndex == recordIndex)
        {
            if (header.deleted)
            {
                std::cerr << "Record #" << recordIndex << " has been deleted." << std::endl;
                return false;
            }

            size_t copySize = std::min(bufferSize - 1, static_cast<size_t>(header.length));
            memcpy(buffer, page.data + offset + sizeof(header), copySize);
            buffer[copySize] = '\0';
            std::cout << "Read record #" << recordIndex << " from page #" << pageID << std::endl;
            return true;
        }

        offset += sizeof(header) + header.length;
        currentIndex++;
    }

    std::cerr << "Record index out of range." << std::endl;
    return false;
}

bool StorageEngine::deleteRecord(int pageID, int recordIndex)
{
    Page page(pageID);
    readPage(pageID, page);

    size_t offset = 0;
    int currentIndex = 0;

    while (offset < page.freeSpaceOffset)
    {
        RecordHeader header;
        memcpy(&header, page.data + offset, sizeof(header));

        if (currentIndex == recordIndex)
        {
            if (header.deleted)
            {
                std::cerr << "Record #" << recordIndex << " is already deleted." << std::endl;
                return false;
            }

            header.deleted = true;
            memcpy(page.data + offset, &header, sizeof(header));
            writePage(page);
            std::cout << "Deleted record #" << recordIndex << " from page #" << pageID << std::endl;
            return true;
        }

        offset += sizeof(header) + header.length;
        currentIndex++;
    }

    std::cerr << "Record index out of range." << std::endl;
    return false;
}

bool StorageEngine::updateRecord(int pageID, int recordIndex, const char *newRecord, size_t size)
{
    Page page(pageID);
    readPage(pageID, page);

    size_t offset = 0;
    int currentIndex = 0;

    while (offset < page.freeSpaceOffset)
    {
        RecordHeader header;
        memcpy(&header, page.data + offset, sizeof(header));

        if (currentIndex == recordIndex)
        {
            if (header.deleted)
            {
                std::cerr << "Cannot update a deleted record." << std::endl;
                return false;
            }

            if (size <= header.length)
            {
                memcpy(page.data + offset + sizeof(header), newRecord, size);
                writePage(page);
                std::cout << "Updated record #" << recordIndex << " in place." << std::endl;
            }
            else
            {
                header.deleted = true;
                memcpy(page.data + offset, &header, sizeof(header));
                insertRecord(pageID, newRecord, size);
                std::cout << "Record too big for in-place update. Inserted as new record." << std::endl;
            }
            return true;
        }

        offset += sizeof(header) + header.length;
        currentIndex++;
    }

    std::cerr << "Record index out of range." << std::endl;
    return false;
}
