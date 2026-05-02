#include "ui/CanvasOptionsPanel.h"

#include <cstdio>
#include "app/Editor.h"
#include "imgui.h"
#include "ui/Palette.h"
#include "ui/Theme.h"

CanvasOptionsPanel::CanvasOptionsPanel()
    : visible(false), selectedSection(0), snapshotInitialized(false) {
}

static bool optionsDiffer(const CanvasOptions& a, const CanvasOptions& b) {
    if (a.checkerVisible   != b.checkerVisible)   return true;
    if (a.checkerSize      != b.checkerSize)      return true;
    if (a.gridSnapEnabled  != b.gridSnapEnabled)  return true;
    if (a.gridSnapX        != b.gridSnapX)        return true;
    if (a.gridSnapY        != b.gridSnapY)        return true;
    if (a.pixelGridVisible != b.pixelGridVisible) return true;
    if (a.pixelGridZoomMin != b.pixelGridZoomMin) return true;
    for (int i = 0; i < 3; ++i) {
        if (a.checkerLight[i] != b.checkerLight[i]) return true;
        if (a.checkerDark[i]  != b.checkerDark[i])  return true;
    }
    return false;
}

CanvasOptionsPanel::~CanvasOptionsPanel() {
}

void CanvasOptionsPanel::show() {
    visible = true;
}

using pal::INK;
using pal::INK_2;
using pal::INK_3;
using pal::ACCENT;

static const float kLabelW   = 110.0f;
static const float kSidebarW = 170.0f;

// ---- Reusable helpers --------------------------------------------------------

static void rowLabel(const char* text) {
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(INK_2, "%s", text);
    ImGui::SameLine(kLabelW);
}

static void hintLine(const char* text) {
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::TextColored(INK_3, "%s", text);
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
}

static void paneTitle(const char* text) {
    if (g_FontMain) ImGui::PushFont(g_FontMain);
    ImGui::TextColored(INK, "%s", text);
    if (g_FontMain) ImGui::PopFont();

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    float availW = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddLine(
        p0, ImVec2(p0.x + availW, p0.y),
        ImGui::GetColorU32(ImGuiCol_Border), 1.0f);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

// iOS-style toggle switch.
static bool toggleSwitch(const char* id, bool* v) {
    float alpha = ImGui::GetStyle().Alpha;
    float h = ImGui::GetFontSize() + 4.0f;
    float w = h * 1.85f;

    ImVec2 cur = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(id, ImVec2(w, h));
    bool clicked = ImGui::IsItemClicked();
    bool hovered = ImGui::IsItemHovered();
    if (clicked) *v = !*v;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    int A = (int)(255 * alpha);

    // Track
    ImU32 trackOff = IM_COL32(40, 42, 60, A);
    ImU32 trackOn  = IM_COL32(129, 140, 248, A);
    ImU32 trackOnH = IM_COL32(150, 160, 252, A);
    ImU32 trackOffH = IM_COL32(50, 52, 72, A);
    ImU32 trackCol = *v ? (hovered ? trackOnH : trackOn)
                        : (hovered ? trackOffH : trackOff);
    dl->AddRectFilled(cur, ImVec2(cur.x + w, cur.y + h), trackCol, h * 0.5f);

    // Knob
    float knobR = (h * 0.5f) - 3.0f;
    float knobX = *v ? (cur.x + w - knobR - 3.0f) : (cur.x + knobR + 3.0f);
    float knobY = cur.y + h * 0.5f;
    ImU32 knobCol = IM_COL32(255, 255, 255, A);
    dl->AddCircleFilled(ImVec2(knobX, knobY), knobR, knobCol, 24);

    return clicked;
}

// Slim slider (rounded track + circular thumb + accent fill). Value text is
// drawn just to the right of the slider, taking valueTextW pixels.
static bool slimSlider(const char* id, int* v, int min, int max, float w,
                       const char* fmt) {
    float alpha = ImGui::GetStyle().Alpha;
    float thumbR = 7.0f;
    float trackH = 4.0f;
    float h = thumbR * 2.0f + 4.0f;

    const float valueW = 56.0f;
    const float gap    = 8.0f;
    float sliderW = w - valueW - gap;
    if (sliderW < thumbR * 4.0f) sliderW = thumbR * 4.0f;

    ImVec2 cur = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(id, ImVec2(sliderW, h));
    bool active  = ImGui::IsItemActive();
    bool hovered = ImGui::IsItemHovered();

    bool changed = false;
    if (active && ImGui::IsMouseDown(0)) {
        float mx = ImGui::GetIO().MousePos.x;
        float t  = (mx - cur.x - thumbR) / (sliderW - thumbR * 2.0f);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        int newV = min + (int)((max - min) * t + 0.5f);
        if (newV != *v) {
            *v = newV;
            changed = true;
        }
    }

    float t = (max > min) ? ((float)(*v - min) / (float)(max - min)) : 0.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    int A = (int)(255 * alpha);
    float trackY = cur.y + h * 0.5f;
    float trackL = cur.x + thumbR;
    float trackR = cur.x + sliderW - thumbR;
    float fillEnd = trackL + (trackR - trackL) * t;

    // Track bg
    dl->AddRectFilled(
        ImVec2(trackL, trackY - trackH * 0.5f),
        ImVec2(trackR, trackY + trackH * 0.5f),
        IM_COL32(28, 28, 42, A), trackH * 0.5f);

    // Track fill (indigo)
    dl->AddRectFilled(
        ImVec2(trackL, trackY - trackH * 0.5f),
        ImVec2(fillEnd, trackY + trackH * 0.5f),
        IM_COL32(129, 140, 248, A), trackH * 0.5f);

    // Halo behind thumb when hovered/active
    if (active || hovered) {
        dl->AddCircleFilled(ImVec2(fillEnd, trackY), thumbR + 4.0f,
                            IM_COL32(129, 140, 248, (int)(60 * alpha)), 24);
    }

    // Thumb
    dl->AddCircleFilled(ImVec2(fillEnd, trackY), thumbR,
                        IM_COL32(255, 255, 255, A), 24);
    dl->AddCircle(ImVec2(fillEnd, trackY), thumbR,
                  IM_COL32(0, 0, 0, (int)(40 * alpha)), 24, 1.0f);

    // Value text
    char buf[32];
    std::snprintf(buf, sizeof(buf), fmt, *v);
    ImVec2 textSize = ImGui::CalcTextSize(buf);
    float textX = cur.x + sliderW + gap;
    float textY = cur.y + (h - textSize.y) * 0.5f;
    dl->AddText(ImVec2(textX, textY),
                IM_COL32(232, 230, 240, A), buf);

    return changed;
}

// Same as slimSlider but for floats.
static bool slimSliderFloat(const char* id, float* v, float min, float max,
                            float w, const char* fmt) {
    float alpha = ImGui::GetStyle().Alpha;
    float thumbR = 7.0f;
    float trackH = 4.0f;
    float h = thumbR * 2.0f + 4.0f;

    const float valueW = 56.0f;
    const float gap    = 8.0f;
    float sliderW = w - valueW - gap;
    if (sliderW < thumbR * 4.0f) sliderW = thumbR * 4.0f;

    ImVec2 cur = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(id, ImVec2(sliderW, h));
    bool active  = ImGui::IsItemActive();
    bool hovered = ImGui::IsItemHovered();

    bool changed = false;
    if (active && ImGui::IsMouseDown(0)) {
        float mx = ImGui::GetIO().MousePos.x;
        float t  = (mx - cur.x - thumbR) / (sliderW - thumbR * 2.0f);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        float newV = min + (max - min) * t;
        if (newV != *v) { *v = newV; changed = true; }
    }

    float t = (max > min) ? ((*v - min) / (max - min)) : 0.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    int A = (int)(255 * alpha);
    float trackY = cur.y + h * 0.5f;
    float trackL = cur.x + thumbR;
    float trackR = cur.x + sliderW - thumbR;
    float fillEnd = trackL + (trackR - trackL) * t;

    dl->AddRectFilled(
        ImVec2(trackL, trackY - trackH * 0.5f),
        ImVec2(trackR, trackY + trackH * 0.5f),
        IM_COL32(28, 28, 42, A), trackH * 0.5f);
    dl->AddRectFilled(
        ImVec2(trackL, trackY - trackH * 0.5f),
        ImVec2(fillEnd, trackY + trackH * 0.5f),
        IM_COL32(129, 140, 248, A), trackH * 0.5f);

    if (active || hovered) {
        dl->AddCircleFilled(ImVec2(fillEnd, trackY), thumbR + 4.0f,
                            IM_COL32(129, 140, 248, (int)(60 * alpha)), 24);
    }
    dl->AddCircleFilled(ImVec2(fillEnd, trackY), thumbR,
                        IM_COL32(255, 255, 255, A), 24);
    dl->AddCircle(ImVec2(fillEnd, trackY), thumbR,
                  IM_COL32(0, 0, 0, (int)(40 * alpha)), 24, 1.0f);

    char buf[32];
    std::snprintf(buf, sizeof(buf), fmt, *v);
    ImVec2 textSize = ImGui::CalcTextSize(buf);
    float textX = cur.x + sliderW + gap;
    float textY = cur.y + (h - textSize.y) * 0.5f;
    dl->AddText(ImVec2(textX, textY),
                IM_COL32(232, 230, 240, A), buf);

    return changed;
}

static bool colorSwatch(const char* id, int rgb[3]) {
    ImVec4 col(rgb[0] / 255.0f, rgb[1] / 255.0f, rgb[2] / 255.0f, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    bool clicked = ImGui::ColorButton(id, col,
        ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,
        ImVec2(28.0f, 22.0f));
    ImGui::PopStyleVar();

    char popupId[32];
    std::snprintf(popupId, sizeof(popupId), "%s_p", id);
    if (clicked) ImGui::OpenPopup(popupId);
    bool changed = false;
    if (ImGui::BeginPopup(popupId)) {
        float c[3] = { col.x, col.y, col.z };
        if (ImGui::ColorPicker3(id, c,
            ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview)) {
            rgb[0] = (int)(c[0] * 255.0f + 0.5f);
            rgb[1] = (int)(c[1] * 255.0f + 0.5f);
            rgb[2] = (int)(c[2] * 255.0f + 0.5f);
            if (rgb[0] < 0) rgb[0] = 0; if (rgb[0] > 255) rgb[0] = 255;
            if (rgb[1] < 0) rgb[1] = 0; if (rgb[1] > 255) rgb[1] = 255;
            if (rgb[2] < 0) rgb[2] = 0; if (rgb[2] > 255) rgb[2] = 255;
            changed = true;
        }
        ImGui::EndPopup();
    }
    return changed;
}

// ---- Section renderers -------------------------------------------------------

static void renderCheckerGrid(CanvasOptions& opt) {
    paneTitle("Checker Grid");
    float controlW = ImGui::GetContentRegionAvail().x - kLabelW;

    rowLabel("Show");
    toggleSwitch("##chk_show", &opt.checkerVisible);

    if (!opt.checkerVisible) ImGui::BeginDisabled();

    rowLabel("Cell size");
    slimSlider("##chk_size", &opt.checkerSize, 2, 128, controlW, "%d px");

    rowLabel("Colors");
    colorSwatch("##chk_light", opt.checkerLight);
    ImGui::SameLine(0.0f, 6.0f);
    colorSwatch("##chk_dark", opt.checkerDark);

    rowLabel("");
    if (GhostButton("Reset to defaults")) {
        opt.checkerSize     = 16;
        opt.checkerLight[0] = 31; opt.checkerLight[1] = 31; opt.checkerLight[2] = 46;
        opt.checkerDark[0]  = 24; opt.checkerDark[1]  = 24; opt.checkerDark[2]  = 38;
    }

    if (!opt.checkerVisible) ImGui::EndDisabled();
}

static void renderGridSnap(CanvasOptions& opt) {
    paneTitle("Grid Snap");
    float controlW = ImGui::GetContentRegionAvail().x - kLabelW;

    rowLabel("Enable");
    toggleSwitch("##snap_en", &opt.gridSnapEnabled);

    hintLine("Snaps create / move / resize to step multiples.\n"
             "Hold Cmd / Ctrl while dragging for whole-pixel snap.");

    if (!opt.gridSnapEnabled) ImGui::BeginDisabled();

    rowLabel("Step X");
    slimSlider("##snap_x", &opt.gridSnapX, 1, 256, controlW, "%d px");

    rowLabel("Step Y");
    slimSlider("##snap_y", &opt.gridSnapY, 1, 256, controlW, "%d px");

    rowLabel("");
    if (GhostButton("Match checker cell")) {
        opt.gridSnapX = opt.checkerSize;
        opt.gridSnapY = opt.checkerSize;
    }

    if (!opt.gridSnapEnabled) ImGui::EndDisabled();

    if (opt.gridSnapX < 1) opt.gridSnapX = 1;
    if (opt.gridSnapY < 1) opt.gridSnapY = 1;
}

static void renderPixelGrid(CanvasOptions& opt) {
    paneTitle("Pixel Grid");
    float controlW = ImGui::GetContentRegionAvail().x - kLabelW;

    rowLabel("Show");
    toggleSwitch("##px_show", &opt.pixelGridVisible);

    hintLine("Hairline grid at every integer pixel boundary.");

    if (!opt.pixelGridVisible) ImGui::BeginDisabled();

    rowLabel("Min zoom");
    slimSliderFloat("##px_zoom", &opt.pixelGridZoomMin, 1.0f, 32.0f, controlW, "%.1fx");

    if (!opt.pixelGridVisible) ImGui::EndDisabled();

    if (opt.pixelGridZoomMin < 1.0f)  opt.pixelGridZoomMin = 1.0f;
    if (opt.pixelGridZoomMin > 32.0f) opt.pixelGridZoomMin = 32.0f;
}

struct Section {
    const char* title;
    void (*render)(CanvasOptions&);
};

static const Section kSections[] = {
    { "Checker Grid", renderCheckerGrid },
    { "Grid Snap",    renderGridSnap    },
    { "Pixel Grid",   renderPixelGrid   },
};
static const int kNumSections = (int)(sizeof(kSections) / sizeof(kSections[0]));

// ---- Panel -------------------------------------------------------------------

void CanvasOptionsPanel::draw(Editor& editor) {
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(620.0f, 440.0f), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking
                           | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(10.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(8.0f,  6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding,  4.0f);

    bool open = ImGui::Begin("Canvas Options", &visible, flags);

    ImGui::PopStyleVar(5);

    if (!open) {
        ImGui::End();
        return;
    }

    CanvasOptions& opt = editor.canvasOptions;

    // ===== Left sidebar (category list) =====
    // Slightly darker background for the sidebar, like Photoshop's preferences.
    ImVec4 sidebarBg = ImVec4(0.039f, 0.039f, 0.059f, 1.0f);  // #0A0A0F
    ImGui::PushStyleColor(ImGuiCol_ChildBg, sidebarBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 14.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(0.0f, 5.0f));
    ImGui::BeginChild("##cv_sidebar", ImVec2(kSidebarW, 0.0f), 0);

    // Custom row: full-width background with inset text.
    const float rowPadX = 12.0f;
    const float rowPadY = 9.0f;
    float rowW = ImGui::GetContentRegionAvail().x;
    float fontH = ImGui::GetFontSize();
    float rowH = fontH + rowPadY * 2.0f;

    for (int i = 0; i < kNumSections; ++i) {
        bool sel = (i == selectedSection);
        char id[64];
        std::snprintf(id, sizeof(id), "##cat%d", i);

        ImVec2 cur = ImGui::GetCursorScreenPos();
        if (ImGui::InvisibleButton(id, ImVec2(rowW, rowH))) {
            selectedSection = i;
        }
        bool hovered = ImGui::IsItemHovered();
        bool active  = ImGui::IsItemActive();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 bMin = cur;
        ImVec2 bMax = ImVec2(cur.x + rowW, cur.y + rowH);

        ImU32 bg = 0;
        if (sel) {
            bg = IM_COL32(33, 36, 64, 255);  // ~indigo @ low alpha on bg-input
        } else if (active) {
            bg = IM_COL32(28, 28, 42, 255);
        } else if (hovered) {
            bg = IM_COL32(22, 22, 34, 255);
        }
        if (bg != 0) dl->AddRectFilled(bMin, bMax, bg, 4.0f);

        if (sel) {
            // Left accent stripe
            ImU32 acc = IM_COL32(129, 140, 248, 255);  // #818CF8
            dl->AddRectFilled(
                ImVec2(bMin.x, bMin.y + 4.0f),
                ImVec2(bMin.x + 3.0f, bMax.y - 4.0f),
                acc, 1.5f);
        }

        ImU32 textCol = sel
            ? IM_COL32(232, 230, 240, 255)   // ink
            : IM_COL32(176, 171, 189, 255);  // ink-2
        float textY = cur.y + (rowH - fontH) * 0.5f;
        dl->AddText(ImVec2(cur.x + rowPadX, textY), textCol, kSections[i].title);
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    ImGui::SameLine(0.0f, 0.0f);

    // ===== Right pane (selected section content) =====
    ImGui::BeginChild("##cv_pane", ImVec2(0.0f, 0.0f), 0);
    ImGui::Dummy(ImVec2(0.0f, 14.0f));
    ImGui::Indent(20.0f);

    if (selectedSection >= 0 && selectedSection < kNumSections) {
        kSections[selectedSection].render(opt);
    }

    ImGui::Unindent(20.0f);
    ImGui::EndChild();

    ImGui::End();

    // Auto-persist on any change.
    if (!snapshotInitialized) {
        lastSavedSnapshot = opt;
        snapshotInitialized = true;
    } else if (optionsDiffer(opt, lastSavedSnapshot)) {
        opt.save();
        lastSavedSnapshot = opt;
    }
}
