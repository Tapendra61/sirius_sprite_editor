#include "ui/Canvas.h"

#include "app/Editor.h"
#include "imgui.h"
#include "rlImGui.h"
#include "ui/CanvasDraw.h"
#include "ui/CanvasInput.h"

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

static void drawEmptyStateMessage(int w, int h) {
    const char* msg = "Drop an image file here to start";
    int fontSize = 20;
    int textW = MeasureText(msg, fontSize);
    DrawText(msg, (w - textW) / 2, h / 2 - fontSize / 2, fontSize, LIGHTGRAY);
}

static void drawEditToggle(Editor& editor, ImVec2 origin) {
    ImGui::SetCursorScreenPos(ImVec2(origin.x + 12.0f, origin.y + 12.0f));
    const char* label = editor.editMode ? "Edit: ON" : "Edit: OFF";
    bool pushedStyle = editor.editMode;
    if (pushedStyle) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.36f, 0.62f, 1.0f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.71f, 1.0f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.29f, 0.55f, 0.91f, 1.0f));
    }
    if (ImGui::Button(label)) {
        editor.editMode = !editor.editMode;
    }
    if (pushedStyle) {
        ImGui::PopStyleColor(3);
    }
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
            drawCheckerboard(editor.view.camera, w, h);
            if (editor.project.isImageLoaded()) {
                Vector2 origin = { 0.0f, 0.0f };
                DrawTextureV(editor.project.texture, origin, WHITE);
            }
            drawSliceOutlines(editor);
            drawMarquee(editor);
            drawCreationPreview(editor);
            drawGridPreview(editor);
            drawAutoPreview(editor);
        EndMode2D();

        drawSliceHandles(editor);

        if (!editor.project.isImageLoaded()) {
            drawEmptyStateMessage(w, h);
        }
    EndTextureMode();

    ImVec2 imageOrigin = ImGui::GetCursorScreenPos();
    rlImGuiImageRenderTexture(&rt);
    bool imageHovered = ImGui::IsItemHovered();

    drawEditToggle(editor, imageOrigin);
    bool buttonHovered = ImGui::IsItemHovered();

    bool canvasHovered = imageHovered && !buttonHovered;

    handleCanvasMouse(editor, canvasHovered);
    handleCanvasPanZoom(editor, canvasHovered);

    ImGui::End();
}
