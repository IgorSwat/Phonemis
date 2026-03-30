#!/bin/bash

# 1. Creates temporary /build dir and moves into it
mkdir -p build
cd build

# 2. Builds the project with cmake
cmake .. -DBUILD_TESTS=ON

# 3. Compiles it
make

# 4. Runs produced ./phonemis_test executable
if [ -f "./phonemis_test" ]; then
    ./phonemis_test
else
    echo "Error: phonemis_test executable not found."
    cd ..
    rm -rf build
    exit 1
fi

# 5. Moves away from build and removes it
cd ..
rm -rf build
