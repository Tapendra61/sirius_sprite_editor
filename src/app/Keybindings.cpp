#include "app/Keybindings.h"

#include <filesystem>
#include <fstream>
#include <system_error>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static const char* CONFIG_PATH = "keybindings.json";
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

    // Atomic-ish write: tmp + rename
    std::string tmp = std::string(CONFIG_PATH) + ".tmp";
    {
        std::ofstream out(tmp);
        if (!out.is_open()) return false;
        out << j.dump(2);
        if (out.fail()) return false;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, CONFIG_PATH, ec);
    if (ec) {
        std::filesystem::remove(tmp, ec);
        return false;
    }
    return true;
}

bool Keybindings::load() {
    std::ifstream in(CONFIG_PATH);
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
