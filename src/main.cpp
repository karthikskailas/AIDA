#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include "StorageEngine.h"

int main()
{
    std::cout << "AIDA Storage Engine Test with Separate Logs 🚀\n";

    // Open log files
    std::ofstream insertLog("insert.txt");
    std::ofstream readLog("read.txt");
    std::ofstream deleteLog("delete.txt");

    // Create & open database
    StorageEngine::createStorageFile("mydb.aida", 50);
    if (!StorageEngine::openStorageFile("mydb.aida"))
        return 1;

    // ── INSERT TEST ──
    const int TOTAL = 1000;
    insertLog << "=== INSERT TEST ===\n";
    for (int i = 0; i < TOTAL; ++i)
    {
        std::ostringstream oss;
        oss << "Record #" << i << " — stress test entry.";
        std::string rec = oss.str();

        bool ok = StorageEngine::insertRecord(1, rec.c_str(), rec.size() + 1);
        insertLog << (ok
                          ? "OK:   Inserted record " + std::to_string(i) + "\n"
                          : "FAIL: Insert record " + std::to_string(i) + "\n");
    }

    // ── READ TEST ──
    // Spot-check a few positions
    int checks[] = {0, TOTAL / 2, TOTAL - 1};
    readLog << "=== READ TEST ===\n";
    for (int idx : checks)
    {
        char buf[MAX_RECORD_SIZE] = {};
        bool ok = StorageEngine::readRecord(1, idx, buf, sizeof(buf));
        if (ok)
        {
            readLog << "OK:   Record " << idx << ": " << buf << "\n";
        }
        else
        {
            readLog << "FAIL: Record " << idx << " not found\n";
        }
    }

    // ── DELETE TEST ──
    int delIdx = TOTAL / 2;
    deleteLog << "=== DELETE TEST ===\n";
    {
        bool ok = StorageEngine::deleteRecord(1, delIdx);
        deleteLog << (ok
                          ? "OK:   Deleted record " + std::to_string(delIdx) + "\n"
                          : "FAIL: Delete record " + std::to_string(delIdx) + "\n");
    }

    // ── READ-AFTER-DELETE ──
    readLog << "\n=== READ AFTER DELETE ===\n";
    {
        char buf[MAX_RECORD_SIZE] = {};
        bool ok = StorageEngine::readRecord(1, delIdx, buf, sizeof(buf));
        if (ok)
        {
            readLog << "ERROR: Record " << delIdx << " should be deleted but read \"" << buf << "\"\n";
        }
        else
        {
            readLog << "OK:   Record " << delIdx << " correctly reported deleted/not found\n";
        }
    }

    // Clean up
    StorageEngine::closeStorageFile();
    insertLog.close();
    readLog.close();
    deleteLog.close();

    std::cout << "Logs written to insert.txt, read.txt, delete.txt\n";
    return 0;
}
