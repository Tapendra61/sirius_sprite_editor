#ifndef SIRIUS_GRID_SLICE_MODAL_H
#define SIRIUS_GRID_SLICE_MODAL_H

#include <vector>
#include "model/Slice.h"

class Editor;

enum class GridMode {
    BySize,
    ByCount
};

class GridSliceModal {
public:
    GridSliceModal();
    ~GridSliceModal();

    void open();
    void draw(Editor& editor);

    bool showing;
    std::vector<Slice> preview;

private:
    bool openRequested;

    GridMode mode;
    int cellWidth;
    int cellHeight;
    int columns;
    int rows;
    int offsetX;
    int offsetY;
    int paddingX;
    int paddingY;
    bool replaceAll;
};

#endif
