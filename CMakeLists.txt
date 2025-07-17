cmake_minimum_required(VERSION 3.10)
project(AIDA)

set(CMAKE_CXX_STANDARD 17)

# Include header files
include_directories(include)

# Add source files
add_executable(aida
    src/main.cpp
    src/StorageEngine.cpp
)
