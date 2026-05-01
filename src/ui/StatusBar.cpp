#include "ui/StatusBar.h"

#include "app/Editor.h"
#include "util/Coords.h"
#include "imgui.h"
#include "imgui_internal.h"

StatusBar::StatusBar() {
}

StatusBar::~StatusBar() {
}

void StatusBar::draw(Editor& editor) {
    float h = ImGui::GetFrameHeight();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::BeginViewportSideBar("##StatusBar", ImGui::GetMainViewport(),
                                    ImGuiDir_Down, h, flags)) {
        int zoomPct = (int)(editor.view.camera.zoom * 100.0f);
        ImGui::Text("Zoom: %d%%", zoomPct);

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        if (editor.project.isImageLoaded()) {
            Vector2 mouseImg = ScreenToImage(GetMousePosition(), editor.view);
            int x = (int)mouseImg.x;
            int y = (int)mouseImg.y;
            int iw = editor.project.image.width;
            int ih = editor.project.image.height;

            if (x >= 0 && y >= 0 && x < iw && y < ih) {
                Color c = GetImageColor(editor.project.image, x, y);
                ImGui::Text("X: %4d  Y: %4d", x, y);
                ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();
                ImGui::Text("RGBA: (%3d, %3d, %3d, %3d)", c.r, c.g, c.b, c.a);
            } else {
                ImGui::Text("X:    -  Y:    -");
                ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();
                ImGui::Text("RGBA:    -");
            }
        } else {
            ImGui::Text("(no image)");
        }

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();
        ImGui::Text("%d slices, %d selected",
                    editor.project.slices.count(),
                    editor.project.slices.selectionCount());
    }
    ImGui::End();
}
