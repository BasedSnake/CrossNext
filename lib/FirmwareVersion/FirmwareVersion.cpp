#include "FirmwareVersion.h"

#include <cstring>

namespace firmware_version {
namespace {

// One decimal segment. Bounded so a hostile tag cannot overflow the counter.
bool readNumber(const char*& p, const char* end, unsigned& out) {
  if (p == end || *p < '0' || *p > '9') return false;
  unsigned value = 0;
  while (p != end && *p >= '0' && *p <= '9') {
    if (value > 99999) return false;
    value = value * 10 + static_cast<unsigned>(*p - '0');
    ++p;
  }
  out = value;
  return true;
}

bool readDot(const char*& p, const char* end) {
  if (p == end || *p != '.') return false;
  ++p;
  return true;
}

}  // namespace

bool parse(const char* text, size_t len, Version& out) {
  out = Version{};
  Version v;
  const char* p = text;
  const char* const end = text + len;
  if (!readNumber(p, end, v.major) || !readDot(p, end) || !readNumber(p, end, v.minor) || !readDot(p, end) ||
      !readNumber(p, end, v.patch)) {
    return false;
  }
  if (p == end) {
    v.releaseForm = true;
    out = v;
    return true;
  }
  static constexpr char kFork[] = "-fork.";
  constexpr size_t kForkLen = sizeof(kFork) - 1;
  if (static_cast<size_t>(end - p) >= kForkLen && std::memcmp(p, kFork, kForkLen) == 0) {
    p += kForkLen;
    if (!readNumber(p, end, v.fork) || v.fork == 0 || p != end) return false;
    v.releaseForm = true;
  }
  out = v;
  return true;
}

bool isReleaseVersion(const char* text, size_t len) {
  Version v;
  return parse(text, len, v) && v.releaseForm;
}

bool isNewer(const char* latest, const char* current) {
  if (std::strcmp(latest, current) == 0) return false;
  Version l;
  if (!parse(latest, std::strlen(latest), l)) return false;
  // The running version is compiled in. One that does not parse compares as
  // 0.0.0, so any parseable release is offered rather than none.
  Version c;
  parse(current, std::strlen(current), c);
  if (l.major != c.major) return l.major > c.major;
  if (l.minor != c.minor) return l.minor > c.minor;
  if (l.patch != c.patch) return l.patch > c.patch;
  if (l.fork != c.fork) return l.fork > c.fork;
  return std::strstr(current, "-rc") != nullptr;
}

}  // namespace firmware_version
