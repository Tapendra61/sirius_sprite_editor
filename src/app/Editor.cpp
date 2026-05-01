#include "app/Editor.h"

#include <memory>
#include "commands/SliceCommands.h"
#include "imgui.h"
#include "imgui_internal.h"

static void buildDefaultLayout(ImGuiID dockId) {
    ImGui::DockBuilderRemoveNode(dockId);
    ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockId, ImGui::GetMainViewport()->WorkSize);

    ImGuiID mainDock = dockId;

    ImGuiID rightDock;
    ImGui::DockBuilderSplitNode(mainDock, ImGuiDir_Right, 0.25f, &rightDock, &mainDock);

    ImGuiID rightBottomDock;
    ImGui::DockBuilderSplitNode(rightDock, ImGuiDir_Down, 0.40f, &rightBottomDock, &rightDock);

    ImGuiID toolbarDock;
    ImGui::DockBuilderSplitNode(mainDock, ImGuiDir_Up, 0.08f, &toolbarDock, &mainDock);

    ImGui::DockBuilderDockWindow("Toolbar", toolbarDock);
    ImGui::DockBuilderDockWindow("Canvas", mainDock);
    ImGui::DockBuilderDockWindow("Slices", rightDock);
    ImGui::DockBuilderDockWindow("Inspector", rightBottomDock);

    ImGui::DockBuilderFinish(dockId);
}

Editor::Editor() : editMode(true), shouldExit(false), resetLayoutRequested(false) {
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
}

Editor::~Editor() {
}

void Editor::update() {
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

    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) {
        commands.undo(project.slices);
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)) {
        commands.redo(project.slices);
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y)) {
        commands.redo(project.slices);
    }

    bool hasSelection = !project.slices.selectedIds.empty();

    if (hasSelection && (ImGui::IsKeyPressed(ImGuiKey_Delete) ||
                         ImGui::IsKeyPressed(ImGuiKey_Backspace))) {
        std::vector<Slice> deleted;
        for (size_t i = 0; i < project.slices.selectedIds.size(); ++i) {
            const Slice* s = project.slices.find(project.slices.selectedIds[i]);
            if (s != nullptr) {
                deleted.push_back(*s);
            }
        }
        if (!deleted.empty()) {
            std::unique_ptr<DeleteSlicesCommand> cmd(new DeleteSlicesCommand(deleted));
            commands.push(std::move(cmd), project.slices);
            project.markDirty();
        }
    }

    if (hasSelection && ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_D)) {
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
        if (!newIds.empty()) {
            std::unique_ptr<ReplaceAllCommand> cmd(new ReplaceAllCommand(before, after));
            commands.push(std::move(cmd), project.slices);
            project.slices.selectClear();
            for (size_t i = 0; i < newIds.size(); ++i) {
                project.slices.selectAdd(newIds[i]);
            }
            project.markDirty();
        }
    }

    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_A)) {
        project.slices.selectClear();
        for (size_t i = 0; i < project.slices.slices.size(); ++i) {
            project.slices.selectAdd(project.slices.slices[i].id);
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        project.slices.selectClear();
    }
}

void Editor::render() {
    mainMenu.draw(*this);
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

    toolbar.draw(*this);
    canvas.draw(*this);
    inspector.draw(*this);
    sliceList.draw(*this);
    gridModal.draw(*this);
    autoModal.draw(*this);
}
