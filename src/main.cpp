#include <iostream>
#include "StorageEngine.h"

int main() {
    std::cout << "AIDA Storage Engine Started 🚀" << std::endl;

    // 1️⃣ Create a 10MB storage file
    StorageEngine::createStorageFile("mydb.aida", 10);

    // 2️⃣ Open the file
    if (StorageEngine::openStorageFile("mydb.aida")) {
        std::cout << "File opened successfully ✅" << std::endl;

        // 3️⃣ Write "Hello, AIDA!" to block 0
        char data[BLOCK_SIZE] = "Hello, AIDA!";
        StorageEngine::writeBlock(0, data);

        // 4️⃣ Read block 0 back into buffer
        char buffer[BLOCK_SIZE] = {};
        StorageEngine::readBlock(0, buffer);

        std::cout << "Data read from block 0: " << buffer << std::endl;

        // 5️⃣ Close the file
        StorageEngine::closeStorageFile();
        std::cout << "File closed successfully ✅" << std::endl;
    } else {
        std::cerr << "Failed to open file ❌" << std::endl;
    }

    return 0;
}
