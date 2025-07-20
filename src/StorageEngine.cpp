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
    std::cout << "Created storage file: " << filename
              << " (" << sizeInMB << " MB)" << std::endl;
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
    writeBlock(page.pageID, reinterpret_cast<const char *>(&page));
}

void StorageEngine::readPage(int pageID, Page &page)
{
    readBlock(pageID, reinterpret_cast<char *>(&page));
}


bool StorageEngine::insertRecord(int pageID, const char *record, size_t size)
{
    if (size > MAX_RECORD_SIZE)
    {
        std::cerr << "Record too large!" << std::endl;
        return false;
    }

    Page page(pageID);
    readPage(pageID, page);

    // Force set pageID (fix for empty pages)
    page.pageID = pageID;

    if (page.freeSpaceOffset + size > sizeof(page.data))
    {
        std::cerr << "No space left in page #" << pageID << std::endl;
        return false;
    }

    memcpy(page.data + page.freeSpaceOffset, record, size);
    page.freeSpaceOffset += size;
    page.recordCount++;
    std::cout << "[DEBUG] recordCount=" << page.recordCount << std::endl;

    writePage(page);
    std::cout << "Inserted record into page #" << pageID << std::endl;
    return true;
}


bool StorageEngine::readRecord(int pageID, int recordIndex, char *buffer, size_t bufferSize)
{
    Page page(pageID); // Initialize with pageID
    readPage(pageID, page);

    if (recordIndex < 0 || recordIndex >= page.recordCount)
    {
        std::cerr << "Invalid record index!" << std::endl;
        return false;
    }

    size_t offset = 0;
    for (int i = 0; i < recordIndex; ++i)
    {
        offset += strlen(page.data + offset) + 1;
    }

    strncpy(buffer, page.data + offset, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';

    std::cout << "Read record #" << recordIndex << " from page #" << pageID << std::endl;
    return true;
}
