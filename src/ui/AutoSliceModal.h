#ifndef SIRIUS_AUTO_SLICE_MODAL_H
#define SIRIUS_AUTO_SLICE_MODAL_H

#include <vector>
#include "model/Slice.h"
#include "raylib.h"

class Editor;

class AutoSliceModal {
public:
    AutoSliceModal();
    ~AutoSliceModal();

    void open();
    void draw(Editor& editor);

    bool showing;
    std::vector<Slice> preview;

private:
    bool openRequested;

    int alphaThreshold;
    int minSize;
    int mergeDistance;
    int padding;
    bool eightConnected;
    bool replaceAll;

    bool needsRecompute;
    std::vector<Rectangle> cachedRects;
};

#endif
