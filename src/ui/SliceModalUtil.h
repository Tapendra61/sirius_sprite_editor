#ifndef SIRIUS_SLICE_MODAL_UTIL_H
#define SIRIUS_SLICE_MODAL_UTIL_H

#include <string>
#include <vector>
#include "model/Slice.h"
#include "raylib.h"

// Token expansion for naming patterns. Supports {i} (index, zero-padded to
// width of `total`), {r} (row), {c} (column). Auto Slice has no grid, so
// callers pass row=col=0 and simply omit those tokens from their patterns.
std::string expandNamePattern(const std::string& pat, int idx, int row, int col, int total);

// Build preview Slice objects from raw rectangles (used by Grid / Auto preview).
void buildPreviewSlices(std::vector<Slice>& out, const std::vector<Rectangle>& rects);

#endif
