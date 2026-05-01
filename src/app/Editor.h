#ifndef SIRIUS_EDITOR_H
#define SIRIUS_EDITOR_H

#include <vector>
#include "raylib.h"
#include "app/Project.h"
#include "commands/CommandStack.h"
#include "model/Slice.h"
#include "ui/MainMenu.h"
#include "ui/Toolbar.h"
#include "ui/Canvas.h"
#include "ui/Inspector.h"
#include "ui/SliceList.h"
#include "ui/StatusBar.h"
#include "ui/GridSliceModal.h"
#include "ui/AutoSliceModal.h"
#include "ui/ExportModal.h"
#include "util/Coords.h"
#include "util/HitTest.h"

enum class DragMode {
    Idle,
    Creating,
    Marquee,
    Moving,
    Resizing
};

struct DragState {
    DragMode mode;
    Vector2 startImg;
    int activeSliceId;
    HandleId handle;
    std::vector<Slice> snapshot;
    Vector2 marqueeEnd;
    bool marqueeAdditive;
    bool dragHappened;
    int cycleNextId;
};

class Editor {
public:
    Editor();
    ~Editor();

    void update();
    void render();

    Project project;
    CanvasView view;
    CommandStack commands;
    DragState drag;
    bool shouldExit;
    bool resetLayoutRequested;

    MainMenu mainMenu;
    Toolbar toolbar;
    Canvas canvas;
    Inspector inspector;
    SliceList sliceList;
    StatusBar statusBar;
    GridSliceModal gridModal;
    AutoSliceModal autoModal;
    ExportModal    exportModal;
};

#endif
