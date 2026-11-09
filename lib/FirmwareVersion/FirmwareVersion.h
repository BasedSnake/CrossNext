#pragma once

#include <cstddef>

// CrossNext version strings and the order the updater offers them in.
//
// A version is "X.Y.Z" with an optional suffix. This fork releases as
// "X.Y.Z-fork.N": upstream's base version plus a counter that restarts at 1 for
// every new base. Unlike a semver prerelease, a fork counter ranks AFTER its
// base, so 1.4.0 < 1.4.0-fork.1 < 1.4.0-fork.2 < 1.5.0.
namespace firmware_version {

struct Version {
  unsigned major = 0;
  unsigned minor = 0;
  unsigned patch = 0;
  unsigned fork = 0;         // 0: no fork counter
  bool releaseForm = false;  // exactly X.Y.Z or X.Y.Z-fork.N
};

// Reads `len` bytes of `text` (no "v" prefix). "-fork.N" (N >= 1, digits only,
// at the end) sets `fork`; any other suffix, such as an -rc build's, leaves it 0.
// Returns false, with `out` zeroed, when X.Y.Z is missing or a "-fork." suffix
// is malformed.
bool parse(const char* text, size_t len, Version& out);

// True when `text` is exactly X.Y.Z or X.Y.Z-fork.N: the only forms a release
// tag or a versioned release asset name carries.
bool isReleaseVersion(const char* text, size_t len);

// True when a device running `current` should be offered `latest` (both without
// the "v" prefix). Equal strings, an unparseable `latest` and older versions are
// never offered. A same-version `current` built as a release candidate (contains
// "-rc") is offered the release it previews.
bool isNewer(const char* latest, const char* current);

}  // namespace firmware_version
