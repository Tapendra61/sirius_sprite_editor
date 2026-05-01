#ifndef SIRIUS_GEOMETRY_H
#define SIRIUS_GEOMETRY_H

#include <cmath>
#include "raylib.h"

inline Rectangle RectFromCorners(Vector2 a, Vector2 b) {
    Rectangle r;
    r.x = (a.x < b.x) ? a.x : b.x;
    r.y = (a.y < b.y) ? a.y : b.y;
    r.width  = (a.x < b.x) ? (b.x - a.x) : (a.x - b.x);
    r.height = (a.y < b.y) ? (b.y - a.y) : (a.y - b.y);
    return r;
}

inline Rectangle NormalizeRect(Rectangle r) {
    if (r.width < 0.0f) {
        r.x += r.width;
        r.width = -r.width;
    }
    if (r.height < 0.0f) {
        r.y += r.height;
        r.height = -r.height;
    }
    return r;
}

inline Rectangle SnapRectToPixels(Rectangle r) {
    Rectangle out;
    out.x = std::round(r.x);
    out.y = std::round(r.y);
    out.width = std::round(r.width);
    out.height = std::round(r.height);
    return out;
}

inline bool RectsOverlap(Rectangle a, Rectangle b) {
    if (a.x + a.width  < b.x) return false;
    if (b.x + b.width  < a.x) return false;
    if (a.y + a.height < b.y) return false;
    if (b.y + b.height < a.y) return false;
    return true;
}

#endif
