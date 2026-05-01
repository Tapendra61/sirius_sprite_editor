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

    Color c1 = { 40, 40, 40, 255 };
    Color c2 = { 50, 50, 50, 255 };

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

    Color unselectedColor = { 200, 200, 210, 160 };
    Color selectedColor   = { 91, 158, 255, 255 };

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

    Color fill   = { 91, 158, 255, 255 };
    Color border = { 240, 240, 245, 230 };

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

void drawMarquee(const Editor& editor) {
    if (editor.drag.mode != DragMode::Marquee) return;

    Rectangle m = RectFromCorners(editor.drag.startImg, editor.drag.marqueeEnd);
    if (m.width < 0.5f && m.height < 0.5f) return;

    Color fill    = { 91, 158, 255,  40 };
    Color outline = { 91, 158, 255, 220 };
    float thickness = 1.0f / editor.view.camera.zoom;

    DrawRectangleRec(m, fill);
    DrawRectangleLinesEx(m, thickness, outline);
}

void drawCreationPreview(const Editor& editor) {
    if (editor.drag.mode != DragMode::Creating) return;

    Rectangle r = RectFromCorners(editor.drag.startImg, editor.drag.marqueeEnd);
    if (r.width < 0.5f && r.height < 0.5f) return;

    Color fill    = { 80, 200, 120,  40 };
    Color outline = { 80, 200, 120, 230 };
    float thickness = 1.0f / editor.view.camera.zoom;

    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, thickness, outline);
}

void drawGridPreview(const Editor& editor) {
    if (!editor.gridModal.showing) return;

    Color fill    = { 80, 200, 200,  30 };
    Color outline = { 80, 200, 200, 220 };
    float thickness = 1.0f / editor.view.camera.zoom;

    const std::vector<Slice>& preview = editor.gridModal.preview;
    for (size_t i = 0; i < preview.size(); ++i) {
        DrawRectangleRec(preview[i].rect, fill);
        DrawRectangleLinesEx(preview[i].rect, thickness, outline);
    }
}

void drawAutoPreview(const Editor& editor) {
    if (!editor.autoModal.showing) return;

    Color fill    = { 255, 180, 80,  30 };
    Color outline = { 255, 180, 80, 230 };
    float thickness = 1.0f / editor.view.camera.zoom;

    const std::vector<Slice>& preview = editor.autoModal.preview;
    for (size_t i = 0; i < preview.size(); ++i) {
        DrawRectangleRec(preview[i].rect, fill);
        DrawRectangleLinesEx(preview[i].rect, thickness, outline);
    }
}
