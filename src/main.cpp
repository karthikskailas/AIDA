#include <iostream>
#include "StorageEngine.h"
#include <cstring>

int main() {
    std::cout << "AIDA Storage Engine Started 🚀" << std::endl;

    StorageEngine::createStorageFile("mydb.aida", 10);
    if (StorageEngine::openStorageFile("mydb.aida")) {
        const char* record1 = "Hello, AIDA Record 1";
        const char* record2 = "Hello, AIDA Record 2";

        StorageEngine::insertRecord(1, record1, strlen(record1) + 1);
        StorageEngine::insertRecord(1, record2, strlen(record2) + 1);

        char buffer[MAX_RECORD_SIZE];
        if (StorageEngine::readRecord(1, 0, buffer, sizeof(buffer))) {
            std::cout << "Record 0: " << buffer << std::endl;
        }
        if (StorageEngine::readRecord(1, 1, buffer, sizeof(buffer))) {
            std::cout << "Record 1: " << buffer << std::endl;
        }

        StorageEngine::closeStorageFile();
    }
    return 0;
}
