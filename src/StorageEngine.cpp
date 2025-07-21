#include "StorageEngine.h"
#include <fstream>
#include <iostream>
#include <algorithm>

std::fstream file;

// ————————————— File & Block Management —————————————

void StorageEngine::createStorageFile(const std::string &filename, size_t sizeInMB)
{
    std::ofstream f(filename, std::ios::binary);
    if (!f)
    {
        std::cerr << "Error creating file!\n";
        return;
    }
    f.seekp(sizeInMB * 1024 * 1024 - 1);
    f.write("", 1);
    f.close();
    std::cout << "Created storage file: " << filename << " (" << sizeInMB << " MB)\n";

    // initialize metadata in block 0
    Metadata meta;
    updateMetadata(meta);
}

bool StorageEngine::openStorageFile(const std::string &filename)
{
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }
    std::cout << "Opened storage file: " << filename << "\n";
    return true;
}

void StorageEngine::closeStorageFile()
{
    if (file.is_open())
    {
        file.close();
        std::cout << "Closed storage file.\n";
    }
}

void StorageEngine::writeBlock(int blockID, const char *data)
{
    if (!file.is_open())
        return;
    file.seekp(blockID * BLOCK_SIZE);
    file.write(data, BLOCK_SIZE);
    file.flush();
    std::cout << "Wrote to block #" << blockID << "\n";
}

void StorageEngine::readBlock(int blockID, char *buffer)
{
    if (!file.is_open())
        return;
    file.seekg(blockID * BLOCK_SIZE);
    file.read(buffer, BLOCK_SIZE);
    std::cout << "Read from block #" << blockID << "\n";
}

// ————————————— Metadata Management —————————————

void StorageEngine::loadMetadata(Metadata &meta)
{
    char buf[BLOCK_SIZE] = {};
    readBlock(0, buf);

    size_t off = 0;
    memcpy(&meta.totalPages, buf + off, sizeof(meta.totalPages));
    off += sizeof(meta.totalPages);
    memcpy(&meta.nextPageID, buf + off, sizeof(meta.nextPageID));
    off += sizeof(meta.nextPageID);
    memcpy(meta.freePagesBitmap, buf + off, sizeof(meta.freePagesBitmap));

    std::cout << "[DEBUG] Loaded Metadata: totalPages=" << meta.totalPages
              << ", nextPageID=" << meta.nextPageID << "\n";
}

void StorageEngine::updateMetadata(const Metadata &meta)
{
    char buf[BLOCK_SIZE] = {};
    size_t off = 0;
    memcpy(buf + off, &meta.totalPages, sizeof(meta.totalPages));
    off += sizeof(meta.totalPages);
    memcpy(buf + off, &meta.nextPageID, sizeof(meta.nextPageID));
    off += sizeof(meta.nextPageID);
    memcpy(buf + off, meta.freePagesBitmap, sizeof(meta.freePagesBitmap));

    writeBlock(0, buf);
    std::cout << "[DEBUG] Updated Metadata.\n";
}

int StorageEngine::allocatePage()
{
    Metadata meta;
    loadMetadata(meta);

    for (uint32_t i = 1; i < 1024; ++i)
    {
        int byte = i / 8, bit = i % 8;
        if (!(meta.freePagesBitmap[byte] & (1 << bit)))
        {
            meta.freePagesBitmap[byte] |= (1 << bit);
            meta.totalPages = std::max(meta.totalPages, i + 1u);
            meta.nextPageID = i + 1;
            updateMetadata(meta);
            std::cout << "[DEBUG] Allocated Page #" << i << "\n";
            return i;
        }
    }
    std::cerr << "No free pages available!\n";
    return -1;
}

// ————————————— Page I/O —————————————

void StorageEngine::writePage(const Page &page)
{
    char buf[BLOCK_SIZE] = {};
    size_t off = 0;
    memcpy(buf + off, &page.pageID, sizeof(page.pageID));
    off += sizeof(page.pageID);
    memcpy(buf + off, &page.recordCount, sizeof(page.recordCount));
    off += sizeof(page.recordCount);
    memcpy(buf + off, &page.freeSpaceOffset, sizeof(page.freeSpaceOffset));
    off += sizeof(page.freeSpaceOffset);
    memcpy(buf + off, page.data, sizeof(page.data));

    std::cout << "[DEBUG] Writing Page #" << page.pageID << " to block #" << page.pageID << "\n";
    writeBlock(page.pageID, buf);
}

void StorageEngine::readPage(int pageID, Page &page)
{
    char buf[BLOCK_SIZE] = {};
    readBlock(pageID, buf);

    // detect empty block
    bool empty = true;
    for (size_t i = 0; i < BLOCK_SIZE; ++i)
    {
        if (buf[i] != 0)
        {
            empty = false;
            break;
        }
    }
    if (empty)
    {
        std::cout << "[DEBUG] Block #" << pageID << " is empty. Initializing new page.\n";
        page.pageID = pageID;
        page.recordCount = 0;
        page.freeSpaceOffset = 0;
        memset(page.data, 0, sizeof(page.data));
        return;
    }

    size_t off = 0;
    memcpy(&page.pageID, buf + off, sizeof(page.pageID));
    off += sizeof(page.pageID);
    memcpy(&page.recordCount, buf + off, sizeof(page.recordCount));
    off += sizeof(page.recordCount);
    memcpy(&page.freeSpaceOffset, buf + off, sizeof(page.freeSpaceOffset));
    off += sizeof(page.freeSpaceOffset);
    memcpy(page.data, buf + off, sizeof(page.data));

    std::cout << "[DEBUG] Page Metadata - pageID:" << page.pageID
              << ", recordCount:" << page.recordCount
              << ", freeSpaceOffset:" << page.freeSpaceOffset << "\n";
}

// ————————————— CRUD Operations —————————————

bool StorageEngine::insertRecord(int pageID, const char *record, size_t size)
{
    if (size > MAX_RECORD_SIZE)
    {
        std::cerr << "Record too large!\n";
        return false;
    }

    Page page(pageID);
    page.pageID = pageID;
    readPage(pageID, page);

    size_t recSize = sizeof(RecordHeader) + size;
    if (page.freeSpaceOffset + recSize > sizeof(page.data))
    {
        std::cout << "[DEBUG] Page #" << pageID << " full. Allocating new page.\n";
        pageID = allocatePage();
        if (pageID < 0)
            return false;
        page = Page(pageID);
    }

    RecordHeader hdr = {false, static_cast<uint16_t>(size)};
    memcpy(page.data + page.freeSpaceOffset, &hdr, sizeof(hdr));
    memcpy(page.data + page.freeSpaceOffset + sizeof(hdr), record, size);
    page.freeSpaceOffset += recSize;
    page.recordCount++;

    writePage(page);
    std::cout << "Inserted record into page #" << pageID << "\n";
    return true;
}

bool StorageEngine::readRecord(int pageID, int recordIndex, char *buffer, size_t bufferSize)
{
    Page page(pageID);
    readPage(pageID, page);

    size_t off = 0;
    int idx = 0;
    while (off < page.freeSpaceOffset)
    {
        RecordHeader hdr;
        memcpy(&hdr, page.data + off, sizeof(hdr));

        if (idx == recordIndex)
        {
            if (hdr.deleted)
            {
                std::cerr << "Record #" << recordIndex << " has been deleted.\n";
                return false;
            }
            size_t copyLen = std::min(bufferSize - 1, static_cast<size_t>(hdr.length));
            memcpy(buffer, page.data + off + sizeof(hdr), copyLen);
            buffer[copyLen] = '\0';
            std::cout << "Read record #" << recordIndex << " from page #" << pageID << "\n";
            return true;
        }
        off += sizeof(hdr) + hdr.length;
        idx++;
    }

    std::cerr << "Record index out of range.\n";
    return false;
}

bool StorageEngine::deleteRecord(int pageID, int recordIndex)
{
    Page page(pageID);
    readPage(pageID, page);

    size_t off = 0;
    int idx = 0;
    while (off < page.freeSpaceOffset)
    {
        RecordHeader hdr;
        memcpy(&hdr, page.data + off, sizeof(hdr));

        if (idx == recordIndex)
        {
            if (hdr.deleted)
            {
                std::cerr << "Record #" << recordIndex << " is already deleted.\n";
                return false;
            }
            hdr.deleted = true;
            memcpy(page.data + off, &hdr, sizeof(hdr));
            writePage(page);
            std::cout << "Deleted record #" << recordIndex << " from page #" << pageID << "\n";
            return true;
        }
        off += sizeof(hdr) + hdr.length;
        idx++;
    }

    std::cerr << "Record index out of range.\n";
    return false;
}

bool StorageEngine::updateRecord(int pageID, int recordIndex, const char *newRecord, size_t size)
{
    Page page(pageID);
    readPage(pageID, page);

    size_t off = 0;
    int idx = 0;
    while (off < page.freeSpaceOffset)
    {
        RecordHeader hdr;
        memcpy(&hdr, page.data + off, sizeof(hdr));

        if (idx == recordIndex)
        {
            if (hdr.deleted)
            {
                std::cerr << "Cannot update a deleted record.\n";
                return false;
            }
            if (size <= hdr.length)
            {
                memcpy(page.data + off + sizeof(hdr), newRecord, size);
                writePage(page);
                std::cout << "Updated record #" << recordIndex << " in place.\n";
            }
            else
            {
                hdr.deleted = true;
                memcpy(page.data + off, &hdr, sizeof(hdr));
                insertRecord(pageID, newRecord, size);
                std::cout << "Record too big; inserted new record instead.\n";
            }
            return true;
        }
        off += sizeof(hdr) + hdr.length;
        idx++;
    }

    std::cerr << "Record not found.\n";
    return false;
}
