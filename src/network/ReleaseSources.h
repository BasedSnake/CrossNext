#pragma once

// Where Check for Updates looks. A build env overrides it with
// -DCROSSNEXT_RELEASE_URL; the Sticky envs point at the fork that publishes
// Sticky releases, every other board keeps upstream's.
#ifndef CROSSNEXT_RELEASE_URL
#define CROSSNEXT_RELEASE_URL "https://api.github.com/repos/manuelx900/CrossNext/releases/latest"
#endif

namespace release_sources {
constexpr const char* const kUrls[] = {
    CROSSNEXT_RELEASE_URL,
};
constexpr int kCount = static_cast<int>(sizeof(kUrls) / sizeof(kUrls[0]));
}  // namespace release_sources
