#ifndef SIRIUS_STATUS_BAR_H
#define SIRIUS_STATUS_BAR_H

class Editor;

class StatusBar {
public:
    StatusBar();
    ~StatusBar();

    void draw(Editor& editor);
};

#endif
