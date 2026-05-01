#include "ui/CanvasOptionsPanel.h"

#include "app/Editor.h"
#include "imgui.h"

CanvasOptionsPanel::CanvasOptionsPanel() : visible(false) {
}

CanvasOptionsPanel::~CanvasOptionsPanel() {
}

void CanvasOptionsPanel::show() {
    visible = true;
}

static const ImVec4 INK_3 = ImVec4(0.478f, 0.455f, 0.565f, 1.0f);

static void colorEdit(const char* label, int rgb[3]) {
    float c[3] = { rgb[0] / 255.0f, rgb[1] / 255.0f, rgb[2] / 255.0f };
    if (ImGui::ColorEdit3(label, c, ImGuiColorEditFlags_NoInputs)) {
        rgb[0] = (int)(c[0] * 255.0f + 0.5f);
        rgb[1] = (int)(c[1] * 255.0f + 0.5f);
        rgb[2] = (int)(c[2] * 255.0f + 0.5f);
        if (rgb[0] < 0) rgb[0] = 0; if (rgb[0] > 255) rgb[0] = 255;
        if (rgb[1] < 0) rgb[1] = 0; if (rgb[1] > 255) rgb[1] = 255;
        if (rgb[2] < 0) rgb[2] = 0; if (rgb[2] > 255) rgb[2] = 255;
    }
}

void CanvasOptionsPanel::draw(Editor& editor) {
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking
                           | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_AlwaysAutoResize;

    if (!ImGui::Begin("Canvas Options", &visible, flags)) {
        ImGui::End();
        return;
    }

    CanvasOptions& opt = editor.canvasOptions;

    // ---- Checker Grid ----
    if (ImGui::CollapsingHeader("Checker Grid", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show checker grid", &opt.checkerVisible);

        ImGui::PushItemWidth(140.0f);
        ImGui::SliderInt("Cell size", &opt.checkerSize, 2, 128);
        ImGui::PopItemWidth();

        colorEdit("Light##chk", opt.checkerLight);
        ImGui::SameLine(0.0f, 16.0f);
        colorEdit("Dark##chk", opt.checkerDark);

        if (ImGui::Button("Reset Checker")) {
            opt.checkerSize    = 16;
            opt.checkerLight[0] = 31; opt.checkerLight[1] = 31; opt.checkerLight[2] = 46;
            opt.checkerDark[0]  = 24; opt.checkerDark[1]  = 24; opt.checkerDark[2]  = 38;
        }
    }

    ImGui::Spacing();

    // ---- Grid Snap ----
    if (ImGui::CollapsingHeader("Grid Snap", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable grid snap", &opt.gridSnapEnabled);
        ImGui::TextColored(INK_3,
            "Snaps slice creation / move / resize to step multiples.");
        ImGui::TextColored(INK_3,
            "Hold Cmd / Ctrl while dragging for whole-pixel snap.");

        if (!opt.gridSnapEnabled) ImGui::BeginDisabled();
        ImGui::PushItemWidth(120.0f);
        ImGui::DragInt("Step X", &opt.gridSnapX, 1.0f, 1, 1024);
        ImGui::DragInt("Step Y", &opt.gridSnapY, 1.0f, 1, 1024);
        ImGui::PopItemWidth();
        if (opt.gridSnapX < 1) opt.gridSnapX = 1;
        if (opt.gridSnapY < 1) opt.gridSnapY = 1;
        if (!opt.gridSnapEnabled) ImGui::EndDisabled();

        if (ImGui::Button("Match Checker")) {
            opt.gridSnapX = opt.checkerSize;
            opt.gridSnapY = opt.checkerSize;
        }
    }

    ImGui::Spacing();

    // ---- Pixel Grid ----
    if (ImGui::CollapsingHeader("Pixel Grid", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show pixel grid", &opt.pixelGridVisible);
        ImGui::TextColored(INK_3,
            "Hairline grid at every integer pixel.");

        if (!opt.pixelGridVisible) ImGui::BeginDisabled();
        ImGui::PushItemWidth(140.0f);
        ImGui::DragFloat("Min zoom to show", &opt.pixelGridZoomMin, 0.1f, 1.0f, 32.0f, "%.1fx");
        ImGui::PopItemWidth();
        if (opt.pixelGridZoomMin < 1.0f)  opt.pixelGridZoomMin = 1.0f;
        if (opt.pixelGridZoomMin > 32.0f) opt.pixelGridZoomMin = 32.0f;
        if (!opt.pixelGridVisible) ImGui::EndDisabled();
    }

    ImGui::End();
}
