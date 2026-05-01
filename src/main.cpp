#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

#include "app/Editor.h"
#include "ui/Theme.h"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Sirius Sprite Editor");
    SetTargetFPS(60);

    rlImGuiBeginInitImGui();
    LoadFonts();
    rlImGuiEndInitImGui();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ApplyDarkTheme();

    {
        Editor editor;

        while (!WindowShouldClose() && !editor.shouldExit) {
            BeginDrawing();
            ClearBackground(DARKGRAY);

            rlImGuiBegin();
            editor.update();
            editor.render();
            rlImGuiEnd();

            EndDrawing();
        }
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
