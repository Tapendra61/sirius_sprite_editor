#include "ui/GridSliceModal.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include "app/Editor.h"
#include "commands/SliceCommands.h"
#include "imgui.h"
#include "ops/GridSlicer.h"
#include "ui/Theme.h"

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
    std::strncpy(namePattern, "slice_{i}", sizeof(namePattern) - 1);
    namePattern[sizeof(namePattern) - 1] = '\0';
}

static std::string expandPattern(const std::string& pat, int idx, int row, int col, int total) {
    int width = 1;
    int t = total;
    while (t >= 10) { t /= 10; ++width; }

    std::string out;
    out.reserve(pat.size() + 8);
    char buf[32];

    for (size_t k = 0; k < pat.size(); ++k) {
        if (pat[k] == '{' && k + 2 < pat.size() && pat[k + 2] == '}') {
            char tok = pat[k + 1];
            if (tok == 'i') {
                std::snprintf(buf, sizeof(buf), "%0*d", width, idx);
                out += buf;
                k += 2;
                continue;
            }
            if (tok == 'r') {
                std::snprintf(buf, sizeof(buf), "%d", row);
                out += buf;
                k += 2;
                continue;
            }
            if (tok == 'c') {
                std::snprintf(buf, sizeof(buf), "%d", col);
                out += buf;
                k += 2;
                continue;
            }
        }
        out += pat[k];
    }
    return out;
}

static int countGridCols(int imgW, int cellW, int offX, int padX) {
    if (cellW <= 0) return 1;
    int strideX = cellW + padX;
    if (strideX <= 0) return 1;
    int cols = 0;
    for (int x = offX; x + cellW <= imgW; x += strideX) ++cols;
    return cols < 1 ? 1 : cols;
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
    ImGui::InputText("Name Pattern", namePattern, sizeof(namePattern));
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Tokens: {i}=index, {r}=row, {c}=column");
    }
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

    bool cancelClicked = ImGui::Button("Cancel");
    ImGui::SameLine();

    bool canApply = !preview.empty();
    if (!canApply) ImGui::BeginDisabled();
    bool applyClicked = PrimaryButton("Apply");
    if (!canApply) ImGui::EndDisabled();

    if (applyClicked) {
        std::vector<Slice> oldSlices = editor.project.slices.slices;
        std::vector<Slice> newSlices;
        if (!replaceAll) {
            newSlices = oldSlices;
        }

        int effCellW = (mode == GridMode::BySize) ? cellWidth :
            (imgW - offsetX - (columns - 1) * paddingX) / (columns > 0 ? columns : 1);
        if (effCellW < 1) effCellW = 1;
        int colCount = countGridCols(imgW, effCellW, offsetX, paddingX);
        std::string pattern(namePattern);
        if (pattern.empty()) pattern = "slice_{i}";
        int total = (int)rects.size();

        for (size_t i = 0; i < rects.size(); ++i) {
            Slice s;
            s.id = editor.project.nextId();
            int idx = (int)i + 1;
            int r = (int)i / colCount + 1;
            int c = (int)i % colCount + 1;
            s.name = expandPattern(pattern, idx, r, c, total);
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
