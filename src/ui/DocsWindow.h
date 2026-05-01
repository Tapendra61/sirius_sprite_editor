#ifndef SIRIUS_DOCS_WINDOW_H
#define SIRIUS_DOCS_WINDOW_H

class DocsWindow {
public:
    DocsWindow();
    ~DocsWindow();

    void show();
    void draw();

    bool visible;

private:
    int selectedSection;
};

#endif
