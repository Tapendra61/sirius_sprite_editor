#ifndef SIRIUS_AUTO_SLICER_H
#define SIRIUS_AUTO_SLICER_H

#include <vector>
#include "raylib.h"

struct AutoSliceParams {
    int alphaThreshold;
    int minSize;
    int mergeDistance;
    int padding;
    bool eightConnected;
};

std::vector<Rectangle> autoSlice(const Image& image, const AutoSliceParams& params);

#endif
