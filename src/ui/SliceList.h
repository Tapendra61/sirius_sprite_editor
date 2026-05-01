#ifndef SIRIUS_SLICE_LIST_H
#define SIRIUS_SLICE_LIST_H

#include <string>

class Editor;

class SliceList {
public:
    SliceList();
    ~SliceList();

    void draw(Editor& editor);

private:
    std::string searchText;
};

#endif
