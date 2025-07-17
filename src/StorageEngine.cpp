#include "StorageEngine.h"
#include <fstream>
#include <iostream>

std::fstream file;

void StorageEngine::createStorageFile(const std::string& filename, size_t sizeInMB) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
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

bool StorageEngine::openStorageFile(const std::string& filename) {
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    std::cout << "Opened storage file: " << filename << std::endl;
    return true;
}

void StorageEngine::closeStorageFile() {
    if (file.is_open()) {
        file.close();
        std::cout << "Closed storage file." << std::endl;
    }
}

void StorageEngine::writeBlock(int blockID, const char* data) {
    if (!file.is_open()) return;
    file.seekp(blockID * BLOCK_SIZE);
    file.write(data, BLOCK_SIZE);
    file.flush();
    std::cout << "Wrote to block #" << blockID << std::endl;
}

void StorageEngine::readBlock(int blockID, char* buffer) {
    if (!file.is_open()) return;
    file.seekg(blockID * BLOCK_SIZE);
    file.read(buffer, BLOCK_SIZE);
    std::cout << "Read from block #" << blockID << std::endl;
}
