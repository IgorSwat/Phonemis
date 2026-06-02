# ------------------------
# --- Project metadata ---
# ------------------------
cmake_minimum_required(VERSION 3.10)
project(phonemis VERSION 1.0 LANGUAGES CXX)

# ---------------------
# --- Configuration ---
# ---------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# By default, we only build a static library.
# Building the tests (-DBUILD_TESTS=ON) and building the sample runner (-DBUILD_RUNNER=ON) must
# be explciitely specified as CMAKE options.
option(BUILD_RUNNER "Build phonemis runner" OFF)
option(BUILD_TESTS "Build tests" OFF)

# Additional option to specify the usage of ExecuTorch for neural model inference.
# To use it, you must explicitely link the package with ExecuTorch libs or
# build it together.
option(ET_ON "Use ExecuTorch" OFF)

# ----------------
# --- Includes ---
# ----------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/third-party)

# --------------------
# --- Source Files ---
# --------------------
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/phonemis/*.cpp
)
# Exclude main.cpp from the static library
list(FILTER LIB_SOURCES EXCLUDE REGEX "main\\.cpp$")

# Exclude protophone sources when ExecuTorch is used
if(ET_ON)
  list(FILTER LIB_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/src/phonemis/protophone/")
endif()

# ---------------
# --- Targets ---
# ---------------
# Always build the static library
add_library(phonemis STATIC ${LIB_SOURCES})

if(ET_ON)
  target_compile_definitions(phonemis PRIVATE ET_ON)
endif()

# Threading support (std::thread in the Protophone convolution stage).
find_package(Threads REQUIRED)
target_link_libraries(phonemis PUBLIC Threads::Threads)

# SIMD architecture selection (xsimd)
# xsimd picks its widest batch type at compile time from the enabled ISA. ARM
# targets (Android arm64-v8a, iOS arm64) already imply NEON, so nothing is
# needed there. For native x86_64 desktop builds we opt into AVX2 + FMA. The
# flag is PUBLIC so every translation unit (and the tests) share the same
# default arch, keeping xsimd's alignment/ABI consistent across the boundary.
if(NOT ET_ON AND NOT ANDROID AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS"
   AND CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64|amd64)")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-mavx2" PHONEMIS_HAS_AVX2)
  check_cxx_compiler_flag("-mfma" PHONEMIS_HAS_FMA)
  if(PHONEMIS_HAS_AVX2)
    target_compile_options(phonemis PUBLIC -mavx2)
  endif()
  if(PHONEMIS_HAS_FMA)
    target_compile_options(phonemis PUBLIC -mfma)
  endif()
endif()

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

  # Always add previously excluded Protophone sources.
  if(ET_ON)
    file(GLOB_RECURSE PROTOPHONE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/phonemis/protophone/*.cpp)
    target_sources(phonemis_test PRIVATE ${PROTOPHONE_SOURCES})
  endif()
endif()