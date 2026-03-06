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

# Build test executables
add_executable(test_everything test/test_everything.cpp)
target_link_libraries(test_everything phonemis)

add_executable(test_preprocessor test/test_preprocessor.cpp)
target_link_libraries(test_preprocessor phonemis)

add_executable(test_tokenizer test/test_tokenizer.cpp)
target_link_libraries(test_tokenizer phonemis)

add_executable(test_pipeline test/test_pipeline.cpp)
target_link_libraries(test_pipeline phonemis)