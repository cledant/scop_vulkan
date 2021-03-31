#ifndef SCOP_APP_VERSION_HPP
#define SCOP_APP_VERSION_HPP

#include <cstdint>

#ifndef VERSION_MAJOR
#define VERSION_MAJOR 0
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR 0
#endif

#ifndef VERSION_PATCH
#define VERSION_PATCH 0
#endif

#ifndef COMMIT_HASH
#define COMMIT_HASH "???"
#endif

namespace scop {
constexpr uint32_t const APP_VERSION_MAJOR = VERSION_MAJOR;
constexpr uint32_t const APP_VERSION_MINOR = VERSION_MINOR;
constexpr uint32_t const APP_VERSION_PATCH = VERSION_PATCH;
constexpr char const *APP_COMMIT_HASH = COMMIT_HASH;
}

#endif // SCOP_APP_VERSION_HPP
