cmake_minimum_required(VERSION 3.10)
project(phonemis VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phonemis/*.cpp
)

# List of source files for library (excluding main.cpp to avoid duplicate symbols in tests)
list(FILTER SOURCE_FILES EXCLUDE REGEX "main\\.cpp$")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/third-party)

# Option to build runner
option(BUILD_RUNNER "Build phonemis runner" OFF)

# Option to build tests
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_RUNNER)
    # Create the main executable
    set(MAIN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/phonemis/main.cpp)
    add_executable(phonemis_runner ${MAIN_SOURCE} ${SOURCE_FILES})
endif()

if(BUILD_TESTS)
    # Define test source files
    file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
    
    # Create test executable
    add_executable(phonemis_test ${TEST_SOURCES} ${SOURCE_FILES})
    
    # Include test directory for test.h etc.
    target_include_directories(phonemis_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test)
    
    # Set include path for the library sources
    target_include_directories(phonemis_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

    # Define the project root macro for tests
    target_compile_definitions(phonemis_test PRIVATE PHONEMIS_PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

    target_compile_options(phonemis_test PRIVATE -Wno-deprecated-declarations)
else()
    # Build static library
    add_library(phonemis STATIC ${SOURCE_FILES})
endif()