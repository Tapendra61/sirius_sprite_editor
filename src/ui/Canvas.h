#ifndef SIRIUS_CANVAS_H
#define SIRIUS_CANVAS_H

#include "raylib.h"

class Editor;

class Canvas {
public:
    Canvas();
    ~Canvas();

    void draw(Editor& editor);

private:
    RenderTexture2D rt;
    int rtWidth;
    int rtHeight;
};

#endif
