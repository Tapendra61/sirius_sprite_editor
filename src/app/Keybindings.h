#ifndef SIRIUS_KEYBINDINGS_H
#define SIRIUS_KEYBINDINGS_H

#include "imgui.h"

enum class Action {
    OpenProject = 0,
    SaveProject,
    SaveProjectAs,
    OpenImage,
    Quit,

    Undo,
    Redo,
    SelectAll,
    Deselect,
    Duplicate,
    Delete,

    ZoomIn,
    ZoomOut,
    ZoomFit,
    ZoomActual,

    ToolSelect,
    ToolMove,
    ToolRect,

    GridSlice,
    AutoSlice,
    Export,

    Count
};

struct ActionDef {
    const char*    name;
    const char*    category;
    ImGuiKeyChord  defaultChord;
};

class Keybindings {
public:
    Keybindings();

    bool isPressed(Action a) const;
    ImGuiKeyChord getChord(Action a) const;
    void setChord(Action a, ImGuiKeyChord chord);
    void resetToDefault(Action a);
    void resetAll();

    static const char*    nameOf(Action a);
    static const char*    categoryOf(Action a);
    static ImGuiKeyChord  defaultChordOf(Action a);
    static int            count() { return (int)Action::Count; }

private:
    ImGuiKeyChord chords_[(int)Action::Count];
};

#endif
