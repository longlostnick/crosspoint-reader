#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build/home_selector_memory_tests"
BINARY="$BUILD_DIR/HomeSelectorMemoryTest"

mkdir -p "$BUILD_DIR"

CXXFLAGS=(
  -std=c++20
  -O2
  -Wall
  -Wextra
  -pedantic
  -I"$ROOT_DIR"
)

c++ "${CXXFLAGS[@]}" "$ROOT_DIR/test/home_selector_memory/HomeSelectorMemoryTest.cpp" -o "$BINARY"

"$BINARY"
