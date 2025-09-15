#!/bin/bash

BUILD=${1:-debug}  # default to debug if no argument given

if [[ "$BUILD" == "debug" ]]; then
    mkdir -p build/linux-x64-debug
    g++ -g -Iinclude source/profiler.cpp -shared -fPIC -std=c++20 \
        -o build/linux-x64-debug/profiler.so
elif [[ "$BUILD" == "release" ]]; then
    mkdir -p build/linux-x64-release
    g++ -O2 -DNDEBUG -Iinclude source/profiler.cpp -shared -fPIC -std=c++20 \
        -o build/linux-x64-release/profiler.so
else
    echo "Unknown build type: $BUILD"
    exit 1
fi
