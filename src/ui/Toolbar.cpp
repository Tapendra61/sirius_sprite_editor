#include "ui/Toolbar.h"

#include "app/Editor.h"
#include "imgui.h"
#include "imgui_internal.h"

Toolbar::Toolbar() : mode(ToolMode::Select) {
}

Toolbar::~Toolbar() {
}

static const ImVec4 ACCENT_SEL      = ImVec4(0.506f, 0.549f, 0.973f, 1.0f);
static const ImVec4 ACCENT_SEL_SOFT = ImVec4(0.506f, 0.549f, 0.973f, 0.15f);
static const ImVec4 BG_TOOLBAR      = ImVec4( 26.0f / 255.0f,  26.0f / 255.0f,  38.0f / 255.0f, 1.0f);

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
    float h = ImGui::GetFrameHeight() + 14.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, BG_TOOLBAR);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 4.0f));
    bool open = ImGui::BeginViewportSideBar("##Toolbar", ImGui::GetMainViewport(),
                                              ImGuiDir_Up, h, flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    if (!open) {
        ImGui::End();
        return;
    }

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

    bool hasSlices = editor.project.slices.count() > 0;
    if (!hasSlices) ImGui::BeginDisabled();
    if (ImGui::Button("Export  \xe2\x8c\x98" "E")) {
        editor.exportModal.open();
    }
    if (!hasSlices) ImGui::EndDisabled();

    ImGui::End();
}
