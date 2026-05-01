#include "ops/GridSlicer.h"

std::vector<Rectangle> sliceGridBySize(int imageWidth, int imageHeight, const GridParams& params) {
    std::vector<Rectangle> result;
    if (params.cellWidth <= 0 || params.cellHeight <= 0) return result;

    int strideX = params.cellWidth + params.paddingX;
    int strideY = params.cellHeight + params.paddingY;
    if (strideX <= 0 || strideY <= 0) return result;

    for (int y = params.offsetY; y + params.cellHeight <= imageHeight; y += strideY) {
        for (int x = params.offsetX; x + params.cellWidth <= imageWidth; x += strideX) {
            Rectangle r;
            r.x = (float)x;
            r.y = (float)y;
            r.width = (float)params.cellWidth;
            r.height = (float)params.cellHeight;
            result.push_back(r);
        }
    }
    return result;
}

std::vector<Rectangle> sliceGridByCount(int imageWidth, int imageHeight,
                                        int columns, int rows,
                                        int offsetX, int offsetY,
                                        int paddingX, int paddingY) {
    std::vector<Rectangle> result;
    if (columns <= 0 || rows <= 0) return result;

    int availW = imageWidth  - offsetX - (columns - 1) * paddingX;
    int availH = imageHeight - offsetY - (rows    - 1) * paddingY;
    if (availW <= 0 || availH <= 0) return result;

    int cellW = availW / columns;
    int cellH = availH / rows;
    if (cellW <= 0 || cellH <= 0) return result;

    GridParams p;
    p.cellWidth  = cellW;
    p.cellHeight = cellH;
    p.offsetX    = offsetX;
    p.offsetY    = offsetY;
    p.paddingX   = paddingX;
    p.paddingY   = paddingY;

    return sliceGridBySize(imageWidth, imageHeight, p);
}
