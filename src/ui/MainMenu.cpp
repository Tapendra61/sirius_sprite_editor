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
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 7.0f));
    bool menuOpen = ImGui::BeginMainMenuBar();
    ImGui::PopStyleVar();

    if (menuOpen) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Project...", "Ctrl+O")) {
                editor.openProject();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                editor.saveProject();
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                editor.saveProjectAs();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Open Image...", "Ctrl+Shift+I")) {
                openImageDialog(editor);
            }

            bool hasImage = editor.project.isImageLoaded();
            if (ImGui::MenuItem("Reload Image", nullptr, false, hasImage)) {
                std::string path = editor.project.imagePath;
                editor.project.loadImage(path);
            }

            ImGui::Separator();

            bool hasSlices = editor.project.slices.count() > 0;
            if (ImGui::MenuItem("Export...", "Ctrl+E", false, hasSlices)) {
                editor.exportModal.open();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
                editor.shouldExit = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, editor.commands.canUndo())) {
                editor.commands.undo(editor.project.slices);
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, editor.commands.canRedo())) {
                editor.commands.redo(editor.project.slices);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Keybindings...")) {
                editor.keybindingsModal.open();
            }

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
