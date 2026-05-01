#ifndef SIRIUS_INSPECTOR_H
#define SIRIUS_INSPECTOR_H

#include <vector>
#include "model/Slice.h"

class Editor;

class Inspector {
public:
    Inspector();
    ~Inspector();

    void draw(Editor& editor);

private:
    bool wasActive;
    bool hasSnapshot;
    std::vector<Slice> snapshot;
};

#endif
