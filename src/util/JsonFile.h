#ifndef SIRIUS_JSON_FILE_H
#define SIRIUS_JSON_FILE_H

#include <string>

// Atomic write: dump `contents` to `path.tmp`, then rename over `path`. Returns
// empty string on success or a short error description on failure.
std::string writeFileAtomic(const std::string& path, const std::string& contents);

#endif
