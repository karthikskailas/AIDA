#pragma once
#include <string>

const size_t BLOCK_SIZE = 4096; // 4KB

class StorageEngine {
public:
    static void createStorageFile(const std::string& filename, size_t sizeInMB);
    static bool openStorageFile(const std::string& filename);
    static void closeStorageFile();
    static void writeBlock(int blockID, const char* data);
    static void readBlock(int blockID, char* buffer);
};
