#include "app/Editor.h"

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include "commands/SliceCommands.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ops/Importer.h"
#include "ops/Trim.h"
#include "portable-file-dialogs.h"

static void buildDefaultLayout(ImGuiID dockId) {
    ImGui::DockBuilderRemoveNode(dockId);
    ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockId, ImGui::GetMainViewport()->WorkSize);

    ImGuiID mainDock = dockId;

    ImGuiID rightDock;
    ImGui::DockBuilderSplitNode(mainDock, ImGuiDir_Right, 0.25f, &rightDock, &mainDock);

    ImGuiID rightBottomDock;
    ImGui::DockBuilderSplitNode(rightDock, ImGuiDir_Down, 0.40f, &rightBottomDock, &rightDock);

    ImGui::DockBuilderDockWindow("Canvas", mainDock);
    ImGui::DockBuilderDockWindow("Slices", rightDock);
    ImGui::DockBuilderDockWindow("Inspector", rightBottomDock);

    ImGui::DockBuilderFinish(dockId);
}

Editor::Editor() : shouldExit(false), resetLayoutRequested(false) {
    view.camera.target = { 0.0f, 0.0f };
    view.camera.offset = { 0.0f, 0.0f };
    view.camera.rotation = 0.0f;
    view.camera.zoom = 1.0f;
    view.panelBounds = { 0.0f, 0.0f, 0.0f, 0.0f };

    drag.mode = DragMode::Idle;
    drag.startImg = { 0.0f, 0.0f };
    drag.activeSliceId = -1;
    drag.handle = HandleId::None;
    drag.marqueeEnd = { 0.0f, 0.0f };
    drag.marqueeAdditive = false;
    drag.dragHappened = false;
    drag.cycleNextId = -1;
    drag.hoveredSliceId = -1;

    iconCmd        = LoadTexture("resources/icons/command-key.png");
    iconShift      = LoadTexture("resources/icons/shift-key.png");
    iconToolSelect = LoadTexture("resources/icons/selection-tool.png");
    iconToolMove   = LoadTexture("resources/icons/move-tool.png");
    iconToolRect   = LoadTexture("resources/icons/rect-tool.png");
    if (iconCmd.id != 0)        SetTextureFilter(iconCmd,        TEXTURE_FILTER_BILINEAR);
    if (iconShift.id != 0)      SetTextureFilter(iconShift,      TEXTURE_FILTER_BILINEAR);
    if (iconToolSelect.id != 0) SetTextureFilter(iconToolSelect, TEXTURE_FILTER_BILINEAR);
    if (iconToolMove.id != 0)   SetTextureFilter(iconToolMove,   TEXTURE_FILTER_BILINEAR);
    if (iconToolRect.id != 0)   SetTextureFilter(iconToolRect,   TEXTURE_FILTER_BILINEAR);
}

Editor::~Editor() {
    if (iconCmd.id != 0)        UnloadTexture(iconCmd);
    if (iconShift.id != 0)      UnloadTexture(iconShift);
    if (iconToolSelect.id != 0) UnloadTexture(iconToolSelect);
    if (iconToolMove.id != 0)   UnloadTexture(iconToolMove);
    if (iconToolRect.id != 0)   UnloadTexture(iconToolRect);
}

static void updateWindowTitle(const Project& project) {
    char title[512];
    const char* dirty = project.isDirty() ? "*" : "";
    if (project.projectPath.empty()) {
        std::snprintf(title, sizeof(title),
                      "untitled%s \xe2\x80\x94 Sirius Sprite Editor", dirty);
    } else {
        std::filesystem::path p(project.projectPath);
        std::snprintf(title, sizeof(title),
                      "%s%s \xe2\x80\x94 Sirius Sprite Editor",
                      p.filename().string().c_str(), dirty);
    }
    SetWindowTitle(title);
}

void Editor::update() {
    updateWindowTitle(project);

    if (IsFileDropped()) {
        FilePathList dropped = LoadDroppedFiles();
        TraceLog(LOG_INFO, "drag-drop: %u file(s)", dropped.count);
        if (dropped.count > 0) {
            TraceLog(LOG_INFO, "drag-drop path: %s", dropped.paths[0]);
            if (project.loadImage(dropped.paths[0])) {
                view.camera.target = { 0.0f, 0.0f };
                view.camera.zoom = 1.0f;
            } else {
                TraceLog(LOG_WARNING, "drag-drop: loadImage failed");
            }
        }
        UnloadDroppedFiles(dropped);
    }

    // While the keybindings modal is recording a chord, swallow all shortcuts
    // so the new chord doesn't also fire its previous binding.
    if (keybindingsModal.isRecording()) return;

    // File
    if (keybindings.isPressed(Action::OpenProject))   openProject();
    if (keybindings.isPressed(Action::SaveProjectAs)) saveProjectAs();
    else if (keybindings.isPressed(Action::SaveProject)) saveProject();
    if (keybindings.isPressed(Action::OpenImage)) {
        std::vector<std::string> filters = {
            "Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.gif",
            "All Files", "*"
        };
        std::vector<std::string> sel = pfd::open_file(
            "Open Image", "", filters, pfd::opt::none).result();
        if (!sel.empty()) {
            if (project.loadImage(sel[0])) {
                view.camera.target = { 0.0f, 0.0f };
                view.camera.zoom = 1.0f;
            }
        }
    }
    if (keybindings.isPressed(Action::Quit)) shouldExit = true;

    // Slice / Export
    if (keybindings.isPressed(Action::Export) && project.slices.count() > 0) {
        exportModal.open();
    }
    if (keybindings.isPressed(Action::GridSlice) && project.isImageLoaded()) {
        gridModal.open();
    }
    if (keybindings.isPressed(Action::AutoSlice) && project.isImageLoaded()) {
        autoModal.open();
    }

    // Zoom
    if (keybindings.isPressed(Action::ZoomActual)) zoomTo(1.0f);
    if (keybindings.isPressed(Action::ZoomFit))    zoomFit();
    if (keybindings.isPressed(Action::ZoomIn))     zoomBy(1.1f);
    if (keybindings.isPressed(Action::ZoomOut))    zoomBy(1.0f / 1.1f);

    // Tools
    if (keybindings.isPressed(Action::ToolSelect)) toolbar.mode = ToolMode::Select;
    if (keybindings.isPressed(Action::ToolMove))   toolbar.mode = ToolMode::Move;
    if (keybindings.isPressed(Action::ToolRect))   toolbar.mode = ToolMode::Rectangle;

    // Edit / undo
    if (keybindings.isPressed(Action::Undo)) commands.undo(project.slices);
    if (keybindings.isPressed(Action::Redo)) commands.redo(project.slices);

    bool hasSelection = !project.slices.selectedIds.empty();

    if (hasSelection && keybindings.isPressed(Action::Delete)) {
        deleteSelected();
    }

    if (hasSelection && keybindings.isPressed(Action::Duplicate)) {
        duplicateSelected();
    }

    if (keybindings.isPressed(Action::SelectAll)) {
        project.slices.selectClear();
        for (size_t i = 0; i < project.slices.slices.size(); ++i) {
            project.slices.selectAdd(project.slices.slices[i].id);
        }
    }

    if (keybindings.isPressed(Action::Deselect)) {
        project.slices.selectClear();
    }
}

static std::vector<std::string> projectFilters() {
    return {
        "Sirius Project (*.srsprite)", "*.srsprite",
        "All Files", "*"
    };
}

void Editor::openProject() {
    std::vector<std::string> picks = pfd::open_file(
        "Open Project", "", projectFilters(), pfd::opt::none).result();
    if (picks.empty()) return;
    openProjectPath(picks[0]);
}

void Editor::openProjectPath(const std::string& path) {
    if (path.empty()) return;
    if (Importer::loadProject(project, view, path)) {
        commands.clear();
        drag.mode = DragMode::Idle;
        drag.snapshot.clear();
        recentFiles.add(path);
    } else {
        recentFiles.remove(path);
    }
}

void Editor::saveProject() {
    if (project.projectPath.empty()) {
        saveProjectAs();
        return;
    }
    if (Importer::saveProject(project, view, project.projectPath)) {
        project.clearDirty();
        recentFiles.add(project.projectPath);
    }
}

void Editor::zoomTo(float zoom) {
    view.camera.zoom = zoom;
    view.camera.target = { 0.0f, 0.0f };
}

void Editor::zoomFit() {
    if (!project.isImageLoaded()) {
        zoomTo(1.0f);
        return;
    }
    float panelW = view.panelBounds.width;
    float panelH = view.panelBounds.height;
    if (panelW < 1.0f || panelH < 1.0f) return;

    float imgW = (float)project.image.width;
    float imgH = (float)project.image.height;
    float zx = panelW / imgW;
    float zy = panelH / imgH;
    float fit = (zx < zy ? zx : zy) * 0.95f;
    if (fit < 0.1f)  fit = 0.1f;
    if (fit > 64.0f) fit = 64.0f;

    view.camera.zoom = fit;
    view.camera.target.x = imgW * 0.5f - panelW / (2.0f * fit);
    view.camera.target.y = imgH * 0.5f - panelH / (2.0f * fit);
}

void Editor::deleteSelected() {
    if (project.slices.selectedIds.empty()) return;

    std::vector<Slice> deleted;
    for (size_t i = 0; i < project.slices.selectedIds.size(); ++i) {
        const Slice* s = project.slices.find(project.slices.selectedIds[i]);
        if (s != nullptr) deleted.push_back(*s);
    }
    if (deleted.empty()) return;

    std::unique_ptr<DeleteSlicesCommand> cmd(new DeleteSlicesCommand(deleted));
    commands.push(std::move(cmd), project.slices);
    project.markDirty();
}

void Editor::duplicateSelected() {
    if (project.slices.selectedIds.empty()) return;

    std::vector<Slice> before = project.slices.slices;
    std::vector<Slice> after = before;
    std::vector<int> newIds;
    for (size_t i = 0; i < project.slices.selectedIds.size(); ++i) {
        const Slice* s = project.slices.find(project.slices.selectedIds[i]);
        if (s == nullptr) continue;
        Slice copy = *s;
        copy.id = project.nextId();
        copy.rect.x += 8.0f;
        copy.rect.y += 8.0f;
        copy.name = std::string("slice_") + std::to_string(copy.id);
        after.push_back(copy);
        newIds.push_back(copy.id);
    }
    if (newIds.empty()) return;

    std::unique_ptr<ReplaceAllCommand> cmd(new ReplaceAllCommand(before, after));
    commands.push(std::move(cmd), project.slices);
    project.slices.selectClear();
    for (size_t i = 0; i < newIds.size(); ++i) {
        project.slices.selectAdd(newIds[i]);
    }
    project.markDirty();
}

void Editor::trimSelected() {
    if (project.slices.selectedIds.empty()) return;
    if (!project.isImageLoaded()) return;

    std::vector<Slice> before;
    std::vector<Slice> after;
    for (size_t i = 0; i < project.slices.selectedIds.size(); ++i) {
        const Slice* s = project.slices.find(project.slices.selectedIds[i]);
        if (s == nullptr) continue;
        Rectangle trimmed = trimTransparentEdges(project.image, s->rect, 0);
        if (trimmed.x == s->rect.x && trimmed.y == s->rect.y &&
            trimmed.width == s->rect.width && trimmed.height == s->rect.height) {
            continue;
        }
        before.push_back(*s);
        Slice updated = *s;
        updated.rect = trimmed;
        after.push_back(updated);
    }
    if (before.empty()) return;

    std::unique_ptr<EditSlicesCommand> cmd(new EditSlicesCommand(before, after));
    commands.push(std::move(cmd), project.slices);
    project.markDirty();
}

void Editor::renameSlice(int sliceId, const std::string& newName) {
    const Slice* current = project.slices.find(sliceId);
    if (current == nullptr) return;
    if (current->name == newName) return;

    std::vector<Slice> before;
    before.push_back(*current);
    Slice updated = *current;
    updated.name = newName;
    std::vector<Slice> after;
    after.push_back(updated);

    std::unique_ptr<EditSlicesCommand> cmd(new EditSlicesCommand(before, after));
    commands.push(std::move(cmd), project.slices);
    project.markDirty();
}

void Editor::zoomBy(float factor) {
    float newZoom = view.camera.zoom * factor;
    if (newZoom < 0.1f)  newZoom = 0.1f;
    if (newZoom > 64.0f) newZoom = 64.0f;

    float panelW = view.panelBounds.width;
    float panelH = view.panelBounds.height;
    Vector2 centerScreen = { panelW * 0.5f, panelH * 0.5f };
    Vector2 worldBefore = GetScreenToWorld2D(centerScreen, view.camera);
    view.camera.zoom = newZoom;
    Vector2 worldAfter = GetScreenToWorld2D(centerScreen, view.camera);
    view.camera.target.x += worldBefore.x - worldAfter.x;
    view.camera.target.y += worldBefore.y - worldAfter.y;
}

void Editor::saveProjectAs() {
    std::string path = pfd::save_file(
        "Save Project As", "untitled.srsprite",
        projectFilters(), pfd::opt::none).result();
    if (path.empty()) return;

    if (Importer::saveProject(project, view, path)) {
        project.projectPath = path;
        project.clearDirty();
        recentFiles.add(path);
    }
}

void Editor::render() {
    mainMenu.draw(*this);
    toolbar.draw(*this);
    statusBar.draw(*this);

    ImGuiID dockId = ImGui::DockSpaceOverViewport();

    static bool firstFrame = true;
    if (firstFrame || resetLayoutRequested) {
        bool shouldBuild = resetLayoutRequested;
        if (firstFrame) {
            ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockId);
            if (node == nullptr || node->IsLeafNode()) {
                shouldBuild = true;
            }
        }
        if (shouldBuild) {
            buildDefaultLayout(dockId);
        }
        firstFrame = false;
        resetLayoutRequested = false;
    }

    canvas.draw(*this);
    inspector.draw(*this);
    sliceList.draw(*this);
    gridModal.draw(*this);
    autoModal.draw(*this);
    exportModal.draw(*this);
    keybindingsModal.draw(*this);
    docsWindow.draw();
    canvasOptionsPanel.draw(*this);
}
