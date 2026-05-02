#include "ui/StatusBar.h"

#include <cstdio>
#include <filesystem>
#include "app/Editor.h"
#include "ui/Palette.h"
#include "ui/Theme.h"
#include "util/Coords.h"
#include "imgui.h"
#include "imgui_internal.h"

StatusBar::StatusBar() {
}

StatusBar::~StatusBar() {
}

using pal::INK;
using pal::INK_2;
using pal::INK_3;
using pal::PIVOT;

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

    if (g_FontMono) ImGui::PushFont(g_FontMono);

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

    // Project filename + dirty marker (right-aligned)
    char title[512];
    const char* dirty = editor.project.isDirty() ? "*" : "";
    if (editor.project.projectPath.empty()) {
        std::snprintf(title, sizeof(title), "untitled%s", dirty);
    } else {
        std::filesystem::path p(editor.project.projectPath);
        std::snprintf(title, sizeof(title), "%s%s",
                      p.filename().string().c_str(), dirty);
    }
    ImVec2 titleSize = ImGui::CalcTextSize(title);
    float regionRight = ImGui::GetWindowContentRegionMax().x;
    ImGui::SameLine(regionRight - titleSize.x);
    ImGui::TextColored(editor.project.isDirty() ? PIVOT : INK_2, "%s", title);

    if (g_FontMono) ImGui::PopFont();

    ImGui::End();
}
