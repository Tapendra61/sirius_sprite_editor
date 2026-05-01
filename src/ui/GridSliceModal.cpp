#include "ui/GridSliceModal.h"

#include <memory>
#include <string>
#include "app/Editor.h"
#include "commands/SliceCommands.h"
#include "imgui.h"
#include "ops/GridSlicer.h"

GridSliceModal::GridSliceModal()
    : showing(false),
      openRequested(false),
      mode(GridMode::BySize),
      cellWidth(32),
      cellHeight(32),
      columns(8),
      rows(8),
      offsetX(0),
      offsetY(0),
      paddingX(0),
      paddingY(0),
      replaceAll(true) {
}

GridSliceModal::~GridSliceModal() {
}

void GridSliceModal::open() {
    openRequested = true;
}

static std::vector<Rectangle> computeRects(GridMode mode,
                                            int cellW, int cellH,
                                            int cols, int rows,
                                            int offX, int offY,
                                            int padX, int padY,
                                            int imgW, int imgH) {
    if (mode == GridMode::BySize) {
        GridParams p;
        p.cellWidth  = cellW;
        p.cellHeight = cellH;
        p.offsetX    = offX;
        p.offsetY    = offY;
        p.paddingX   = padX;
        p.paddingY   = padY;
        return sliceGridBySize(imgW, imgH, p);
    }
    return sliceGridByCount(imgW, imgH, cols, rows, offX, offY, padX, padY);
}

void GridSliceModal::draw(Editor& editor) {
    if (openRequested) {
        ImGui::OpenPopup("Grid Slice");
        openRequested = false;
    }

    showing = ImGui::IsPopupOpen("Grid Slice");

    if (!ImGui::BeginPopupModal("Grid Slice", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!showing) preview.clear();
        return;
    }

    if (ImGui::BeginTabBar("##gridmode")) {
        if (ImGui::BeginTabItem("By Size")) {
            mode = GridMode::BySize;
            ImGui::InputInt("Cell Width",  &cellWidth);
            ImGui::InputInt("Cell Height", &cellHeight);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("By Count")) {
            mode = GridMode::ByCount;
            ImGui::InputInt("Columns", &columns);
            ImGui::InputInt("Rows",    &rows);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    ImGui::InputInt("Offset X",  &offsetX);
    ImGui::InputInt("Offset Y",  &offsetY);
    ImGui::InputInt("Padding X", &paddingX);
    ImGui::InputInt("Padding Y", &paddingY);

    ImGui::Separator();
    ImGui::Checkbox("Replace existing slices", &replaceAll);

    if (cellWidth  < 1) cellWidth  = 1;
    if (cellHeight < 1) cellHeight = 1;
    if (columns    < 1) columns    = 1;
    if (rows       < 1) rows       = 1;
    if (offsetX    < 0) offsetX    = 0;
    if (offsetY    < 0) offsetY    = 0;
    if (paddingX   < 0) paddingX   = 0;
    if (paddingY   < 0) paddingY   = 0;

    int imgW = editor.project.isImageLoaded() ? editor.project.image.width  : 256;
    int imgH = editor.project.isImageLoaded() ? editor.project.image.height : 256;

    std::vector<Rectangle> rects = computeRects(
        mode, cellWidth, cellHeight, columns, rows,
        offsetX, offsetY, paddingX, paddingY, imgW, imgH);

    preview.clear();
    for (size_t i = 0; i < rects.size(); ++i) {
        Slice s;
        s.id = -1;
        s.rect = rects[i];
        s.pivot = { 0.5f, 0.5f };
        s.border = { 0.0f, 0.0f, 0.0f, 0.0f };
        preview.push_back(s);
    }

    ImGui::Separator();
    ImGui::Text("Will create %d slices", (int)preview.size());

    ImGui::Separator();

    bool canApply = !preview.empty();
    if (!canApply) ImGui::BeginDisabled();
    bool applyClicked = ImGui::Button("Apply");
    if (!canApply) ImGui::EndDisabled();

    ImGui::SameLine();
    bool cancelClicked = ImGui::Button("Cancel");

    if (applyClicked) {
        std::vector<Slice> oldSlices = editor.project.slices.slices;
        std::vector<Slice> newSlices;
        if (!replaceAll) {
            newSlices = oldSlices;
        }
        for (size_t i = 0; i < rects.size(); ++i) {
            Slice s;
            s.id = editor.project.nextId();
            s.name = std::string("slice_") + std::to_string(s.id);
            s.rect = rects[i];
            s.pivot = { 0.5f, 0.5f };
            s.border = { 0.0f, 0.0f, 0.0f, 0.0f };
            newSlices.push_back(s);
        }
        std::unique_ptr<ReplaceAllCommand> cmd(new ReplaceAllCommand(oldSlices, newSlices));
        editor.commands.push(std::move(cmd), editor.project.slices);
        editor.project.markDirty();

        preview.clear();
        ImGui::CloseCurrentPopup();
    } else if (cancelClicked) {
        preview.clear();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
