#ifndef SIRIUS_TOOLBAR_H
#define SIRIUS_TOOLBAR_H

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

    void draw(Editor& editor);

    ToolMode mode;
};

#endif
