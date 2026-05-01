#ifndef SIRIUS_LOGGER_H
#define SIRIUS_LOGGER_H

#include <cstdio>

inline void LogInfo(const char* msg) {
    std::printf("[info] %s\n", msg);
}

inline void LogWarn(const char* msg) {
    std::printf("[warn] %s\n", msg);
}

inline void LogError(const char* msg) {
    std::fprintf(stderr, "[error] %s\n", msg);
}

#endif
