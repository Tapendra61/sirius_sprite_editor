#include "util/ConfigDir.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>
#include "raylib.h"

static const char* APP_FOLDER_NAME = "SiriusSpriteEditor";

static std::string ensureTrailingSlash(std::string s) {
    if (s.empty()) return s;
    char last = s.back();
    if (last != '/' && last != '\\') s += '/';
    return s;
}

static std::string platformConfigBase() {
    // Windows: %APPDATA% (e.g. C:\Users\<u>\AppData\Roaming)
    if (const char* appdata = std::getenv("APPDATA")) {
        return std::string(appdata);
    }

    const char* home = std::getenv("HOME");
    if (!home) return std::string();  // fall back to app dir below

#if defined(__APPLE__)
    // macOS: ~/Library/Application Support
    return std::string(home) + "/Library/Application Support";
#else
    // Linux / BSD: respect XDG_CONFIG_HOME, default ~/.config
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) {
        if (xdg[0] != '\0') return std::string(xdg);
    }
    return std::string(home) + "/.config";
#endif
}

const std::string& configDir() {
    static std::string cached;
    if (!cached.empty()) return cached;

    std::string base = platformConfigBase();
    std::string dir;
    if (!base.empty()) {
        dir = ensureTrailingSlash(base) + APP_FOLDER_NAME;
    } else {
        // Last resort: write next to the executable.
        dir = std::string(GetApplicationDirectory()) + APP_FOLDER_NAME;
    }

    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    // Even if creation failed, return the path; callers will get an open
    // failure and fall back to defaults gracefully.

    cached = ensureTrailingSlash(dir);
    return cached;
}

std::string configPath(const std::string& filename) {
    return configDir() + filename;
}
