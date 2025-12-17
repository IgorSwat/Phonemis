#!/bin/bash

# Exit on error
set -e

# --- Configuration ---
PROJECT_NAME="phonemis"
SOURCE_DIR=$(pwd)
BUILD_ROOT="build_android"
OUTPUT_DIR="output_android"
ABIs=("arm64-v8a" "x86_64")
MIN_SDK=21

# Option handling
if [ "$1" == "--clean" ]; then
    # Clean build and output directories
    rm -rf "$BUILD_ROOT" "$OUTPUT_DIR"
    echo "Cleaned $BUILD_ROOT and $OUTPUT_DIR"
    exit 0
fi

# NDK path selection
if [ -n "$ANDROID_NDK_HOME" ]; then
    ndk_path="$ANDROID_NDK_HOME"
elif [ -n "$ANDROID_NDK" ]; then
    ndk_path="$ANDROID_NDK"
else
    echo "Error: ANDROID_NDK_HOME or ANDROID_NDK is not set."
    exit 1
fi

echo "Starting build for Android architectures: ${ABIs[*]}"

# Clean previous builds
rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

# --- Build Loop ---
for abi in "${ABIs[@]}"
do
    # Paths for build and output
    build_dir="$BUILD_ROOT/$abi"
    abi_output_lib_dir="$OUTPUT_DIR/$abi/lib"
    abi_output_include_dir="$OUTPUT_DIR/$abi/include"

    echo "------------------------------------------------"
    echo "Building for ABI: $abi"
    echo "Build Directory: $build_dir"
    echo "------------------------------------------------"

    # Create build directory
    mkdir -p "$build_dir"

    # Configure
    cmake -S "$SOURCE_DIR" -B "$build_dir" \
        -DCMAKE_TOOLCHAIN_FILE="$ndk_path/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI="$abi" \
        -DANDROID_PLATFORM="android-$MIN_SDK" \
        -DCMAKE_BUILD_TYPE=Release \
        -DANDROID_STL=c++_static

    # Build
    cmake --build "$build_dir" --parallel $(nproc)

    # Copy built libraries
    mkdir -p "$abi_output_lib_dir"
    find "$build_dir" -name "*.a" -exec cp {} "$abi_output_lib_dir" \;

    # Copy headers (if present)
    if [ -d "$build_dir/include" ]; then
        mkdir -p "$abi_output_include_dir"
        cp -r "$build_dir/include/"* "$abi_output_include_dir/"
    fi

    echo "Finished building $abi"
done

echo "------------------------------------------------"
echo "All builds completed successfully in $BUILD_ROOT/"
echo "Libraries and headers are located in $OUTPUT_DIR/"