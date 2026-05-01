#ifndef SIRIUS_CANVAS_DRAW_H
#define SIRIUS_CANVAS_DRAW_H

#include "raylib.h"

class Editor;

void drawCheckerboard(const Camera2D& camera, int panelW, int panelH);
void drawSliceOutlines(const Editor& editor);
void drawSliceBorders(const Editor& editor);
void drawSliceHandles(const Editor& editor);
void drawSlicePivots(const Editor& editor);
void drawMarquee(const Editor& editor);
void drawCreationPreview(const Editor& editor);
void drawGridPreview(const Editor& editor);
void drawAutoPreview(const Editor& editor);

#endif
