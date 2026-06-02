#!/bin/bash

# 1. Creates build dir and moves into it
mkdir -p build
cd build

# 2. Builds the project with cmake
cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo

# 3. Compiles it
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

# 4. Runs produced ./phonemis_test executable
if [ -f "./phonemis_test" ]; then
    ./phonemis_test
else
    echo "Error: phonemis_test executable not found."
    exit 1
fi
