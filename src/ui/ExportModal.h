#ifndef SIRIUS_EXPORT_MODAL_H
#define SIRIUS_EXPORT_MODAL_H

#include <string>

class Editor;

enum class ExportFormat {
    AtlasJson = 0,
    Pngs      = 1,
    Csv       = 2
};

class ExportModal {
public:
    ExportModal();
    ~ExportModal();

    void open();
    void draw(Editor& editor);

    bool showing;

private:
    bool openRequested;
    int  formatIdx;       // ExportFormat as int for ImGui radio compatibility
    char pathBuf[1024];
    bool selectedOnly;

    bool lastResultShown;  // false until first export attempt
    bool lastResultOk;
    std::string lastResultMsg;
};

#endif
