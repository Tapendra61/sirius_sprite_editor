#ifndef SIRIUS_SLICE_H
#define SIRIUS_SLICE_H

#include <string>
#include "raylib.h"

struct Slice {
    int id;
    std::string name;
    Rectangle rect;
    Vector2 pivot;
    Rectangle border;
};

#endif
