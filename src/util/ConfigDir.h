#ifndef SIRIUS_CONFIG_DIR_H
#define SIRIUS_CONFIG_DIR_H

#include <string>

// Returns the per-user config directory for this app, with trailing slash.
// Creates the directory if it doesn't exist.
//
//   Windows: %APPDATA%/SiriusSpriteEditor/
//   macOS:   ~/Library/Application Support/SiriusSpriteEditor/
//   Linux:   $XDG_CONFIG_HOME/SiriusSpriteEditor/  (or ~/.config/...)
//
// Falls back to the executable directory if no per-user dir can be resolved.
const std::string& configDir();

// Convenience: returns configDir() + filename.
std::string configPath(const std::string& filename);

#endif
