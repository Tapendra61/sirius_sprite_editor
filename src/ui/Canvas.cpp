#include "ui/Canvas.h"

#include <cfloat>
#include "app/Editor.h"
#include "imgui.h"
#include "rlImGui.h"
#include "ui/CanvasDraw.h"
#include "ui/CanvasInput.h"
#include "ui/Theme.h"
#include "util/HitTest.h"

Canvas::Canvas() : rt{}, rtWidth(0), rtHeight(0) {
}

Canvas::~Canvas() {
    if (rt.id != 0) {
        UnloadRenderTexture(rt);
    }
}

static void resizeRenderTexture(RenderTexture2D& rt, int& rtWidth, int& rtHeight, int w, int h) {
    if (rt.id != 0 && rtWidth == w && rtHeight == h) return;
    if (rt.id != 0) UnloadRenderTexture(rt);
    rt = LoadRenderTexture(w, h);
    SetTextureFilter(rt.texture, TEXTURE_FILTER_POINT);
    rtWidth = w;
    rtHeight = h;
}

static void drawEmptyStateOverlay(ImVec2 panelOrigin, int w, int h) {
    const char* msg = "Drop an image file here to start";
    ImFont* font = g_FontMain ? g_FontMain : ImGui::GetFont();
    float fontSize = 32.0f;

    ImVec2 sz = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, msg);
    ImVec2 pos(panelOrigin.x + (w - sz.x) * 0.5f,
               panelOrigin.y + (h - sz.y) * 0.5f);
    ImU32 col = IM_COL32(176, 171, 189, 220);  // ink-2
    ImGui::GetWindowDrawList()->AddText(font, fontSize, pos, col, msg);
}

void Canvas::draw(Editor& editor) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Canvas", nullptr,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar(2);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    int w = (int)avail.x;
    int h = (int)avail.y;
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    resizeRenderTexture(rt, rtWidth, rtHeight, w, h);

    ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
    editor.view.panelBounds.x = cursorScreen.x;
    editor.view.panelBounds.y = cursorScreen.y;
    editor.view.panelBounds.width = (float)w;
    editor.view.panelBounds.height = (float)h;

    BeginTextureMode(rt);
        ClearBackground(BLANK);

        BeginMode2D(editor.view.camera);
            drawCheckerboard(editor, w, h);
            if (editor.project.isImageLoaded()) {
                Vector2 origin = { 0.0f, 0.0f };
                DrawTextureV(editor.project.texture, origin, WHITE);
            }
            drawPixelGrid(editor, w, h);
            drawSliceOutlines(editor);
            drawSliceBorders(editor);
            drawMarquee(editor);
            drawCreationPreview(editor);
            drawGridPreview(editor);
            drawAutoPreview(editor);
        EndMode2D();

        drawSliceHandles(editor);
        drawSlicePivots(editor);
    EndTextureMode();

    rlImGuiImageRenderTexture(&rt);
    bool canvasHovered = ImGui::IsItemHovered();

    // Overlay the empty-state message via ImGui (renders at proper HIDPI density).
    if (!editor.project.isImageLoaded()) {
        drawEmptyStateOverlay(cursorScreen, w, h);
    }

    handleCanvasMouse(editor, canvasHovered);
    handleCanvasPanZoom(editor, canvasHovered);

    // Right-click context menu: pre-select slice under cursor, then open popup.
    if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        Hit hit = pickHit(GetMousePosition(), editor.project.slices, editor.view);
        if (hit.sliceId != -1 && !editor.project.slices.isSelected(hit.sliceId)) {
            editor.project.slices.selectOnly(hit.sliceId);
        }
        ImGui::OpenPopup("##canvas_ctx");
    }

    if (ImGui::BeginPopup("##canvas_ctx")) {
        bool hasSelection = editor.project.slices.selectionCount() > 0;
        if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, hasSelection)) {
            editor.duplicateSelected();
        }
        if (ImGui::MenuItem("Trim Transparent Edges", nullptr, false,
                            hasSelection && editor.project.isImageLoaded())) {
            editor.trimSelected();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete", "Del", false, hasSelection)) {
            editor.deleteSelected();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}
