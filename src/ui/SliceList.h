#ifndef SIRIUS_SLICE_LIST_H
#define SIRIUS_SLICE_LIST_H

class Editor;

class SliceList {
public:
    SliceList();
    ~SliceList();

    void draw(Editor& editor);

private:
    int  renamingId = 0;
    char renameBuffer[128] = {0};
    bool renameFocusPending = false;
    int  contextMenuId = 0;

    char searchBuf[128] = {0};
};

#endif
