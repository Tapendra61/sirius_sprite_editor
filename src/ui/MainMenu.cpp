#include "ui/MainMenu.h"

#include <string>
#include <vector>
#include "app/Editor.h"
#include "imgui.h"
#include "portable-file-dialogs.h"

MainMenu::MainMenu() {
}

MainMenu::~MainMenu() {
}

static void openImageDialog(Editor& editor) {
    std::vector<std::string> filters;
    filters.push_back("Image Files (*.png *.jpg *.jpeg *.bmp *.tga *.gif)");
    filters.push_back("*.png *.jpg *.jpeg *.bmp *.tga *.gif");
    filters.push_back("All Files");
    filters.push_back("*");

    std::vector<std::string> selection = pfd::open_file(
        "Open Image", "", filters, pfd::opt::none).result();

    if (!selection.empty()) {
        if (editor.project.loadImage(selection[0])) {
            editor.view.camera.target = { 0.0f, 0.0f };
            editor.view.camera.zoom = 1.0f;
        }
    }
}

void MainMenu::draw(Editor& editor) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Image...", "Ctrl+Shift+O")) {
                openImageDialog(editor);
            }

            bool hasImage = editor.project.isImageLoaded();
            if (ImGui::MenuItem("Reload Image", nullptr, false, hasImage)) {
                std::string path = editor.project.imagePath;
                editor.project.loadImage(path);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
                editor.shouldExit = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("(phase 3+)", nullptr, false, false);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Reset Layout")) {
                editor.resetLayoutRequested = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Slice")) {
            bool hasImage = editor.project.isImageLoaded();
            if (ImGui::MenuItem("Grid Slice...", "Ctrl+G", false, hasImage)) {
                editor.gridModal.open();
            }
            if (ImGui::MenuItem("Auto Slice...", "Ctrl+Shift+A", false, hasImage)) {
                editor.autoModal.open();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("(later)", nullptr, false, false);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
