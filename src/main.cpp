#include <iostream>
#include "StorageEngine.h"
#include <cstring>

int main()
{
    std::cout << "AIDA Storage Engine Started 🚀" << std::endl;

    // Create or open storage file
    StorageEngine::createStorageFile("mydb.aida", 10);
    if (!StorageEngine::openStorageFile("mydb.aida"))
        return 1;

    // Insert records
    const char *record1 = "Hello, AIDA Record 1";
    const char *record2 = "Hello, AIDA Record 2";
    const char *record3 = "Hello, AIDA Record 3";

    StorageEngine::insertRecord(1, record1, strlen(record1) + 1);
    StorageEngine::insertRecord(1, record2, strlen(record2) + 1);
    StorageEngine::insertRecord(1, record3, strlen(record3) + 1);

    // Read records
    char buffer[MAX_RECORD_SIZE];

    std::cout << "\n🔍 Reading records after insertion:" << std::endl;
    for (int i = 0; i < 3; ++i)
    {
        if (StorageEngine::readRecord(1, i, buffer, sizeof(buffer)))
            std::cout << "Record " << i << ": " << buffer << std::endl;
    }

    // Delete Record 1
    std::cout << "\n🗑️ Deleting Record 1..." << std::endl;
    StorageEngine::deleteRecord(1, 1);

    // Read records after deletion
    std::cout << "\n🔍 Reading records after deletion:" << std::endl;
    for (int i = 0; i < 3; ++i)
    {
        if (StorageEngine::readRecord(1, i, buffer, sizeof(buffer)))
            std::cout << "Record " << i << ": " << buffer << std::endl;
        else
            std::cout << "Record " << i << ": [Deleted or Not Found]" << std::endl;
    }

    // Update Record 2 (index 2)
    const char *updatedRecord3 = "Hello, AIDA Record 3 UPDATED!";
    std::cout << "\n✏️ Updating Record 2..." << std::endl;
    StorageEngine::updateRecord(1, 2, updatedRecord3, strlen(updatedRecord3) + 1);

    // Read records after update
    std::cout << "\n🔍 Reading records after update:" << std::endl;
    for (int i = 0; i < 4; ++i)
    {
        if (StorageEngine::readRecord(1, i, buffer, sizeof(buffer)))
            std::cout << "Record " << i << ": " << buffer << std::endl;
        else
            std::cout << "Record " << i << ": [Deleted or Not Found]" << std::endl;
    }

    StorageEngine::closeStorageFile();
    return 0;
}
