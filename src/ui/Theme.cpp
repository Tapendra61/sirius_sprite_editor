#include "ui/Theme.h"

#include "imgui.h"

static ImVec4 RGB(int r, int g, int b, int a) {
    ImVec4 c;
    c.x = r / 255.0f;
    c.y = g / 255.0f;
    c.z = b / 255.0f;
    c.w = a / 255.0f;
    return c;
}

void LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 2;
    cfg.PixelSnapH = false;

    ImFont* font = nullptr;
#ifdef SIRIUS_FONT_PATH
    font = io.Fonts->AddFontFromFileTTF(SIRIUS_FONT_PATH, 15.0f, &cfg);
#endif

    if (font == nullptr) {
        io.Fonts->AddFontDefault();
    }
}

void ApplyDarkTheme() {
    ImGuiStyle& s = ImGui::GetStyle();

    // Spacing — modern but still dense enough for a pro tool
    s.WindowPadding     = ImVec2(10.0f, 8.0f);
    s.FramePadding      = ImVec2(10.0f, 6.0f);
    s.CellPadding       = ImVec2(6.0f, 4.0f);
    s.ItemSpacing       = ImVec2(8.0f, 6.0f);
    s.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
    s.IndentSpacing     = 20.0f;
    s.ScrollbarSize     = 12.0f;
    s.GrabMinSize       = 10.0f;

    // Borders
    s.WindowBorderSize  = 1.0f;
    s.ChildBorderSize   = 1.0f;
    s.PopupBorderSize   = 1.0f;
    s.FrameBorderSize   = 0.0f;
    s.TabBorderSize     = 0.0f;

    // Rounding — subtle, modern
    s.WindowRounding    = 6.0f;
    s.ChildRounding     = 4.0f;
    s.FrameRounding     = 4.0f;
    s.PopupRounding     = 6.0f;
    s.ScrollbarRounding = 6.0f;
    s.GrabRounding      = 4.0f;
    s.TabRounding       = 4.0f;

    // Alignment
    s.WindowTitleAlign        = ImVec2(0.0f, 0.5f);
    s.WindowMenuButtonPosition = ImGuiDir_Right;
    s.ColorButtonPosition     = ImGuiDir_Right;
    s.ButtonTextAlign         = ImVec2(0.5f, 0.5f);

    // Color palette
    ImVec4 bgCanvas    = RGB(24, 27, 34, 255);
    ImVec4 bgPanel     = RGB(29, 32, 39, 255);
    ImVec4 bgElevated  = RGB(35, 39, 48, 255);
    ImVec4 bgInput     = RGB(17, 19, 26, 255);

    ImVec4 border         = RGB(44, 48, 58, 255);
    ImVec4 borderStrong   = RGB(58, 63, 75, 255);

    ImVec4 text         = RGB(225, 227, 232, 255);
    ImVec4 textDisabled = RGB(95, 100, 110, 255);

    ImVec4 accent        = RGB(91, 158, 255, 255);
    ImVec4 accentHover   = RGB(126, 179, 255, 255);
    ImVec4 accentActive  = RGB(74, 141, 232, 255);
    ImVec4 accentSubtle  = RGB(91, 158, 255, 64);
    ImVec4 accentMid     = RGB(91, 158, 255, 128);

    ImVec4 buttonBg    = RGB(40, 44, 54, 255);
    ImVec4 buttonHover = RGB(50, 55, 67, 255);

    ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    ImVec4* c = s.Colors;

    c[ImGuiCol_Text]                  = text;
    c[ImGuiCol_TextDisabled]          = textDisabled;
    c[ImGuiCol_WindowBg]              = bgPanel;
    c[ImGuiCol_ChildBg]               = bgPanel;
    c[ImGuiCol_PopupBg]               = bgElevated;
    c[ImGuiCol_Border]                = border;
    c[ImGuiCol_BorderShadow]          = transparent;

    c[ImGuiCol_FrameBg]               = bgInput;
    c[ImGuiCol_FrameBgHovered]        = RGB(28, 31, 41, 255);
    c[ImGuiCol_FrameBgActive]         = RGB(34, 38, 50, 255);

    c[ImGuiCol_TitleBg]               = bgPanel;
    c[ImGuiCol_TitleBgActive]         = bgElevated;
    c[ImGuiCol_TitleBgCollapsed]      = bgPanel;

    c[ImGuiCol_MenuBarBg]             = bgElevated;

    c[ImGuiCol_ScrollbarBg]           = transparent;
    c[ImGuiCol_ScrollbarGrab]         = border;
    c[ImGuiCol_ScrollbarGrabHovered]  = borderStrong;
    c[ImGuiCol_ScrollbarGrabActive]   = accent;

    c[ImGuiCol_CheckMark]             = accent;
    c[ImGuiCol_SliderGrab]            = accent;
    c[ImGuiCol_SliderGrabActive]      = accentActive;

    c[ImGuiCol_Button]                = buttonBg;
    c[ImGuiCol_ButtonHovered]         = buttonHover;
    c[ImGuiCol_ButtonActive]          = accent;

    c[ImGuiCol_Header]                = accentSubtle;
    c[ImGuiCol_HeaderHovered]         = accentMid;
    c[ImGuiCol_HeaderActive]          = accent;

    c[ImGuiCol_Separator]             = border;
    c[ImGuiCol_SeparatorHovered]      = accent;
    c[ImGuiCol_SeparatorActive]       = accentActive;

    c[ImGuiCol_ResizeGrip]            = transparent;
    c[ImGuiCol_ResizeGripHovered]     = accentSubtle;
    c[ImGuiCol_ResizeGripActive]      = accent;

    ImVec4 tabRaised  = bgElevated;
    ImVec4 tabHover   = RGB(48, 52, 64, 255);
    ImVec4 tabInset   = bgInput;

    c[ImGuiCol_Tab]                       = tabRaised;
    c[ImGuiCol_TabHovered]                = tabHover;
    c[ImGuiCol_TabSelected]               = tabInset;
    c[ImGuiCol_TabSelectedOverline]       = transparent;
    c[ImGuiCol_TabDimmed]                 = tabRaised;
    c[ImGuiCol_TabDimmedSelected]         = tabInset;
    c[ImGuiCol_TabDimmedSelectedOverline] = transparent;

    c[ImGuiCol_DockingPreview]        = accentSubtle;
    c[ImGuiCol_DockingEmptyBg]        = bgCanvas;

    c[ImGuiCol_PlotLines]             = text;
    c[ImGuiCol_PlotLinesHovered]      = accent;
    c[ImGuiCol_PlotHistogram]         = accent;
    c[ImGuiCol_PlotHistogramHovered]  = accentHover;

    c[ImGuiCol_TableHeaderBg]         = bgElevated;
    c[ImGuiCol_TableBorderStrong]     = borderStrong;
    c[ImGuiCol_TableBorderLight]      = border;
    c[ImGuiCol_TableRowBg]            = transparent;
    c[ImGuiCol_TableRowBgAlt]         = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);

    c[ImGuiCol_TextSelectedBg]        = accentMid;
    c[ImGuiCol_DragDropTarget]        = accent;

    c[ImGuiCol_NavCursor]             = accent;
    c[ImGuiCol_NavWindowingHighlight] = transparent;
    c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
    c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
}
