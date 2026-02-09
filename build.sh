#!/bin/bash
set -e

mkdir -p build

g++ -std=c++17 -O2 -Wall -Wextra src/main.cpp -o build/vier_gewinnt

echo "Build OK."
echo "Start: ./build/vier_gewinnt"
