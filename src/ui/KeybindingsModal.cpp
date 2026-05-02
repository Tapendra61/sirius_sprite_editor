#include "ui/KeybindingsModal.h"

#include <cstdio>
#include <cstring>
#include "app/Editor.h"
#include "app/Keybindings.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/Palette.h"

KeybindingsModal::KeybindingsModal()
    : showing(false),
      openRequested(false),
      recordingActionIdx(-1) {
}

KeybindingsModal::~KeybindingsModal() {
}

void KeybindingsModal::open() {
    openRequested = true;
    recordingActionIdx = -1;
}

using pal::INK_3;

static ImGuiKey captureNonModKey() {
    for (int k = ImGuiKey_A; k <= ImGuiKey_Z; ++k) {
        if (ImGui::IsKeyPressed((ImGuiKey)k)) return (ImGuiKey)k;
    }
    for (int k = ImGuiKey_0; k <= ImGuiKey_9; ++k) {
        if (ImGui::IsKeyPressed((ImGuiKey)k)) return (ImGuiKey)k;
    }
    for (int k = ImGuiKey_F1; k <= ImGuiKey_F12; ++k) {
        if (ImGui::IsKeyPressed((ImGuiKey)k)) return (ImGuiKey)k;
    }
    static const ImGuiKey specials[] = {
        ImGuiKey_Tab, ImGuiKey_Space, ImGuiKey_Enter, ImGuiKey_Backspace,
        ImGuiKey_Delete, ImGuiKey_Insert, ImGuiKey_Home, ImGuiKey_End,
        ImGuiKey_PageUp, ImGuiKey_PageDown,
        ImGuiKey_LeftArrow, ImGuiKey_RightArrow,
        ImGuiKey_UpArrow, ImGuiKey_DownArrow,
        ImGuiKey_Apostrophe, ImGuiKey_Comma, ImGuiKey_Minus,
        ImGuiKey_Period, ImGuiKey_Slash,
        ImGuiKey_Semicolon, ImGuiKey_Equal,
        ImGuiKey_LeftBracket, ImGuiKey_Backslash, ImGuiKey_RightBracket,
        ImGuiKey_GraveAccent,
    };
    for (size_t i = 0; i < sizeof(specials) / sizeof(specials[0]); ++i) {
        if (ImGui::IsKeyPressed(specials[i])) return specials[i];
    }
    return ImGuiKey_None;
}

static const char* CATEGORIES[] = { "File", "Edit", "View", "Tools", "Slice" };

void KeybindingsModal::draw(Editor& editor) {
    if (openRequested) {
        ImGui::OpenPopup("Keybindings");
        openRequested = false;
    }

    showing = ImGui::IsPopupOpen("Keybindings");

    if (!ImGui::BeginPopupModal("Keybindings", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!showing) recordingActionIdx = -1;
        return;
    }

    ImGui::TextColored(INK_3,
        "Click a chord to rebind. Esc to cancel a recording.");
    ImGui::Spacing();

    for (size_t catIdx = 0; catIdx < sizeof(CATEGORIES) / sizeof(CATEGORIES[0]); ++catIdx) {
        const char* cat = CATEGORIES[catIdx];

        if (!ImGui::CollapsingHeader(cat, ImGuiTreeNodeFlags_DefaultOpen)) continue;

        char tableId[32];
        std::snprintf(tableId, sizeof(tableId), "##bindings_%zu", catIdx);

        if (!ImGui::BeginTable(tableId, 3, ImGuiTableFlags_None)) continue;

        ImGui::TableSetupColumn("##name",  ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##chord", ImGuiTableColumnFlags_WidthFixed, 180.0f);
        ImGui::TableSetupColumn("##reset", ImGuiTableColumnFlags_WidthFixed, 30.0f);

        for (int i = 0; i < Keybindings::count(); ++i) {
            Action a = (Action)i;
            if (std::strcmp(Keybindings::categoryOf(a), cat) != 0) continue;

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", Keybindings::nameOf(a));

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            if (recordingActionIdx == i) {
                ImGui::PushStyleColor(ImGuiCol_Button,
                    ImVec4(0.961f, 0.620f, 0.420f, 0.85f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                ImGui::Button("Press a key\xe2\x80\xa6", ImVec2(-1.0f, 0.0f));
                ImGui::PopStyleColor(2);
            } else {
                ImGuiKeyChord chord = editor.keybindings.getChord(a);
                const char* chordText;
                if (chord == ImGuiKey_None) chordText = "(unbound)";
                else chordText = ImGui::GetKeyChordName(chord);

                if (ImGui::Button(chordText, ImVec2(-1.0f, 0.0f))) {
                    recordingActionIdx = i;
                }
            }

            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button("\xe2\x86\xba", ImVec2(-1.0f, 0.0f))) {
                editor.keybindings.resetToDefault(a);
                if (recordingActionIdx == i) recordingActionIdx = -1;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Reset to default");
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    // Recording capture
    if (recordingActionIdx >= 0) {
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            recordingActionIdx = -1;
        } else {
            ImGuiKey key = captureNonModKey();
            if (key != ImGuiKey_None) {
                ImGuiIO& io = ImGui::GetIO();
                ImGuiKeyChord chord = key;
                if (io.KeyCtrl)  chord |= ImGuiMod_Ctrl;
                if (io.KeyShift) chord |= ImGuiMod_Shift;
                if (io.KeyAlt)   chord |= ImGuiMod_Alt;
                if (io.KeySuper) chord |= ImGuiMod_Super;

                editor.keybindings.setChord((Action)recordingActionIdx, chord);
                recordingActionIdx = -1;
            }
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Reset All to Defaults")) {
        editor.keybindings.resetAll();
        recordingActionIdx = -1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Close")) {
        recordingActionIdx = -1;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
