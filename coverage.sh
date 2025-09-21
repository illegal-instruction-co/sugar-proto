#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

BUILD_DIR="build"
COV_DIR="$BUILD_DIR/coverage"

rm -rf "$COV_DIR"
mkdir -p "$COV_DIR"

echo "[1/3] CMake configure & build with coverage flags..."
cmake -S . -B "$BUILD_DIR" -DCOVERAGE=ON -DBUILD_TESTS=ON
cmake --build "$BUILD_DIR" -- -j"$(nproc)"

echo "[2/3] Running all test executables..."
find "$BUILD_DIR" -type f -perm -111 -name "unit_test_*" | while read -r testexe; do
    echo "Running $testexe"
    "$testexe"
done

echo "[3/3] Generating coverage report with gcovr..."

gcovr -r . "$BUILD_DIR" \
  --filter 'src/.*' \
  --exclude 'test/.*' \
  --exclude 'generated/.*' \
  > "$COV_DIR/coverage.txt"

gcovr -r . "$BUILD_DIR" \
  --filter 'src/.*' \
  --exclude 'test/.*' \
  --exclude 'generated/.*' \
  --html --html-details -o "$COV_DIR/coverage.html"

gcovr -r . "$BUILD_DIR" \
  --filter 'src/.*' \
  --exclude 'test/.*' \
  --exclude 'generated/.*' \
  --xml -o "$COV_DIR/coverage.xml"

echo
echo "Coverage reports generated in: $COV_DIR"
echo " - Console summary: $COV_DIR/coverage.txt"
echo " - HTML report   : $COV_DIR/coverage.html"
echo " - XML report    : $COV_DIR/coverage.xml"
