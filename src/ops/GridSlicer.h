#ifndef SIRIUS_GRID_SLICER_H
#define SIRIUS_GRID_SLICER_H

#include <vector>
#include <string>
#include "raylib.h"

struct GridParams {
    int cellWidth;
    int cellHeight;
    int offsetX;
    int offsetY;
    int paddingX;
    int paddingY;
    std::string namePrefix;
};

std::vector<Rectangle> sliceGridBySize(int imageWidth, int imageHeight, const GridParams& params);

std::vector<Rectangle> sliceGridByCount(int imageWidth, int imageHeight,
                                        int columns, int rows,
                                        int offsetX, int offsetY,
                                        int paddingX, int paddingY);

#endif
