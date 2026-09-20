#!/bin/sh
set -e

git pull --ff-only

[ -f build/CMakeCache.txt ] || cmake -S . -B build   # first run

cmake --build build --target rpi
./build/rpi
