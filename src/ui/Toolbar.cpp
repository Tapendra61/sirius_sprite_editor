#include "ui/Toolbar.h"

#include <cfloat>
#include <cstdio>
#include "app/Editor.h"
#include "ui/Theme.h"
#include "imgui.h"
#include "imgui_internal.h"

Toolbar::Toolbar() : mode(ToolMode::Select) {
}

Toolbar::~Toolbar() {
}

static const ImU32 ACCENT_SEL       = IM_COL32(129, 140, 248, 255);
static const ImU32 ACCENT_SEL_SOFT  = IM_COL32(129, 140, 248,  38);
static const ImU32 INK              = IM_COL32(225, 225, 235, 255);
static const ImU32 INK_3            = IM_COL32(122, 116, 144, 255);
static const ImU32 BG_HOVER         = IM_COL32(40, 40, 60, 255);
static const ImU32 BG_PRESS         = IM_COL32(50, 50, 70, 255);
static const ImU32 BG_TOOLBAR       = IM_COL32(26, 26, 38, 255);

static bool toolButton(const char* label, const char* shortcut, Texture2D icon, bool active) {
    const float padX     = 10.0f;
    const float iconSize = 16.0f;
    const float gap      = 6.0f;
    const float h        = ImGui::GetFrameHeight();

    ImVec2 labelSize = ImGui::CalcTextSize(label);

    float monoSize = g_FontMono ? g_FontMono->LegacySize : ImGui::GetFontSize();
    ImVec2 shortSize = g_FontMono
        ? g_FontMono->CalcTextSizeA(monoSize, FLT_MAX, 0.0f, shortcut)
        : ImGui::CalcTextSize(shortcut);
    bool   hasIcon   = icon.id != 0;

    float w = padX + labelSize.x + gap + (hasIcon ? iconSize + gap : 0.0f) + shortSize.x + padX;

    char id[64];
    std::snprintf(id, sizeof(id), "##mode_%s", label);

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, ImVec2(w, h));
    bool hovered = ImGui::IsItemHovered();
    bool pressed = ImGui::IsItemActive();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 bMin = cursor;
    ImVec2 bMax = ImVec2(cursor.x + w, cursor.y + h);

    ImU32 bg = 0;
    if (active)        bg = ACCENT_SEL_SOFT;
    else if (pressed)  bg = BG_PRESS;
    else if (hovered)  bg = BG_HOVER;
    if (bg != 0) dl->AddRectFilled(bMin, bMax, bg, 2.0f);
    if (active)  dl->AddRect(bMin, bMax, ACCENT_SEL, 2.0f);

    float textY = cursor.y + (h - ImGui::GetFontSize()) * 0.5f;
    float monoY = cursor.y + (h - monoSize) * 0.5f;
    float iconY = cursor.y + (h - iconSize) * 0.5f;
    ImU32 textCol = active ? ACCENT_SEL : INK;

    float x = cursor.x + padX;
    dl->AddText(ImVec2(x, textY), textCol, label);
    x += labelSize.x + gap;

    if (hasIcon) {
        dl->AddImage((ImTextureID)(intptr_t)icon.id,
                     ImVec2(x, iconY), ImVec2(x + iconSize, iconY + iconSize));
        x += iconSize + gap;
    }

    if (g_FontMono) {
        dl->AddText(g_FontMono, monoSize, ImVec2(x, monoY), INK_3, shortcut);
    } else {
        dl->AddText(ImVec2(x, textY), INK_3, shortcut);
    }
    return clicked;
}

static bool actionButton(const char* label,
                         Texture2D cmdIcon, Texture2D shiftIcon,
                         char keyLetter, bool useShift, bool disabled) {
    const float padX     = 10.0f;
    const float iconSize = 14.0f;
    const float gap      = 4.0f;
    const float labelGap = 12.0f;
    const float h        = ImGui::GetFrameHeight();

    ImVec2 labelSize = ImGui::CalcTextSize(label);
    char keyStr[2] = { keyLetter, '\0' };
    float monoSize = g_FontMono ? g_FontMono->LegacySize : ImGui::GetFontSize();
    ImVec2 keySize = g_FontMono
        ? g_FontMono->CalcTextSizeA(monoSize, FLT_MAX, 0.0f, keyStr)
        : ImGui::CalcTextSize(keyStr);

    bool hasCmd   = cmdIcon.id != 0;
    bool hasShift = useShift && shiftIcon.id != 0;

    float chordW = (hasShift ? iconSize + gap : 0.0f)
                 + (hasCmd   ? iconSize + gap : 0.0f)
                 + keySize.x;
    float w = padX + labelSize.x + labelGap + chordW + padX;

    char id[64];
    std::snprintf(id, sizeof(id), "##act_%s", label);

    if (disabled) ImGui::BeginDisabled();

    ImVec2 cursor = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, ImVec2(w, h));
    bool hovered = ImGui::IsItemHovered();
    bool pressed = ImGui::IsItemActive();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 bMin = cursor;
    ImVec2 bMax = ImVec2(cursor.x + w, cursor.y + h);

    ImU32 bg = 0;
    if (pressed)      bg = BG_PRESS;
    else if (hovered) bg = BG_HOVER;
    if (bg != 0) dl->AddRectFilled(bMin, bMax, bg, 2.0f);

    float textY = cursor.y + (h - ImGui::GetFontSize()) * 0.5f;
    float monoY = cursor.y + (h - monoSize) * 0.5f;
    float iconY = cursor.y + (h - iconSize) * 0.5f;
    ImU32 textCol = disabled ? INK_3 : INK;

    float x = cursor.x + padX;
    dl->AddText(ImVec2(x, textY), textCol, label);
    x += labelSize.x + labelGap;

    if (hasShift) {
        dl->AddImage((ImTextureID)(intptr_t)shiftIcon.id,
                     ImVec2(x, iconY), ImVec2(x + iconSize, iconY + iconSize));
        x += iconSize + gap;
    }
    if (hasCmd) {
        dl->AddImage((ImTextureID)(intptr_t)cmdIcon.id,
                     ImVec2(x, iconY), ImVec2(x + iconSize, iconY + iconSize));
        x += iconSize + gap;
    }
    if (g_FontMono) {
        dl->AddText(g_FontMono, monoSize, ImVec2(x, monoY), INK_3, keyStr);
    } else {
        dl->AddText(ImVec2(x, textY), INK_3, keyStr);
    }

    if (disabled) ImGui::EndDisabled();

    return clicked;
}

static void verticalSep() {
    ImGui::SameLine(0.0f, 8.0f);
    float h = ImGui::GetFrameHeight() * 0.7f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos.y += (ImGui::GetFrameHeight() - h) * 0.5f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 col = ImGui::GetColorU32(ImGuiCol_Border);
    dl->AddLine(pos, ImVec2(pos.x, pos.y + h), col, 1.0f);
    ImGui::Dummy(ImVec2(1.0f, ImGui::GetFrameHeight()));
    ImGui::SameLine(0.0f, 8.0f);
}

void Toolbar::draw(Editor& editor) {
    float h = ImGui::GetFrameHeight() + 24.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(BG_TOOLBAR));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 12.0f));
    bool open = ImGui::BeginViewportSideBar("##Toolbar", ImGui::GetMainViewport(),
                                              ImGuiDir_Up, h, flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    if (!open) {
        ImGui::End();
        return;
    }

    if (toolButton("Select", "V", editor.iconToolSelect, mode == ToolMode::Select)) {
        mode = ToolMode::Select;
    }
    ImGui::SameLine();
    if (toolButton("Move",   "H", editor.iconToolMove,   mode == ToolMode::Move)) {
        mode = ToolMode::Move;
    }
    ImGui::SameLine();
    if (toolButton("Rect",   "R", editor.iconToolRect,   mode == ToolMode::Rectangle)) {
        mode = ToolMode::Rectangle;
    }

    verticalSep();

    bool hasImage  = editor.project.isImageLoaded();
    if (actionButton("Grid Slice", editor.iconCmd, editor.iconShift,
                     'G', false, !hasImage)) {
        editor.gridModal.open();
    }
    ImGui::SameLine();
    if (actionButton("Auto Slice", editor.iconCmd, editor.iconShift,
                     'A', true, !hasImage)) {
        editor.autoModal.open();
    }

    verticalSep();

    bool hasSlices = editor.project.slices.count() > 0;
    if (actionButton("Export", editor.iconCmd, editor.iconShift,
                     'E', false, !hasSlices)) {
        editor.exportModal.open();
    }

    ImGui::End();
}
