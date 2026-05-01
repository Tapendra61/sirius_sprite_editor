#include "app/Keybindings.h"

static const ActionDef ACTION_TABLE[] = {
    /* File */
    { "Open Project",   "File",  ImGuiMod_Ctrl | ImGuiKey_O },
    { "Save",           "File",  ImGuiMod_Ctrl | ImGuiKey_S },
    { "Save As",        "File",  ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S },
    { "Open Image",     "File",  ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_I },
    { "Quit",           "File",  ImGuiMod_Ctrl | ImGuiKey_Q },

    /* Edit */
    { "Undo",           "Edit",  ImGuiMod_Ctrl | ImGuiKey_Z },
    { "Redo",           "Edit",  ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z },
    { "Select All",     "Edit",  ImGuiMod_Ctrl | ImGuiKey_A },
    { "Deselect",       "Edit",  ImGuiKey_Escape },
    { "Duplicate",      "Edit",  ImGuiMod_Ctrl | ImGuiKey_D },
    { "Delete",         "Edit",  ImGuiKey_Delete },

    /* View */
    { "Zoom In",        "View",  ImGuiKey_Equal },
    { "Zoom Out",       "View",  ImGuiKey_Minus },
    { "Zoom Fit",       "View",  ImGuiKey_F },
    { "Zoom 100%",      "View",  ImGuiKey_1 },

    /* Tools */
    { "Select Tool",    "Tools", ImGuiKey_V },
    { "Move Tool",      "Tools", ImGuiKey_H },
    { "Rectangle Tool", "Tools", ImGuiKey_R },

    /* Slice */
    { "Grid Slice",     "Slice", ImGuiMod_Ctrl | ImGuiKey_G },
    { "Auto Slice",     "Slice", ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_A },
    { "Export",         "Slice", ImGuiMod_Ctrl | ImGuiKey_E },
};

static_assert(sizeof(ACTION_TABLE) / sizeof(ACTION_TABLE[0]) == (size_t)Action::Count,
              "ACTION_TABLE size must match Action::Count");

Keybindings::Keybindings() {
    resetAll();
}

bool Keybindings::isPressed(Action a) const {
    int idx = (int)a;
    if (idx < 0 || idx >= (int)Action::Count) return false;
    ImGuiKeyChord chord = chords_[idx];
    if (chord == ImGuiKey_None) return false;
    return ImGui::IsKeyChordPressed(chord);
}

ImGuiKeyChord Keybindings::getChord(Action a) const {
    int idx = (int)a;
    if (idx < 0 || idx >= (int)Action::Count) return ImGuiKey_None;
    return chords_[idx];
}

void Keybindings::setChord(Action a, ImGuiKeyChord chord) {
    int idx = (int)a;
    if (idx < 0 || idx >= (int)Action::Count) return;
    chords_[idx] = chord;
}

void Keybindings::resetToDefault(Action a) {
    setChord(a, defaultChordOf(a));
}

void Keybindings::resetAll() {
    for (int i = 0; i < (int)Action::Count; ++i) {
        chords_[i] = ACTION_TABLE[i].defaultChord;
    }
}

const char* Keybindings::nameOf(Action a) {
    int idx = (int)a;
    if (idx < 0 || idx >= (int)Action::Count) return "?";
    return ACTION_TABLE[idx].name;
}

const char* Keybindings::categoryOf(Action a) {
    int idx = (int)a;
    if (idx < 0 || idx >= (int)Action::Count) return "?";
    return ACTION_TABLE[idx].category;
}

ImGuiKeyChord Keybindings::defaultChordOf(Action a) {
    int idx = (int)a;
    if (idx < 0 || idx >= (int)Action::Count) return ImGuiKey_None;
    return ACTION_TABLE[idx].defaultChord;
}
