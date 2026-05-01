#include "ui/Inspector.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include "app/Editor.h"
#include "commands/SliceCommands.h"
#include "imgui.h"

Inspector::Inspector() : wasActive(false), hasSnapshot(false) {
}

Inspector::~Inspector() {
}

static const ImVec4 INK_3   = ImVec4(0.478f, 0.455f, 0.565f, 1.0f);  // labels / muted
static const ImVec4 PIVOT   = ImVec4(0.961f, 0.620f, 0.420f, 1.0f);  // accent-pivot

static bool slicesEqual(const Slice& a, const Slice& b) {
    if (a.id != b.id) return false;
    if (a.name != b.name) return false;
    if (a.rect.x != b.rect.x || a.rect.y != b.rect.y ||
        a.rect.width != b.rect.width || a.rect.height != b.rect.height) return false;
    if (a.pivot.x != b.pivot.x || a.pivot.y != b.pivot.y) return false;
    if (a.border.x != b.border.x || a.border.y != b.border.y ||
        a.border.width != b.border.width || a.border.height != b.border.height) return false;
    return true;
}

static bool snapshotsDiffer(const std::vector<Slice>& a, const std::vector<Slice>& b) {
    if (a.size() != b.size()) return true;
    for (size_t i = 0; i < a.size(); ++i) {
        if (!slicesEqual(a[i], b[i])) return true;
    }
    return false;
}

static std::vector<Slice> takeSelectionSnapshot(const Editor& editor) {
    std::vector<Slice> out;
    const std::vector<int>& selIds = editor.project.slices.selectedIds;
    for (size_t i = 0; i < selIds.size(); ++i) {
        const Slice* s = editor.project.slices.find(selIds[i]);
        if (s != nullptr) out.push_back(*s);
    }
    return out;
}

// Layout helpers --------------------------------------------------------------

static float twoColFieldWidth() {
    float availW = ImGui::GetContentRegionAvail().x;
    float labelW = 16.0f;  // single-letter labels (X, Y, W, H, L, T, R, B)
    float gap    = 6.0f;   // between label and input
    float colGap = ImGui::GetStyle().ItemSpacing.x;
    float w = (availW - 2.0f * labelW - 2.0f * gap - colGap) * 0.5f;
    if (w < 32.0f) w = 32.0f;
    return w;
}

static bool intCell(const char* label, const char* id, int* val,
                    float w, bool mixed, bool& anyActive) {
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(INK_3, "%s", label);
    ImGui::SameLine(0.0f, 6.0f);
    ImGui::PushItemWidth(w);
    bool changed = ImGui::DragInt(id, val, 0.5f, 0, 0, mixed ? "\xe2\x80\x94" : "%d");
    ImGui::PopItemWidth();
    if (ImGui::IsItemActive()) anyActive = true;
    return changed;
}

static bool floatCell(const char* label, const char* id, float* val,
                      float w, bool mixed, bool& anyActive) {
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(INK_3, "%s", label);
    ImGui::SameLine(0.0f, 6.0f);
    ImGui::PushItemWidth(w);
    bool changed = ImGui::DragFloat(id, val, 0.01f, 0.0f, 1.0f,
                                    mixed ? "\xe2\x80\x94" : "%.2f");
    ImGui::PopItemWidth();
    if (ImGui::IsItemActive()) anyActive = true;
    return changed;
}

// Sections --------------------------------------------------------------------

static void drawIdentity(const std::vector<Slice*>& sel, bool& anyActive) {
    if (!ImGui::CollapsingHeader("Identity", ImGuiTreeNodeFlags_DefaultOpen)) return;

    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(INK_3, "Name");
    ImGui::SameLine(0.0f, 8.0f);
    ImGui::PushItemWidth(-1.0f);

    if (sel.size() == 1) {
        Slice* s = sel[0];
        char nameBuf[128];
        std::strncpy(nameBuf, s->name.c_str(), sizeof(nameBuf) - 1);
        nameBuf[sizeof(nameBuf) - 1] = '\0';
        if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf))) {
            s->name = nameBuf;
        }
        if (ImGui::IsItemActive()) anyActive = true;
    } else {
        char placeholder[64];
        std::snprintf(placeholder, sizeof(placeholder), "%d slices", (int)sel.size());
        ImGui::BeginDisabled();
        ImGui::InputText("##name", placeholder, sizeof(placeholder),
                         ImGuiInputTextFlags_ReadOnly);
        ImGui::EndDisabled();
    }
    ImGui::PopItemWidth();

    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(INK_3, "ID");
    ImGui::SameLine(0.0f, 8.0f);
    if (sel.size() == 1) {
        ImGui::Text("#%d", sel[0]->id);
    } else {
        ImGui::TextColored(INK_3, "(multiple)");
    }
}

static void drawGeometry(const std::vector<Slice*>& sel, bool& anyActive) {
    if (!ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) return;

    int rx = (int)sel[0]->rect.x;
    int ry = (int)sel[0]->rect.y;
    int rw = (int)sel[0]->rect.width;
    int rh = (int)sel[0]->rect.height;
    bool sameX = true, sameY = true, sameW = true, sameH = true;
    for (size_t i = 1; i < sel.size(); ++i) {
        if ((int)sel[i]->rect.x      != rx) sameX = false;
        if ((int)sel[i]->rect.y      != ry) sameY = false;
        if ((int)sel[i]->rect.width  != rw) sameW = false;
        if ((int)sel[i]->rect.height != rh) sameH = false;
    }

    float fw = twoColFieldWidth();

    if (intCell("X", "##gx", &rx, fw, !sameX, anyActive)) {
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->rect.x = (float)rx;
    }
    ImGui::SameLine();
    if (intCell("Y", "##gy", &ry, fw, !sameY, anyActive)) {
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->rect.y = (float)ry;
    }

    if (intCell("W", "##gw", &rw, fw, !sameW, anyActive)) {
        if (rw < 1) rw = 1;
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->rect.width = (float)rw;
    }
    ImGui::SameLine();
    if (intCell("H", "##gh", &rh, fw, !sameH, anyActive)) {
        if (rh < 1) rh = 1;
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->rect.height = (float)rh;
    }
}

static void drawPivot(const std::vector<Slice*>& sel, bool& anyActive) {
    if (!ImGui::CollapsingHeader("Pivot", ImGuiTreeNodeFlags_DefaultOpen)) return;

    float px = sel[0]->pivot.x;
    float py = sel[0]->pivot.y;
    bool same = true;
    for (size_t i = 1; i < sel.size(); ++i) {
        if (sel[i]->pivot.x != px || sel[i]->pivot.y != py) { same = false; break; }
    }

    // 3x3 preset grid
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            float ppx = col * 0.5f;
            float ppy = row * 0.5f;
            bool active = same && (px == ppx && py == ppy);

            if (active) ImGui::PushStyleColor(ImGuiCol_Button, PIVOT);
            char id[8];
            std::snprintf(id, sizeof(id), "##p%d%d", row, col);
            if (ImGui::Button(id, ImVec2(22.0f, 22.0f))) {
                for (size_t i = 0; i < sel.size(); ++i) {
                    sel[i]->pivot.x = ppx;
                    sel[i]->pivot.y = ppy;
                }
            }
            if (ImGui::IsItemActive()) anyActive = true;
            if (active) ImGui::PopStyleColor();

            if (col < 2) ImGui::SameLine(0.0f, 4.0f);
        }
    }

    ImGui::Spacing();

    float fw = twoColFieldWidth();
    float pv[2] = { same ? px : 0.5f, same ? py : 0.5f };
    if (floatCell("X", "##pvx", &pv[0], fw, !same, anyActive)) {
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->pivot.x = pv[0];
    }
    ImGui::SameLine();
    if (floatCell("Y", "##pvy", &pv[1], fw, !same, anyActive)) {
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->pivot.y = pv[1];
    }
}

static int clampNonNeg(int v) { return v < 0 ? 0 : v; }

static void drawBorder(const std::vector<Slice*>& sel, bool& anyActive) {
    if (!ImGui::CollapsingHeader("9-Slice Border")) return;

    int bl = (int)sel[0]->border.x;
    int bt = (int)sel[0]->border.y;
    int br = (int)sel[0]->border.width;
    int bb = (int)sel[0]->border.height;
    bool sameL = true, sameT = true, sameR = true, sameB = true;
    for (size_t i = 1; i < sel.size(); ++i) {
        if ((int)sel[i]->border.x      != bl) sameL = false;
        if ((int)sel[i]->border.y      != bt) sameT = false;
        if ((int)sel[i]->border.width  != br) sameR = false;
        if ((int)sel[i]->border.height != bb) sameB = false;
    }

    float fw = twoColFieldWidth();

    if (intCell("L", "##bl", &bl, fw, !sameL, anyActive)) {
        bl = clampNonNeg(bl);
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->border.x = (float)bl;
    }
    ImGui::SameLine();
    if (intCell("T", "##bt", &bt, fw, !sameT, anyActive)) {
        bt = clampNonNeg(bt);
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->border.y = (float)bt;
    }

    if (intCell("R", "##br", &br, fw, !sameR, anyActive)) {
        br = clampNonNeg(br);
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->border.width = (float)br;
    }
    ImGui::SameLine();
    if (intCell("B", "##bb", &bb, fw, !sameB, anyActive)) {
        bb = clampNonNeg(bb);
        for (size_t i = 0; i < sel.size(); ++i) sel[i]->border.height = (float)bb;
    }
}

void Inspector::draw(Editor& editor) {
    ImGui::Begin("Inspector");

    int selCount = editor.project.slices.selectionCount();

    if (selCount == 0) {
        ImGui::TextColored(INK_3, "No slice selected.");
        ImGui::TextWrapped("Click a slice on the canvas or in the list.");
        ImGui::End();
        return;
    }

    ImGui::TextColored(INK_3, "%d selected", selCount);
    ImGui::Spacing();

    std::vector<Slice*> sel;
    const std::vector<int>& selIds = editor.project.slices.selectedIds;
    for (size_t i = 0; i < selIds.size(); ++i) {
        Slice* s = editor.project.slices.find(selIds[i]);
        if (s != nullptr) sel.push_back(s);
    }

    bool anyActive = false;

    if (!sel.empty()) {
        drawIdentity(sel, anyActive);
        drawGeometry(sel, anyActive);
        drawPivot(sel, anyActive);
        drawBorder(sel, anyActive);
    }

    bool startedEditing = !wasActive && anyActive;
    bool stoppedEditing = wasActive && !anyActive;

    if (startedEditing && !hasSnapshot) {
        snapshot = takeSelectionSnapshot(editor);
        hasSnapshot = true;
    }

    if (stoppedEditing && hasSnapshot) {
        std::vector<Slice> after = takeSelectionSnapshot(editor);
        if (snapshotsDiffer(snapshot, after)) {
            std::unique_ptr<EditSlicesCommand> cmd(new EditSlicesCommand(snapshot, after));
            editor.commands.pushAlreadyApplied(std::move(cmd));
            editor.project.markDirty();
        }
        hasSnapshot = false;
    }

    wasActive = anyActive;

    ImGui::End();
}
