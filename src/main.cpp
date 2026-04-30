#include <iostream>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

int main() {
    std::cout << "Sirius Sprite Editor!" << std::endl;

    InitWindow(1280, 720, "Sirius Sprite Editor");
    SetTargetFPS(60);

    rlImGuiSetup(true);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        rlImGuiBegin();
        ImGui::ShowDemoWindow();
        rlImGuiEnd();

        EndDrawing();
    }

    ImGui::DestroyContext();
    CloseWindow();
    return 0;
}