#ifndef SIRIUS_TRIM_H
#define SIRIUS_TRIM_H

#include "raylib.h"

// Returns a tight rect around non-transparent pixels within `bounds`.
// If every pixel is below the alpha threshold, the original bounds are returned.
// Decodes the image to RGBA8 once internally — for batch trims, prefer the
// pre-decoded variant below to avoid repeated decode work.
Rectangle trimTransparentEdges(const Image& image, Rectangle bounds, int alphaThreshold = 0);

// Same, but caller supplies a pre-decoded RGBA8 pixel buffer (e.g. from
// LoadImageColors). `imageWidth` and `imageHeight` describe the buffer.
Rectangle trimTransparentEdges(const Color* pixels, int imageWidth, int imageHeight,
                               Rectangle bounds, int alphaThreshold = 0);

#endif
