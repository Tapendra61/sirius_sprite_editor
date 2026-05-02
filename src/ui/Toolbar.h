#ifndef SIRIUS_TOOLBAR_H
#define SIRIUS_TOOLBAR_H

#include "raylib.h"

class Editor;

enum class ToolMode {
    Select,
    Move,
    Rectangle
};

class Toolbar {
public:
    Toolbar();
    ~Toolbar();

    void loadIcons();    // Call after InitWindow.
    void unloadIcons();  // Call before CloseWindow.
    void draw(Editor& editor);

    ToolMode mode;

    Texture2D iconCmd;
    Texture2D iconShift;
    Texture2D iconToolSelect;
    Texture2D iconToolMove;
    Texture2D iconToolRect;
};

#endif
