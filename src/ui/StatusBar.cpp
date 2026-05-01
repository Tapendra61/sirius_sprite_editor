#include "ui/StatusBar.h"

#include "app/Editor.h"
#include "util/Coords.h"
#include "imgui.h"
#include "imgui_internal.h"

StatusBar::StatusBar() {
}

StatusBar::~StatusBar() {
}

static const ImVec4 INK     = ImVec4(0.910f, 0.902f, 0.941f, 1.0f);  // #E8E6F0
static const ImVec4 INK_2   = ImVec4(0.690f, 0.671f, 0.741f, 1.0f);  // #B0ABBD
static const ImVec4 INK_3   = ImVec4(0.478f, 0.455f, 0.565f, 1.0f);  // #7A7490
static const ImVec4 PIVOT   = ImVec4(0.961f, 0.620f, 0.420f, 1.0f);  // #F59E6B

static void statusSep() {
    ImGui::SameLine(0.0f, 6.0f);
    ImGui::TextColored(INK_3, "\xc2\xb7");  // middle dot ·
    ImGui::SameLine(0.0f, 6.0f);
}

void StatusBar::draw(Editor& editor) {
    float h = ImGui::GetFrameHeight();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 4.0f));
    bool open = ImGui::BeginViewportSideBar("##StatusBar", ImGui::GetMainViewport(),
                                             ImGuiDir_Down, h, flags);
    ImGui::PopStyleVar();

    if (!open) {
        ImGui::End();
        return;
    }

    // Zoom
    ImGui::TextColored(INK_2, "Zoom");
    ImGui::SameLine(0.0f, 4.0f);
    int zoomPct = (int)(editor.view.camera.zoom * 100.0f);
    ImGui::TextColored(INK, "%d%%", zoomPct);

    statusSep();

    // X / Y — always shown (relative to image origin in canvas-space coords)
    Vector2 mouseImg = ScreenToImage(GetMousePosition(), editor.view);
    int x = (int)mouseImg.x;
    int y = (int)mouseImg.y;

    ImGui::TextColored(INK_2, "X");
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::TextColored(INK, "%4d", x);

    ImGui::SameLine(0.0f, 8.0f);
    ImGui::TextColored(INK_2, "Y");
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::TextColored(INK, "%4d", y);

    statusSep();

    // Pixel color swatch — only when image loaded AND cursor in image bounds
    if (editor.project.isImageLoaded()) {
        int iw = editor.project.image.width;
        int ih = editor.project.image.height;
        bool inBounds = (x >= 0 && y >= 0 && x < iw && y < ih);

        if (inBounds) {
            Color c = GetImageColor(editor.project.image, x, y);
            ImVec4 swatch = ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 1.0f);
            ImGui::ColorButton("##px-swatch", swatch,
                ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder |
                ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoPicker,
                ImVec2(10.0f, 10.0f));
            ImGui::SameLine(0.0f, 6.0f);
            ImGui::TextColored(INK, "rgba(%d,%d,%d,%d)", c.r, c.g, c.b, c.a);
        } else {
            ImGui::TextColored(INK_3, "rgba(   -)");
        }
    } else {
        ImGui::TextColored(INK_3, "no image");
    }

    statusSep();

    // Slice count + selection
    int total = editor.project.slices.count();
    int selCount = editor.project.slices.selectionCount();
    ImGui::TextColored(INK, "%d", total);
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::TextColored(INK_2, "slices");

    ImGui::SameLine(0.0f, 8.0f);
    ImGui::TextColored(INK_3, "\xc2\xb7");
    ImGui::SameLine(0.0f, 8.0f);

    ImGui::TextColored(INK, "%d", selCount);
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::TextColored(INK_2, "selected");

    // Dirty indicator (right-aligned)
    if (editor.project.isDirty()) {
        const char* dot = "\xe2\x97\x8f";  // ●
        ImVec2 dotSize = ImGui::CalcTextSize(dot);
        float regionRight = ImGui::GetWindowContentRegionMax().x;
        ImGui::SameLine(regionRight - dotSize.x);
        ImGui::TextColored(PIVOT, "%s", dot);
    }

    ImGui::End();
}
