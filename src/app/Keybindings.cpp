#include "app/Keybindings.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include "util/ConfigDir.h"
#include "util/JsonFile.h"

using json = nlohmann::json;

static const char* CONFIG_NAME = "keybindings.json";
static const int   CONFIG_VERSION = 1;

static const ActionDef ACTION_TABLE[] = {
    /* File */
    { "open_project",    "Open Project",   "File",  ImGuiMod_Ctrl | ImGuiKey_O },
    { "save_project",    "Save",           "File",  ImGuiMod_Ctrl | ImGuiKey_S },
    { "save_project_as", "Save As",        "File",  ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S },
    { "open_image",      "Open Image",     "File",  ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_I },
    { "quit",            "Quit",           "File",  ImGuiMod_Ctrl | ImGuiKey_Q },

    /* Edit */
    { "undo",            "Undo",           "Edit",  ImGuiMod_Ctrl | ImGuiKey_Z },
    { "redo",            "Redo",           "Edit",  ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z },
    { "select_all",      "Select All",     "Edit",  ImGuiMod_Ctrl | ImGuiKey_A },
    { "deselect",        "Deselect",       "Edit",  ImGuiKey_Escape },
    { "duplicate",       "Duplicate",      "Edit",  ImGuiMod_Ctrl | ImGuiKey_D },
    { "delete",          "Delete",         "Edit",  ImGuiKey_Delete },

    /* View */
    { "zoom_in",         "Zoom In",        "View",  ImGuiKey_Equal },
    { "zoom_out",        "Zoom Out",       "View",  ImGuiKey_Minus },
    { "zoom_fit",        "Zoom Fit",       "View",  ImGuiKey_F },
    { "zoom_actual",     "Zoom 100%",      "View",  ImGuiKey_1 },

    /* Tools */
    { "tool_select",     "Select Tool",    "Tools", ImGuiKey_V },
    { "tool_move",       "Move Tool",      "Tools", ImGuiKey_H },
    { "tool_rect",       "Rectangle Tool", "Tools", ImGuiKey_R },

    /* Slice */
    { "grid_slice",      "Grid Slice",     "Slice", ImGuiMod_Ctrl | ImGuiKey_G },
    { "auto_slice",      "Auto Slice",     "Slice", ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_A },
    { "export",          "Export",         "Slice", ImGuiMod_Ctrl | ImGuiKey_E },
};

static_assert(sizeof(ACTION_TABLE) / sizeof(ACTION_TABLE[0]) == (size_t)Action::Count,
              "ACTION_TABLE size must match Action::Count");

Keybindings::Keybindings() {
    resetAll();
    load();  // overrides defaults if config file exists
}

bool Keybindings::isPressed(Action a) const {
    int idx = (int)a;
    if (idx < 0 || idx >= (int)Action::Count) return false;
    ImGuiKeyChord chord = chords_[idx];
    if (chord == ImGuiKey_None) return false;

    if (ImGui::IsKeyChordPressed(chord)) return true;

    // Apple keyboards label their main keyboard key "delete" but it actually
    // sends Backspace — true Forward-Delete is Fn+delete or only present on
    // extended keyboards. When the user has the default Delete chord, accept
    // Backspace too so the obvious key works on Mac. (No-op on Windows/Linux:
    // pressing Backspace there is rare for "delete selection" and users who
    // remap Action::Delete to something else don't get this fallback.)
    if (a == Action::Delete && chord == ImGuiKey_Delete) {
        if (ImGui::IsKeyChordPressed(ImGuiKey_Backspace)) return true;
    }
    return false;
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
    save();
}

void Keybindings::resetToDefault(Action a) {
    setChord(a, defaultChordOf(a));
}

void Keybindings::resetAll() {
    for (int i = 0; i < (int)Action::Count; ++i) {
        chords_[i] = ACTION_TABLE[i].defaultChord;
    }
    save();
}

bool Keybindings::save() const {
    json j;
    j["version"] = CONFIG_VERSION;
    j["bindings"] = json::object();
    for (int i = 0; i < (int)Action::Count; ++i) {
        j["bindings"][ACTION_TABLE[i].id] = (int)chords_[i];
    }

    return writeFileAtomic(configPath(CONFIG_NAME), j.dump(2)).empty();
}

bool Keybindings::load() {
    std::ifstream in(configPath(CONFIG_NAME));
    if (!in.is_open()) return false;

    json j = json::parse(in, nullptr, false);
    if (j.is_discarded() || !j.is_object()) return false;
    if (!j.contains("bindings") || !j["bindings"].is_object()) return false;

    const json& bindings = j["bindings"];
    for (int i = 0; i < (int)Action::Count; ++i) {
        const char* id = ACTION_TABLE[i].id;
        if (bindings.contains(id) && bindings[id].is_number_integer()) {
            chords_[i] = (ImGuiKeyChord)(int)bindings[id];
        }
    }
    return true;
}

const char* Keybindings::idOf(Action a) {
    int idx = (int)a;
    if (idx < 0 || idx >= (int)Action::Count) return "?";
    return ACTION_TABLE[idx].id;
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
