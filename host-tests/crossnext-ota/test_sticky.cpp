// Built as a Sticky (-DFREEINK_DEVICE_STICKY=1) whose env overrides the release
// source, the way platformio.ini's sticky envs do.
#include <ReleaseJsonParser.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

#include "../../src/network/FirmwareBoardTag.h"
#include "../../src/network/ReleaseSources.h"

namespace {
int checks = 0, failed = 0;
void check(bool ok, const std::string& message) {
  ++checks;
  if (!ok) {
    ++failed;
    std::fprintf(stderr, "FAIL: %s\n", message.c_str());
  }
}
std::string asset(const std::string& name, const std::string& url, int size) {
  return "{\"size\":" + std::to_string(size) + ",\"browser_download_url\":\"" + url + "\",\"name\":\"" + name + "\"}";
}

// Every asset a fork release or a neighbouring board could publish, none of
// which is a Sticky application image.
const char* const kOthers[] = {
    "CrossNext-X4PRO-OTA.bin",
    "firmware.bin",
    "firmware-x4pro.bin",
    "firmware-papermono.bin",
    "crossnext-v1.4.0-fork.1-sticky-full.bin",
    "firmware-sticky-v1.4.0-fork.1-full.bin",
    "firmware-sticky-v1.4.0-fork.0.bin",
    "firmware-sticky-v1.4.bin",
    "firmware-sticky-1.4.0.bin",
    "firmware-stickyx-v1.4.0.bin",
    "firmware-sticky-v.bin",
    // 49 characters whose first 47 are a valid Sticky image name: stored
    // truncated, it would pass for one.
    "firmware-sticky-v999999.999999.99999-fork.9.bin.x",
    "firmware-sticky-v1.4.0-fork.1.bin.sha256",
    "firmware-sticky-v1.4.0-fork.1-and-a-name-too-long-to-store.bin",
    "CrossNext-Resources.zip",
    "SHA256SUMS",
};

// The selected asset for a release listing `others` plus, when given, `wanted`.
void parse(const std::string& wanted) {
  std::string assets;
  int size = 1;
  for (const char* other : kOthers) {
    if (!assets.empty()) assets += ",";
    assets += asset(other, std::string("https://example.invalid/") + other, size++);
  }
  if (!wanted.empty()) assets += "," + asset(wanted, "https://example.invalid/" + wanted, 5600000);
  for (size_t chunk : {size_t(1), size_t(3), size_t(512)}) {
    const std::string json = "{\"assets\":[" + assets + "],\"tag_name\":\"v1.4.0-fork.1\"}";
    ReleaseJsonParser parser;
    parser.setFirmwareAssetName(CROSSPOINT_RELEASE_ASSET);
    for (size_t at = 0; at < json.size(); at += chunk) parser.feed(json.data() + at, std::min(chunk, json.size() - at));
    check(parser.foundTag() && std::strcmp(parser.getTagName(), "v1.4.0-fork.1") == 0, "fork tag parsed");
    if (wanted.empty()) {
      check(!parser.foundFirmware(), "no Sticky image among other boards' and look-alike assets");
    } else {
      check(parser.foundFirmware(), "Sticky image found: " + wanted);
      check(parser.getFirmwareUrl() == "https://example.invalid/" + wanted, "URL is the Sticky image's: " + wanted);
      check(parser.getFirmwareSize() == 5600000, "size is the Sticky image's: " + wanted);
    }
  }
}
}  // namespace

int main() {
  check(std::strcmp(release_sources::kUrls[0], CROSSNEXT_RELEASE_URL) == 0, "the env's release source is used");
  check(
      std::strcmp(release_sources::kUrls[0], "https://api.github.com/repos/manuelx900/CrossNext/releases/latest") != 0,
      "the override replaces upstream rather than adding to it");
  check(release_sources::kCount == 1, "one release source");
  check(std::strcmp(CROSSPOINT_RELEASE_ASSET, "firmware-sticky.bin") == 0, "Sticky legacy asset name");
  parse("firmware-sticky.bin");
  parse("firmware-sticky-v1.4.0-fork.1.bin");
  parse("firmware-sticky-v1.5.0.bin");
  parse("");
  std::printf("crossnext-ota sticky: %d checks, %d failed\n", checks, failed);
  return failed ? 1 : 0;
}
