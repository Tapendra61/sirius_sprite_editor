#ifndef SIRIUS_EDITOR_H
#define SIRIUS_EDITOR_H

#include <vector>
#include "raylib.h"
#include "app/Keybindings.h"
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
#include "ui/KeybindingsModal.h"
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
    int hoveredSliceId;
};

class Editor {
public:
    Editor();
    ~Editor();

    void update();
    void render();

    void openProject();
    void saveProject();
    void saveProjectAs();

    void zoomTo(float zoom);
    void zoomFit();
    void zoomBy(float factor);

    Project project;
    CanvasView view;
    CommandStack commands;
    DragState drag;
    Keybindings keybindings;
    bool shouldExit;
    bool resetLayoutRequested;

    MainMenu mainMenu;
    Toolbar toolbar;
    Canvas canvas;
    Inspector inspector;
    SliceList sliceList;
    StatusBar statusBar;
    GridSliceModal   gridModal;
    AutoSliceModal   autoModal;
    ExportModal      exportModal;
    KeybindingsModal keybindingsModal;

    Texture2D iconCmd;
    Texture2D iconShift;
    Texture2D iconToolSelect;
    Texture2D iconToolMove;
    Texture2D iconToolRect;
};

#endif
