#!/bin/sh
set -e
cd "$(dirname "$0")"
BUILD_DIR="${TMPDIR:-/tmp}/$(basename "${CXX:-c++}")-crossnext-ota-tests-$(cd ../.. && pwd | cksum | cut -d" " -f1)"
mkdir -p "$BUILD_DIR"
CXXFLAGS="-std=c++17 -Wall -Wextra -Werror -I../../lib/JsonParser -I../../lib/FirmwareVersion \
  -I../../freeink-sdk/libs/network/JsonSax/include"
PARSER="../../freeink-sdk/libs/network/JsonSax/src/StreamingJsonParser.cpp ../../lib/JsonParser/ReleaseJsonParser.cpp \
  ../../lib/FirmwareVersion/FirmwareVersion.cpp"
# shellcheck disable=SC2086
"${CXX:-c++}" $CXXFLAGS -DFREEINK_DEVICE_X4PRO=1 $PARSER test.cpp -o "$BUILD_DIR/test_crossnext_ota"
# shellcheck disable=SC2086
"${CXX:-c++}" $CXXFLAGS -DFREEINK_DEVICE_STICKY=1 \
  '-DCROSSNEXT_RELEASE_URL="https://example.invalid/fork/releases/latest"' \
  $PARSER test_sticky.cpp -o "$BUILD_DIR/test_crossnext_ota_sticky"
# shellcheck disable=SC2086
"${CXX:-c++}" $CXXFLAGS ../../lib/FirmwareVersion/FirmwareVersion.cpp test_version.cpp \
  -o "$BUILD_DIR/test_crossnext_ota_version"
"$BUILD_DIR/test_crossnext_ota"
"$BUILD_DIR/test_crossnext_ota_sticky"
"$BUILD_DIR/test_crossnext_ota_version"
