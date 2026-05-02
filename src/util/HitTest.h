#ifndef SIRIUS_HIT_TEST_H
#define SIRIUS_HIT_TEST_H

#include <vector>
#include "raylib.h"

class SliceStore;
struct CanvasView;

enum class HandleId {
    None,
    Body,
    NW, N, NE,
    E,
    SE, S, SW,
    W
};

struct Hit {
    int sliceId;
    HandleId handle;
};

Hit pickHit(Vector2 mouseScreen, const SliceStore& store, const CanvasView& view);

// Returns all slice IDs whose body contains the cursor, ordered top-most first.
std::vector<int> pickAllBodyHits(Vector2 mouseScreen, const SliceStore& store, const CanvasView& view);

#endif
