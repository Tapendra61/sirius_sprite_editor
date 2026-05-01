#ifndef SIRIUS_INSPECTOR_H
#define SIRIUS_INSPECTOR_H

class Editor;

class Inspector {
public:
    Inspector();
    ~Inspector();

    void draw(Editor& editor);
};

#endif
