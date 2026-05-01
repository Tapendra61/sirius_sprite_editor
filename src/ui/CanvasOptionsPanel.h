#ifndef SIRIUS_CANVAS_OPTIONS_PANEL_H
#define SIRIUS_CANVAS_OPTIONS_PANEL_H

class Editor;

class CanvasOptionsPanel {
public:
    CanvasOptionsPanel();
    ~CanvasOptionsPanel();

    void show();
    void draw(Editor& editor);

    bool visible;
};

#endif
