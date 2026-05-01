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

// Larger padding for menu headers; popped before dropdown contents render.
static void pushHeaderPad() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 14.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,  ImVec2(14.0f, 4.0f));
}
static void popHeaderPad() {
    ImGui::PopStyleVar(2);
}

// Larger row height + spacing for items inside a dropdown.
static void pushItemPad() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(12.0f, 14.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(8.0f,  8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f,  8.0f));
}
static void popItemPad() {
    ImGui::PopStyleVar(3);
}

// Vertical line between menu headers, drawn in the gap.
// menuH must match the header padding pushed in pushHeaderPad (14.0f vertical).
static void menuItemSep() {
    ImVec2 cursor  = ImGui::GetCursorScreenPos();
    float menuH    = ImGui::GetFontSize() + 14.0f * 2.0f;
    float lineH    = menuH * 0.9f;
    float spacing  = 14.0f;

    float lineX    = cursor.x - spacing * 0.5f;
    float lineYTop = cursor.y + (menuH - lineH) * 0.5f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 col = ImGui::GetColorU32(ImGuiCol_Border);
    dl->AddLine(ImVec2(lineX, lineYTop), ImVec2(lineX, lineYTop + lineH), col, 1.0f);
}

void MainMenu::draw(Editor& editor) {
    // No bg effect on hover, click, or open — labels only.
    ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    ImGui::PushStyleColor(ImGuiCol_Header,        transparent);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, transparent);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  transparent);

    // Push header padding so the bar's height is computed with it
    pushHeaderPad();
    bool barOpen = ImGui::BeginMainMenuBar();
    popHeaderPad();

    if (barOpen) {
        // FILE
        pushHeaderPad();
        bool fileOpen = ImGui::BeginMenu("File");
        popHeaderPad();
        if (fileOpen) {
            pushItemPad();
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
            popItemPad();
            ImGui::EndMenu();
        }

        menuItemSep();

        // EDIT
        pushHeaderPad();
        bool editOpen = ImGui::BeginMenu("Edit");
        popHeaderPad();
        if (editOpen) {
            pushItemPad();
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

            popItemPad();
            ImGui::EndMenu();
        }

        menuItemSep();

        // VIEW
        pushHeaderPad();
        bool viewOpen = ImGui::BeginMenu("View");
        popHeaderPad();
        if (viewOpen) {
            pushItemPad();
            if (ImGui::MenuItem("Reset Layout")) {
                editor.resetLayoutRequested = true;
            }
            popItemPad();
            ImGui::EndMenu();
        }

        menuItemSep();

        // SLICE
        pushHeaderPad();
        bool sliceOpen = ImGui::BeginMenu("Slice");
        popHeaderPad();
        if (sliceOpen) {
            pushItemPad();
            bool hasImage = editor.project.isImageLoaded();
            if (ImGui::MenuItem("Grid Slice...", "Ctrl+G", false, hasImage)) {
                editor.gridModal.open();
            }
            if (ImGui::MenuItem("Auto Slice...", "Ctrl+Shift+A", false, hasImage)) {
                editor.autoModal.open();
            }
            popItemPad();
            ImGui::EndMenu();
        }

        menuItemSep();

        // HELP
        pushHeaderPad();
        bool helpOpen = ImGui::BeginMenu("Help");
        popHeaderPad();
        if (helpOpen) {
            pushItemPad();
            ImGui::MenuItem("(later)", nullptr, false, false);
            popItemPad();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleColor(3);
}
