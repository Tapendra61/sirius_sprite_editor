#include "ui/Toolbar.h"

#include "app/Editor.h"
#include "imgui.h"

Toolbar::Toolbar() : mode(ToolMode::Select) {
}

Toolbar::~Toolbar() {
}

static const ImVec4 ACCENT_SEL      = ImVec4(0.506f, 0.549f, 0.973f, 1.0f);  // #818CF8
static const ImVec4 ACCENT_SEL_SOFT = ImVec4(0.506f, 0.549f, 0.973f, 0.15f);
static const ImVec4 INK_2           = ImVec4(0.690f, 0.671f, 0.741f, 1.0f);
static const ImVec4 INK_3           = ImVec4(0.478f, 0.455f, 0.565f, 1.0f);

static bool modeButton(const char* label, const char* shortcut, bool active) {
    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ACCENT_SEL_SOFT);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ACCENT_SEL_SOFT);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ACCENT_SEL_SOFT);
        ImGui::PushStyleColor(ImGuiCol_Text,          ACCENT_SEL);
        ImGui::PushStyleColor(ImGuiCol_Border,        ACCENT_SEL);
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "%s  %s", label, shortcut);
    bool clicked = ImGui::Button(buf);
    if (active) {
        ImGui::PopStyleColor(5);
    }
    return clicked;
}

static void verticalSep() {
    ImGui::SameLine(0.0f, 8.0f);
    float h = ImGui::GetFrameHeight() * 0.7f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos.y += (ImGui::GetFrameHeight() - h) * 0.5f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 col = ImGui::GetColorU32(ImGuiCol_Border);
    dl->AddLine(pos, ImVec2(pos.x, pos.y + h), col, 1.0f);
    ImGui::Dummy(ImVec2(1.0f, ImGui::GetFrameHeight()));
    ImGui::SameLine(0.0f, 8.0f);
}

void Toolbar::draw(Editor& editor) {
    ImGui::Begin("Toolbar");

    // Mode group
    if (modeButton("Select", "V", mode == ToolMode::Select)) {
        mode = ToolMode::Select;
    }
    ImGui::SameLine();
    if (modeButton("Move", "H", mode == ToolMode::Move)) {
        mode = ToolMode::Move;
    }
    ImGui::SameLine();
    if (modeButton("Rect", "R", mode == ToolMode::Rectangle)) {
        mode = ToolMode::Rectangle;
    }

    verticalSep();

    // Action group
    bool hasImage = editor.project.isImageLoaded();
    if (!hasImage) ImGui::BeginDisabled();
    if (ImGui::Button("Grid Slice  \xe2\x8c\x98G")) {
        editor.gridModal.open();
    }
    ImGui::SameLine();
    if (ImGui::Button("Auto Slice  \xe2\x8c\x98\xe2\x87\xa7" "A")) {
        editor.autoModal.open();
    }
    if (!hasImage) ImGui::EndDisabled();

    verticalSep();

    // Export (Phase 7 placeholder)
    ImGui::BeginDisabled();
    ImGui::Button("Export  \xe2\x8c\x98" "E");
    ImGui::EndDisabled();

    ImGui::End();
}
