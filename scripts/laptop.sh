#!/bin/sh
set -e

[ -f build/CMakeCache.txt ] || cmake -S . -B build   # first run

cmake --build build --target laptop
./build/laptop
