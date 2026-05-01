#ifndef SIRIUS_TRIM_H
#define SIRIUS_TRIM_H

#include "raylib.h"

// Returns a tight rect around non-transparent pixels within `bounds`.
// If every pixel is below the alpha threshold, the original bounds are returned.
Rectangle trimTransparentEdges(const Image& image, Rectangle bounds, int alphaThreshold = 0);

#endif
