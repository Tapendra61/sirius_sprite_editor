#include "util/HitTest.h"

#include "model/SliceStore.h"
#include "util/Coords.h"

static bool nearPoint(Vector2 mouse, Vector2 target, float maxDist) {
    float dx = mouse.x - target.x;
    float dy = mouse.y - target.y;
    return (dx * dx + dy * dy) <= (maxDist * maxDist);
}

Hit pickHit(Vector2 mouseScreen, const SliceStore& store, const CanvasView& v) {
    const float HIT_PX = 8.0f;

    Vector2 mouseInPanel;
    mouseInPanel.x = mouseScreen.x - v.panelBounds.x;
    mouseInPanel.y = mouseScreen.y - v.panelBounds.y;

    bool checkHandles = (store.selectedIds.size() == 1);

    // First pass: selected slices (handles only when single-selected, body always)
    for (size_t i = 0; i < store.selectedIds.size(); ++i) {
        const Slice* s = store.find(store.selectedIds[i]);
        if (s == nullptr) continue;

        Vector2 imgTL = { s->rect.x, s->rect.y };
        Vector2 imgBR = { s->rect.x + s->rect.width, s->rect.y + s->rect.height };

        Vector2 nw = GetWorldToScreen2D(imgTL, v.camera);
        Vector2 se = GetWorldToScreen2D(imgBR, v.camera);
        Vector2 ne = { se.x, nw.y };
        Vector2 sw = { nw.x, se.y };
        Vector2 n  = { (nw.x + ne.x) * 0.5f, nw.y };
        Vector2 s_ = { (sw.x + se.x) * 0.5f, sw.y };
        Vector2 e  = { ne.x, (ne.y + se.y) * 0.5f };
        Vector2 w  = { nw.x, (nw.y + sw.y) * 0.5f };

        Hit h;
        h.sliceId = s->id;

        if (checkHandles) {
            if (nearPoint(mouseInPanel, nw, HIT_PX)) { h.handle = HandleId::NW; return h; }
            if (nearPoint(mouseInPanel, ne, HIT_PX)) { h.handle = HandleId::NE; return h; }
            if (nearPoint(mouseInPanel, se, HIT_PX)) { h.handle = HandleId::SE; return h; }
            if (nearPoint(mouseInPanel, sw, HIT_PX)) { h.handle = HandleId::SW; return h; }

            if (nearPoint(mouseInPanel, n,  HIT_PX)) { h.handle = HandleId::N;  return h; }
            if (nearPoint(mouseInPanel, e,  HIT_PX)) { h.handle = HandleId::E;  return h; }
            if (nearPoint(mouseInPanel, s_, HIT_PX)) { h.handle = HandleId::S;  return h; }
            if (nearPoint(mouseInPanel, w,  HIT_PX)) { h.handle = HandleId::W;  return h; }
        }

        Rectangle ssRect = { nw.x, nw.y, se.x - nw.x, se.y - nw.y };
        if (CheckCollisionPointRec(mouseInPanel, ssRect)) {
            h.handle = HandleId::Body;
            return h;
        }
    }

    // Second pass: unselected slices, body only, top-most first
    for (size_t i = store.slices.size(); i > 0; --i) {
        const Slice& s = store.slices[i - 1];
        if (store.isSelected(s.id)) continue;

        Vector2 imgTL = { s.rect.x, s.rect.y };
        Vector2 imgBR = { s.rect.x + s.rect.width, s.rect.y + s.rect.height };
        Vector2 nw = GetWorldToScreen2D(imgTL, v.camera);
        Vector2 se = GetWorldToScreen2D(imgBR, v.camera);

        Rectangle ssRect = { nw.x, nw.y, se.x - nw.x, se.y - nw.y };
        if (CheckCollisionPointRec(mouseInPanel, ssRect)) {
            Hit h;
            h.sliceId = s.id;
            h.handle = HandleId::Body;
            return h;
        }
    }

    Hit miss;
    miss.sliceId = -1;
    miss.handle = HandleId::None;
    return miss;
}

std::vector<int> pickAllBodyHits(Vector2 mouseScreen, const SliceStore& store, const CanvasView& v) {
    Vector2 mouseInPanel;
    mouseInPanel.x = mouseScreen.x - v.panelBounds.x;
    mouseInPanel.y = mouseScreen.y - v.panelBounds.y;

    std::vector<int> result;
    for (size_t i = store.slices.size(); i > 0; --i) {
        const Slice& s = store.slices[i - 1];
        Vector2 imgTL = { s.rect.x, s.rect.y };
        Vector2 imgBR = { s.rect.x + s.rect.width, s.rect.y + s.rect.height };
        Vector2 nw = GetWorldToScreen2D(imgTL, v.camera);
        Vector2 se = GetWorldToScreen2D(imgBR, v.camera);
        Rectangle ssRect = { nw.x, nw.y, se.x - nw.x, se.y - nw.y };
        if (CheckCollisionPointRec(mouseInPanel, ssRect)) {
            result.push_back(s.id);
        }
    }
    return result;
}
