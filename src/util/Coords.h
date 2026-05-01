#ifndef SIRIUS_COORDS_H
#define SIRIUS_COORDS_H

#include "raylib.h"

struct CanvasView {
    Camera2D camera;
    Rectangle panelBounds;
};

inline Vector2 ScreenToImage(Vector2 mouseScreen, const CanvasView& v) {
    Vector2 inPanel;
    inPanel.x = mouseScreen.x - v.panelBounds.x;
    inPanel.y = mouseScreen.y - v.panelBounds.y;
    return GetScreenToWorld2D(inPanel, v.camera);
}

inline Vector2 ImageToScreen(Vector2 imagePos, const CanvasView& v) {
    Vector2 inPanel = GetWorldToScreen2D(imagePos, v.camera);
    Vector2 result;
    result.x = inPanel.x + v.panelBounds.x;
    result.y = inPanel.y + v.panelBounds.y;
    return result;
}

inline Rectangle ImageRectToScreen(Rectangle r, const CanvasView& v) {
    Vector2 imgTL;
    imgTL.x = r.x;
    imgTL.y = r.y;
    Vector2 tl = ImageToScreen(imgTL, v);
    Rectangle out;
    out.x = tl.x;
    out.y = tl.y;
    out.width = r.width * v.camera.zoom;
    out.height = r.height * v.camera.zoom;
    return out;
}

#endif
