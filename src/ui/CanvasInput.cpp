#include "ui/CanvasInput.h"

#include <cmath>
#include <memory>
#include <string>
#include "app/Editor.h"
#include "commands/SliceCommands.h"
#include "imgui.h"
#include "raylib.h"
#include "util/Geometry.h"
#include "util/HitTest.h"

static bool isSnapModifierDown() {
    return IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)
        || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
}

// Snap helpers: pixel modifier always rounds to integers; otherwise if grid
// snap is enabled in canvas options, snap to multiples of step.
static float snapAxis(float v, int step, bool pixelOverride, bool gridOn) {
    if (pixelOverride) return std::round(v);
    if (gridOn && step > 1) return (float)((int)std::round(v / (float)step) * step);
    return v;
}

static Vector2 snapVec(Vector2 p, const Editor& editor, bool pixelOverride) {
    bool gridOn = editor.canvasOptions.gridSnapEnabled;
    return {
        snapAxis(p.x, editor.canvasOptions.gridSnapX, pixelOverride, gridOn),
        snapAxis(p.y, editor.canvasOptions.gridSnapY, pixelOverride, gridOn),
    };
}

static Rectangle snapRect(Rectangle r, const Editor& editor) {
    bool gridOn = editor.canvasOptions.gridSnapEnabled;
    int sx = editor.canvasOptions.gridSnapX;
    int sy = editor.canvasOptions.gridSnapY;
    Rectangle out = r;
    out.x      = snapAxis(r.x,      sx, !gridOn, gridOn);
    out.y      = snapAxis(r.y,      sy, !gridOn, gridOn);
    out.width  = snapAxis(r.width,  sx, !gridOn, gridOn);
    out.height = snapAxis(r.height, sy, !gridOn, gridOn);
    if (out.width  < 1.0f) out.width  = 1.0f;
    if (out.height < 1.0f) out.height = 1.0f;
    return out;
}

static bool isAdditiveModifierDown() {
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) return true;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) return true;
    if (IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) return true;
    return false;
}

static bool isResizeHandle(HandleId h) {
    return h == HandleId::NW || h == HandleId::N  || h == HandleId::NE
        || h == HandleId::E  || h == HandleId::SE || h == HandleId::S
        || h == HandleId::SW || h == HandleId::W;
}

static Rectangle applyResize(Rectangle base, HandleId h, Vector2 delta) {
    Rectangle r = base;
    switch (h) {
        case HandleId::NW:
            r.x += delta.x; r.y += delta.y;
            r.width  -= delta.x; r.height -= delta.y;
            break;
        case HandleId::N:
            r.y += delta.y; r.height -= delta.y;
            break;
        case HandleId::NE:
            r.y += delta.y;
            r.width  += delta.x; r.height -= delta.y;
            break;
        case HandleId::E:
            r.width += delta.x;
            break;
        case HandleId::SE:
            r.width += delta.x; r.height += delta.y;
            break;
        case HandleId::S:
            r.height += delta.y;
            break;
        case HandleId::SW:
            r.x += delta.x;
            r.width  -= delta.x; r.height += delta.y;
            break;
        case HandleId::W:
            r.x += delta.x; r.width -= delta.x;
            break;
        default:
            break;
    }
    return r;
}

static void takeSelectionSnapshot(Editor& editor) {
    editor.drag.snapshot.clear();
    const std::vector<int>& selIds = editor.project.slices.selectedIds;
    for (size_t i = 0; i < selIds.size(); ++i) {
        const Slice* s = editor.project.slices.find(selIds[i]);
        if (s != nullptr) {
            editor.drag.snapshot.push_back(*s);
        }
    }
}

static void handleMousePress(Editor& editor) {
    Vector2 mouseScreen = GetMousePosition();
    Vector2 mouseImg = ScreenToImage(mouseScreen, editor.view);
    Hit hit = pickHit(mouseScreen, editor.project.slices, editor.view);
    bool additive = isAdditiveModifierDown();

    editor.drag.startImg = mouseImg;
    editor.drag.marqueeEnd = mouseImg;
    editor.drag.handle = hit.handle;
    editor.drag.dragHappened = false;
    editor.drag.cycleNextId = -1;

    // Resize-handle hit: bypass body cycle logic.
    if (hit.sliceId != -1 && isResizeHandle(hit.handle)) {
        if (!editor.project.slices.isSelected(hit.sliceId)) {
            editor.project.slices.selectOnly(hit.sliceId);
        }
        editor.drag.activeSliceId = hit.sliceId;
        editor.drag.mode = DragMode::Resizing;
        takeSelectionSnapshot(editor);
        return;
    }

    // Collect every slice whose body contains the cursor (top-most first).
    std::vector<int> overlapIds = pickAllBodyHits(mouseScreen, editor.project.slices, editor.view);

    if (overlapIds.empty()) {
        editor.drag.activeSliceId = -1;
        if (additive) {
            editor.drag.mode = DragMode::Marquee;
            editor.drag.marqueeAdditive = true;
        } else {
            editor.project.slices.selectClear();
            if (editor.toolbar.mode == ToolMode::Rectangle) {
                editor.drag.mode = DragMode::Creating;
            } else {
                editor.drag.mode = DragMode::Marquee;
                editor.drag.marqueeAdditive = false;
            }
        }
        return;
    }

    if (additive) {
        int topId = overlapIds[0];
        if (editor.project.slices.isSelected(topId)) {
            editor.project.slices.selectRemove(topId);
        } else {
            editor.project.slices.selectAdd(topId);
        }
        editor.drag.activeSliceId = topId;
        editor.drag.mode = DragMode::Idle;
        return;
    }

    // Non-additive body hit: find a currently selected slice in the overlap.
    int currentIdx = -1;
    for (size_t i = 0; i < overlapIds.size(); ++i) {
        if (editor.project.slices.isSelected(overlapIds[i])) {
            currentIdx = (int)i;
            break;
        }
    }

    if (currentIdx >= 0) {
        // Selected slice is here. Defer the cycle decision: drag = move,
        // click-without-drag = cycle to the next overlapping slice.
        if (overlapIds.size() > 1) {
            int nextIdx = (currentIdx + 1) % (int)overlapIds.size();
            editor.drag.cycleNextId = overlapIds[nextIdx];
        }
        editor.drag.activeSliceId = overlapIds[currentIdx];
    } else {
        editor.project.slices.selectOnly(overlapIds[0]);
        editor.drag.activeSliceId = overlapIds[0];
    }
    editor.drag.handle = HandleId::Body;
    editor.drag.mode = DragMode::Moving;
    takeSelectionSnapshot(editor);
}

static void handleMouseDrag(Editor& editor) {
    bool pixelOverride = isSnapModifierDown();

    bool inRectDrag = editor.drag.mode == DragMode::Marquee
                   || editor.drag.mode == DragMode::Creating;
    if (inRectDrag) {
        Vector2 mouseImg = ScreenToImage(GetMousePosition(), editor.view);
        mouseImg = snapVec(mouseImg, editor, pixelOverride);
        editor.drag.marqueeEnd = mouseImg;
        return;
    }

    if (editor.drag.mode == DragMode::Moving) {
        Vector2 mouseImg = ScreenToImage(GetMousePosition(), editor.view);
        Vector2 delta;
        delta.x = mouseImg.x - editor.drag.startImg.x;
        delta.y = mouseImg.y - editor.drag.startImg.y;
        delta = snapVec(delta, editor, pixelOverride);

        if (!editor.drag.dragHappened) {
            float zoom = editor.view.camera.zoom;
            float dxPx = delta.x * zoom;
            float dyPx = delta.y * zoom;
            const float DRAG_THRESHOLD_PX_SQ = 3.0f * 3.0f;
            if (dxPx * dxPx + dyPx * dyPx > DRAG_THRESHOLD_PX_SQ) {
                editor.drag.dragHappened = true;
            }
        }

        if (editor.drag.dragHappened) {
            for (size_t i = 0; i < editor.drag.snapshot.size(); ++i) {
                const Slice& orig = editor.drag.snapshot[i];
                Slice* live = editor.project.slices.find(orig.id);
                if (live == nullptr) continue;
                live->rect.x = orig.rect.x + delta.x;
                live->rect.y = orig.rect.y + delta.y;
            }
        }
        return;
    }

    if (editor.drag.mode == DragMode::Resizing) {
        Vector2 mouseImg = ScreenToImage(GetMousePosition(), editor.view);
        Vector2 delta;
        delta.x = mouseImg.x - editor.drag.startImg.x;
        delta.y = mouseImg.y - editor.drag.startImg.y;
        delta = snapVec(delta, editor, pixelOverride);

        for (size_t i = 0; i < editor.drag.snapshot.size(); ++i) {
            if (editor.drag.snapshot[i].id != editor.drag.activeSliceId) continue;
            const Slice& orig = editor.drag.snapshot[i];
            Slice* live = editor.project.slices.find(orig.id);
            if (live == nullptr) continue;
            live->rect = applyResize(orig.rect, editor.drag.handle, delta);
        }
    }
}

static void commitMarquee(Editor& editor) {
    Rectangle m = RectFromCorners(editor.drag.startImg, editor.drag.marqueeEnd);
    if (m.width < 1.0f || m.height < 1.0f) return;

    if (!editor.drag.marqueeAdditive) {
        editor.project.slices.selectClear();
    }
    const std::vector<Slice>& slices = editor.project.slices.slices;
    for (size_t i = 0; i < slices.size(); ++i) {
        if (RectsOverlap(slices[i].rect, m)) {
            editor.project.slices.selectAdd(slices[i].id);
        }
    }
}

static void commitMoving(Editor& editor) {
    bool gridOn = editor.canvasOptions.gridSnapEnabled;
    int sx = editor.canvasOptions.gridSnapX;
    int sy = editor.canvasOptions.gridSnapY;
    std::vector<Slice> after;
    for (size_t i = 0; i < editor.drag.snapshot.size(); ++i) {
        Slice* live = editor.project.slices.find(editor.drag.snapshot[i].id);
        if (live == nullptr) continue;
        live->rect.x = snapAxis(live->rect.x, sx, !gridOn, gridOn);
        live->rect.y = snapAxis(live->rect.y, sy, !gridOn, gridOn);
        after.push_back(*live);
    }
    std::unique_ptr<EditSlicesCommand> cmd(new EditSlicesCommand(editor.drag.snapshot, after));
    editor.commands.pushAlreadyApplied(std::move(cmd));
    editor.drag.snapshot.clear();
    editor.project.markDirty();
}

static void commitResizing(Editor& editor) {
    Slice* live = editor.project.slices.find(editor.drag.activeSliceId);
    if (live == nullptr) {
        editor.drag.snapshot.clear();
        return;
    }

    Rectangle r = NormalizeRect(live->rect);
    if (editor.canvasOptions.gridSnapEnabled) {
        r = snapRect(r, editor);
    } else {
        r = SnapRectToPixels(r);
    }

    if (r.width < 1.0f || r.height < 1.0f) {
        std::vector<Slice> deleted;
        for (size_t i = 0; i < editor.drag.snapshot.size(); ++i) {
            if (editor.drag.snapshot[i].id == editor.drag.activeSliceId) {
                deleted.push_back(editor.drag.snapshot[i]);
                break;
            }
        }
        editor.project.slices.remove(live->id);
        if (!deleted.empty()) {
            std::unique_ptr<DeleteSlicesCommand> cmd(new DeleteSlicesCommand(deleted));
            editor.commands.pushAlreadyApplied(std::move(cmd));
        }
    } else {
        live->rect = r;
        std::vector<Slice> after;
        after.push_back(*live);
        std::unique_ptr<EditSlicesCommand> cmd(new EditSlicesCommand(editor.drag.snapshot, after));
        editor.commands.pushAlreadyApplied(std::move(cmd));
    }
    editor.drag.snapshot.clear();
    editor.project.markDirty();
}

static void commitCreating(Editor& editor) {
    Rectangle r = RectFromCorners(editor.drag.startImg, editor.drag.marqueeEnd);
    if (editor.canvasOptions.gridSnapEnabled) {
        r = snapRect(r, editor);
    } else {
        r = SnapRectToPixels(r);
    }
    if (r.width < 2.0f || r.height < 2.0f) return;

    Slice s;
    s.id = editor.project.nextId();
    s.name = std::string("slice_") + std::to_string(s.id);
    s.rect = r;
    s.pivot = { 0.5f, 0.5f };
    s.border = { 0.0f, 0.0f, 0.0f, 0.0f };
    std::unique_ptr<CreateSliceCommand> cmd(new CreateSliceCommand(s));
    editor.commands.push(std::move(cmd), editor.project.slices);
    editor.project.slices.selectOnly(s.id);
    editor.project.markDirty();
}

static void handleMouseRelease(Editor& editor) {
    switch (editor.drag.mode) {
        case DragMode::Marquee:
            commitMarquee(editor);
            break;
        case DragMode::Moving:
            if (editor.drag.dragHappened) {
                commitMoving(editor);
            } else if (editor.drag.cycleNextId != -1) {
                editor.project.slices.selectOnly(editor.drag.cycleNextId);
                editor.drag.snapshot.clear();
            } else {
                editor.drag.snapshot.clear();
            }
            break;
        case DragMode::Resizing:
            commitResizing(editor);
            break;
        case DragMode::Creating:
            commitCreating(editor);
            break;
        default:
            break;
    }
    editor.drag.mode = DragMode::Idle;
    editor.drag.activeSliceId = -1;
    editor.drag.handle = HandleId::None;
    editor.drag.dragHappened = false;
    editor.drag.cycleNextId = -1;
}

static void applyHoverCursor(HandleId handle, bool bodySelected) {
    switch (handle) {
        case HandleId::NW:
        case HandleId::SE:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE); break;
        case HandleId::NE:
        case HandleId::SW:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW); break;
        case HandleId::N:
        case HandleId::S:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);   break;
        case HandleId::E:
        case HandleId::W:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);   break;
        case HandleId::Body:
            if (bodySelected) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            break;
        default:
            break;
    }
}

void handleCanvasMouse(Editor& editor, bool canvasHovered) {
    if (editor.toolbar.mode == ToolMode::Move) {
        editor.drag.hoveredSliceId = -1;
        return;
    }

    // Hover-only pickHit each frame for cursor + highlight (skip while actively dragging)
    bool isDragging = editor.drag.mode == DragMode::Moving
                   || editor.drag.mode == DragMode::Resizing
                   || editor.drag.mode == DragMode::Marquee
                   || editor.drag.mode == DragMode::Creating;
    if (canvasHovered && !isDragging) {
        Hit hover = pickHit(GetMousePosition(), editor.project.slices, editor.view);
        editor.drag.hoveredSliceId = hover.sliceId;
        bool bodySelected = hover.sliceId != -1 &&
                            editor.project.slices.isSelected(hover.sliceId);
        applyHoverCursor(hover.handle, bodySelected);
    } else if (!canvasHovered) {
        editor.drag.hoveredSliceId = -1;
    }

    if (canvasHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        handleMousePress(editor);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        handleMouseDrag(editor);
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        handleMouseRelease(editor);
    }
}

static bool isZoomModifierDown() {
    return IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)
        || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
}

static void zoomTowardCursor(Editor& editor, float wheelY) {
    if (wheelY == 0.0f) return;

    Vector2 mouseScreen = GetMousePosition();
    Vector2 mouseInPanel;
    mouseInPanel.x = mouseScreen.x - editor.view.panelBounds.x;
    mouseInPanel.y = mouseScreen.y - editor.view.panelBounds.y;

    Vector2 worldBefore = GetScreenToWorld2D(mouseInPanel, editor.view.camera);

    float factor = std::pow(1.1f, wheelY);
    editor.view.camera.zoom *= factor;
    if (editor.view.camera.zoom < 0.1f) editor.view.camera.zoom = 0.1f;
    if (editor.view.camera.zoom > 64.0f) editor.view.camera.zoom = 64.0f;

    Vector2 worldAfter = GetScreenToWorld2D(mouseInPanel, editor.view.camera);
    editor.view.camera.target.x += worldBefore.x - worldAfter.x;
    editor.view.camera.target.y += worldBefore.y - worldAfter.y;
}

static void panByWheel(Editor& editor, Vector2 wheel) {
    const float scrollPxPerUnit = 25.0f;
    float pxPerImg = scrollPxPerUnit / editor.view.camera.zoom;
    editor.view.camera.target.x -= wheel.x * pxPerImg;
    editor.view.camera.target.y -= wheel.y * pxPerImg;
}

void handleCanvasPanZoom(Editor& editor, bool canvasHovered) {
    if (!canvasHovered) return;

    // Middle-drag always pans
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        editor.view.camera.target.x -= delta.x / editor.view.camera.zoom;
        editor.view.camera.target.y -= delta.y / editor.view.camera.zoom;
    }

    // Move tool: LMB pans, and we show a 4-way cursor while hovering the canvas
    if (editor.toolbar.mode == ToolMode::Move) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            editor.view.camera.target.x -= delta.x / editor.view.camera.zoom;
            editor.view.camera.target.y -= delta.y / editor.view.camera.zoom;
        }
    }

    Vector2 wheel = GetMouseWheelMoveV();
    if (wheel.x == 0.0f && wheel.y == 0.0f) return;

    if (isZoomModifierDown()) {
        zoomTowardCursor(editor, wheel.y);
    } else {
        panByWheel(editor, wheel);
    }
}
