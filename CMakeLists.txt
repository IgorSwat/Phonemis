cmake_minimum_required(VERSION 3.10)
project(phonemis VERSION 1.0 LANGUAGES CXX)

# --- Configuration ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_RUNNER "Build phonemis runner" OFF)
option(BUILD_TESTS "Build tests" OFF)

# --- Includes ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/third-party)

# --- Source Files ---
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phonemis/*.cpp
)
# Exclude main.cpp from the static library
list(FILTER LIB_SOURCES EXCLUDE REGEX "main\\.cpp$")

# --- Targets ---

# Always build the static library
add_library(phonemis STATIC ${LIB_SOURCES})

# Build runner if requested
if(BUILD_RUNNER)
    set(MAIN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/phonemis/main.cpp)
    add_executable(phonemis_runner ${MAIN_SOURCE})
    target_link_libraries(phonemis_runner PRIVATE phonemis)
endif()

# Build tests if requested
if(BUILD_TESTS)
    file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
    
    add_executable(phonemis_test ${TEST_SOURCES})
    target_link_libraries(phonemis_test PRIVATE phonemis)
    
    target_include_directories(phonemis_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test)
    target_include_directories(phonemis_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_definitions(phonemis_test PRIVATE PHONEMIS_PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_options(phonemis_test PRIVATE -Wno-deprecated-declarations)
endif()