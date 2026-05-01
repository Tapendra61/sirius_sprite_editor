#include "ui/CanvasDraw.h"

#include <cmath>
#include "app/Editor.h"
#include "util/Geometry.h"

void drawCheckerboard(const Camera2D& camera, int panelW, int panelH) {
    Vector2 topLeftImg     = GetScreenToWorld2D({ 0.0f, 0.0f }, camera);
    Vector2 bottomRightImg = GetScreenToWorld2D({ (float)panelW, (float)panelH }, camera);

    const int sq = 16;
    int startX = (int)std::floor(topLeftImg.x / sq) * sq;
    int startY = (int)std::floor(topLeftImg.y / sq) * sq;
    int endX   = (int)std::ceil(bottomRightImg.x);
    int endY   = (int)std::ceil(bottomRightImg.y);

    Color c1 = { 31, 31, 46, 255 };  // #1F1F2E
    Color c2 = { 24, 24, 38, 255 };  // #181826

    for (int y = startY; y < endY; y += sq) {
        for (int x = startX; x < endX; x += sq) {
            int xi = x / sq;
            int yi = y / sq;
            bool dark = ((xi + yi) & 1) == 0;
            DrawRectangle(x, y, sq, sq, dark ? c1 : c2);
        }
    }
}

void drawSliceOutlines(const Editor& editor) {
    float invZoom = 1.0f / editor.view.camera.zoom;

    Color unselectedColor = { 176, 171, 189, 160 };  // ink-2 @ 60%
    Color selectedColor   = { 129, 140, 248, 255 };  // accent-selection #818CF8

    const std::vector<Slice>& slices = editor.project.slices.slices;
    for (size_t i = 0; i < slices.size(); ++i) {
        const Slice& s = slices[i];
        bool selected = editor.project.slices.isSelected(s.id);

        Color color = selected ? selectedColor : unselectedColor;
        float thickness = (selected ? 2.0f : 1.0f) * invZoom;

        DrawRectangleLinesEx(s.rect, thickness, color);
    }
}

void drawSliceHandles(const Editor& editor) {
    const std::vector<int>& selIds = editor.project.slices.selectedIds;
    if (selIds.size() != 1) return;

    const Camera2D& cam = editor.view.camera;

    Color fill   = { 129, 140, 248, 255 };  // accent-selection
    Color border = { 232, 230, 240, 230 };  // ink

    const float handleSize = 8.0f;
    const float halfSize = handleSize * 0.5f;

    for (size_t i = 0; i < selIds.size(); ++i) {
        const Slice* s = editor.project.slices.find(selIds[i]);
        if (s == nullptr) continue;

        Vector2 imgTL = { s->rect.x, s->rect.y };
        Vector2 imgBR = { s->rect.x + s->rect.width, s->rect.y + s->rect.height };

        Vector2 nw = GetWorldToScreen2D(imgTL, cam);
        Vector2 se = GetWorldToScreen2D(imgBR, cam);
        Vector2 ne = { se.x, nw.y };
        Vector2 sw = { nw.x, se.y };
        Vector2 n  = { (nw.x + ne.x) * 0.5f, nw.y };
        Vector2 s_ = { (sw.x + se.x) * 0.5f, sw.y };
        Vector2 e  = { ne.x, (ne.y + se.y) * 0.5f };
        Vector2 w  = { nw.x, (nw.y + sw.y) * 0.5f };

        Vector2 handles[8] = { nw, n, ne, e, se, s_, sw, w };

        for (int hi = 0; hi < 8; ++hi) {
            int hx = (int)(handles[hi].x - halfSize);
            int hy = (int)(handles[hi].y - halfSize);
            DrawRectangle(hx, hy, (int)handleSize, (int)handleSize, fill);
            DrawRectangleLines(hx, hy, (int)handleSize, (int)handleSize, border);
        }
    }
}

void drawSliceBorders(const Editor& editor) {
    Color borderCol = { 199, 146, 234, 220 };  // accent-border #C792EA
    float thickness = 1.0f / editor.view.camera.zoom;

    const std::vector<Slice>& slices = editor.project.slices.slices;
    for (size_t i = 0; i < slices.size(); ++i) {
        const Slice& s = slices[i];
        if (s.border.x == 0.0f && s.border.y == 0.0f &&
            s.border.width == 0.0f && s.border.height == 0.0f) continue;

        float innerX = s.rect.x + s.border.x;
        float innerY = s.rect.y + s.border.y;
        float innerW = s.rect.width  - s.border.x - s.border.width;
        float innerH = s.rect.height - s.border.y - s.border.height;

        if (innerW <= 0.0f || innerH <= 0.0f) continue;

        Rectangle inner = { innerX, innerY, innerW, innerH };
        DrawRectangleLinesEx(inner, thickness, borderCol);
    }
}

void drawSlicePivots(const Editor& editor) {
    const std::vector<int>& selIds = editor.project.slices.selectedIds;
    if (selIds.empty()) return;

    const Camera2D& cam = editor.view.camera;
    Color fill   = { 245, 158, 107, 255 };  // accent-pivot #F59E6B
    Color border = { 255, 255, 255, 230 };
    const float radius = 5.0f;

    for (size_t i = 0; i < selIds.size(); ++i) {
        const Slice* s = editor.project.slices.find(selIds[i]);
        if (s == nullptr) continue;

        Vector2 pivotImg;
        pivotImg.x = s->rect.x + s->pivot.x * s->rect.width;
        pivotImg.y = s->rect.y + s->pivot.y * s->rect.height;
        Vector2 p = GetWorldToScreen2D(pivotImg, cam);

        DrawCircle((int)p.x, (int)p.y, radius, fill);
        DrawCircleLines((int)p.x, (int)p.y, radius, border);
    }
}

void drawMarquee(const Editor& editor) {
    if (editor.drag.mode != DragMode::Marquee) return;

    Rectangle m = RectFromCorners(editor.drag.startImg, editor.drag.marqueeEnd);
    if (m.width < 0.5f && m.height < 0.5f) return;

    Color fill    = { 129, 140, 248,  40 };
    Color outline = { 129, 140, 248, 220 };
    float thickness = 1.0f / editor.view.camera.zoom;

    DrawRectangleRec(m, fill);
    DrawRectangleLinesEx(m, thickness, outline);
}

void drawCreationPreview(const Editor& editor) {
    if (editor.drag.mode != DragMode::Creating) return;

    Rectangle r = RectFromCorners(editor.drag.startImg, editor.drag.marqueeEnd);
    if (r.width < 0.5f && r.height < 0.5f) return;

    Color fill    = { 107, 203, 119,  40 };  // accent-snap green
    Color outline = { 107, 203, 119, 230 };
    float thickness = 1.0f / editor.view.camera.zoom;

    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, thickness, outline);
}

void drawGridPreview(const Editor& editor) {
    if (!editor.gridModal.showing) return;

    Color fill    = { 79, 209, 197,  30 };  // accent-preview #4FD1C5
    Color outline = { 79, 209, 197, 230 };
    float thickness = 1.0f / editor.view.camera.zoom;

    const std::vector<Slice>& preview = editor.gridModal.preview;
    for (size_t i = 0; i < preview.size(); ++i) {
        DrawRectangleRec(preview[i].rect, fill);
        DrawRectangleLinesEx(preview[i].rect, thickness, outline);
    }
}

void drawAutoPreview(const Editor& editor) {
    if (!editor.autoModal.showing) return;

    Color fill    = { 79, 209, 197,  30 };  // accent-preview #4FD1C5
    Color outline = { 79, 209, 197, 230 };
    float thickness = 1.0f / editor.view.camera.zoom;

    const std::vector<Slice>& preview = editor.autoModal.preview;
    for (size_t i = 0; i < preview.size(); ++i) {
        DrawRectangleRec(preview[i].rect, fill);
        DrawRectangleLinesEx(preview[i].rect, thickness, outline);
    }
}
