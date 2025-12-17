cmake_minimum_required(VERSION 3.10)
project(phonemis VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE SOURCE_FILES "phonemis/src/*.cpp")

# Include directories
include_directories(phonemis/include)

# Build static library
add_library(phonemis STATIC ${SOURCE_FILES})
