cmake_minimum_required(VERSION 3.10)
project(phonemis VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phonemis/*.cpp
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Option to build tests
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    # Define test source files
    file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
    
    # Create test executable
    add_executable(phonemis_test ${TEST_SOURCES} ${SOURCE_FILES})
    
    # Include test directory for test.h etc.
    target_include_directories(phonemis_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test)
    
    # Set include path for the library sources
    target_include_directories(phonemis_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
else()
    # Build static library
    add_library(phonemis STATIC ${SOURCE_FILES})
endif()