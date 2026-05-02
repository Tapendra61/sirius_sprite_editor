#ifndef SIRIUS_GUID_H
#define SIRIUS_GUID_H

#include <string>

// Returns a 32-character lowercase hex string (no dashes), e.g.
// "5e97eb03825dee720800000000000000". Suitable for Unity asset GUIDs and
// per-sprite spriteIDs. Not cryptographically strong — uses std::mt19937_64
// seeded from std::random_device, which is fine for asset-id uniqueness.
std::string makeGuid32();

#endif
