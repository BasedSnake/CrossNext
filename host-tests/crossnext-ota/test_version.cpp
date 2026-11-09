#include <FirmwareVersion.h>

#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace {
int checks = 0, failed = 0;
void check(bool ok, const char* message) {
  ++checks;
  if (!ok) {
    ++failed;
    std::fprintf(stderr, "FAIL: %s\n", message);
  }
}

struct Offer {
  const char* latest;
  const char* current;
  bool offered;
};

// What a device running `current` is offered when the newest release is `latest`.
constexpr Offer kOffers[] = {
    {"1.4.0-fork.2", "1.4.0-fork.1", true},
    {"1.4.0-fork.1", "1.4.0-fork.2", false},
    {"1.4.0-fork.10", "1.4.0-fork.9", true},  // counters compare as numbers
    {"1.4.0-fork.1", "1.4.0", true},          // a dev build moves onto the fork line
    {"1.4.0", "1.4.0-fork.1", false},         // the counter ranks after its base
    {"1.5.0-fork.1", "1.4.0-fork.9", true},   // an upstream base bump wins
    {"1.4.0-fork.9", "1.5.0-fork.1", false},
    {"1.5.0", "1.4.0-fork.3", true},
    {"1.10.0", "1.9.0", true},                // segments compare as numbers
    {"1.4.0-fork.1", "1.4.0-fork.1", false},  // equal
    {"1.4.0", "1.4.0", false},
    {"1.4.0", "1.4.0-rc+abc123", true},  // a release candidate is offered its release
    {"1.4.0-fork.1", "1.4.0-rc+abc123", true},
    {"1.3.9", "1.4.0-rc+abc123", false},
    {"v1.4.1", "1.4.0", false},  // the updater strips the v; a raw tag never parses
    {"1.4", "1.3.0", false},
    {"1.4.1-fork.x", "1.4.0", false},
    {"1.4.1-fork.0", "1.4.0", false},
    {"1.4.1-fork.1-rc", "1.4.0", false},
    {"", "1.4.0", false},
};

void testOffers() {
  for (const Offer& o : kOffers) {
    char message[96];
    std::snprintf(message, sizeof(message), "%s offered to %s: %s", o.latest, o.current, o.offered ? "yes" : "no");
    check(firmware_version::isNewer(o.latest, o.current) == o.offered, message);
  }
}

void testReleaseForm() {
  for (const char* text : {"1.4.0", "1.4.0-fork.1", "12.0.3-fork.27"}) {
    check(firmware_version::isReleaseVersion(text, std::strlen(text)), text);
  }
  for (const char* text : {"1.4", "1.4.0-rc+abc", "1.4.0-fork.0", "1.4.0-fork.", "1.4.0-fork.1-full", "v1.4.0", "",
                           "1.4.0-fork.1234567"}) {
    char message[64];
    std::snprintf(message, sizeof(message), "not a release version: '%s'", text);
    check(!firmware_version::isReleaseVersion(text, std::strlen(text)), message);
  }
}
}  // namespace

int main() {
  testOffers();
  testReleaseForm();
  std::printf("crossnext-ota version: %d checks, %d failed\n", checks, failed);
  return failed ? 1 : 0;
}
