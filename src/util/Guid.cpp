#include "util/Guid.h"

#include <cstdio>
#include <random>

std::string makeGuid32() {
    static thread_local std::mt19937_64 rng = []() {
        std::random_device rd;
        std::seed_seq seed{
            rd(), rd(), rd(), rd(),
            (unsigned)std::random_device{}(),
        };
        return std::mt19937_64(seed);
    }();

    uint64_t a = rng();
    uint64_t b = rng();
    char buf[33];
    std::snprintf(buf, sizeof(buf),
                  "%016llx%016llx",
                  (unsigned long long)a,
                  (unsigned long long)b);
    return std::string(buf);
}
