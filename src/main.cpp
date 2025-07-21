#include <iostream>
#include <cstring>
#include "StorageEngine.h"

int main()
{
    std::cout << "AIDA Storage Engine Started 🚀\n";

    StorageEngine::createStorageFile("mydb.aida", 10);
    if (!StorageEngine::openStorageFile("mydb.aida"))
        return 1;

    // Insert a few records
    const char *rec1 = "Hello, AIDA Record 1";
    const char *rec2 = "Hello, AIDA Record 2";
    const char *rec3 = "Hello, AIDA Record 3";
    StorageEngine::insertRecord(1, rec1, strlen(rec1) + 1);
    StorageEngine::insertRecord(1, rec2, strlen(rec2) + 1);
    StorageEngine::insertRecord(1, rec3, strlen(rec3) + 1);

    // Read them back
    char buf[MAX_RECORD_SIZE];
    std::cout << "\n🔍 Reading after insert:\n";
    for (int i = 0; i < 3; ++i)
    {
        if (StorageEngine::readRecord(1, i, buf, sizeof(buf)))
            std::cout << "Record " << i << ": " << buf << "\n";
    }

    // Delete record 1
    std::cout << "\n🗑️ Deleting record 1...\n";
    StorageEngine::deleteRecord(1, 1);

    std::cout << "\n🔍 Reading after delete:\n";
    for (int i = 0; i < 3; ++i)
    {
        if (StorageEngine::readRecord(1, i, buf, sizeof(buf)))
            std::cout << "Record " << i << ": " << buf << "\n";
        else
            std::cout << "Record " << i << ": [Deleted/Not Found]\n";
    }

    // Update record 2
    const char *upd3 = "Hello, AIDA Record 3 UPDATED!";
    std::cout << "\n✏️ Updating record 2...\n";
    StorageEngine::updateRecord(1, 2, upd3, strlen(upd3) + 1);

    std::cout << "\n🔍 Reading after update:\n";
    for (int i = 0; i < 4; ++i)
    {
        if (StorageEngine::readRecord(1, i, buf, sizeof(buf)))
            std::cout << "Record " << i << ": " << buf << "\n";
        else
            std::cout << "Record " << i << ": [Deleted/Not Found]\n";
    }

    StorageEngine::closeStorageFile();
    return 0;
}
