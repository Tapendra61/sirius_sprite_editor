#ifndef SIRIUS_CANVAS_OPTIONS_H
#define SIRIUS_CANVAS_OPTIONS_H

struct CanvasOptions {
    // Checker grid (background pattern shown behind the canvas / image).
    int   checkerSize     = 16;
    bool  checkerVisible  = true;
    int   checkerLight[3] = { 31, 31, 46 };  // #1F1F2E
    int   checkerDark[3]  = { 24, 24, 38 };  // #181826

    // Grid snap (snap rectangles to multiples of step on create / move / resize).
    bool  gridSnapEnabled = false;
    int   gridSnapX       = 16;
    int   gridSnapY       = 16;

    // Pixel grid overlay (1-px lines at every integer pixel boundary).
    bool  pixelGridVisible  = false;
    float pixelGridZoomMin  = 4.0f;  // Only render when zoom >= this.

    // Persistence (canvas_options.json next to the executable).
    bool save() const;
    bool load();
};

#endif
