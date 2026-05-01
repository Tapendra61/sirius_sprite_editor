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
    font = io.Fonts->AddFontFromFileTTF(SIRIUS_FONT_PATH, 13.0f, &cfg);
#endif

    if (font == nullptr) {
        io.Fonts->AddFontDefault();
    }
}

void ApplyDarkTheme() {
    ImGuiStyle& s = ImGui::GetStyle();

    // Spacing — dense per the design brief (Aseprite/Blender feel)
    s.WindowPadding     = ImVec2(8.0f, 8.0f);
    s.FramePadding      = ImVec2(8.0f, 4.0f);
    s.CellPadding       = ImVec2(4.0f, 3.0f);
    s.ItemSpacing       = ImVec2(6.0f, 4.0f);
    s.ItemInnerSpacing  = ImVec2(4.0f, 4.0f);
    s.IndentSpacing     = 16.0f;
    s.ScrollbarSize     = 10.0f;
    s.GrabMinSize       = 8.0f;

    // Borders — 1px hairlines, including on input frames per design
    s.WindowBorderSize  = 1.0f;
    s.ChildBorderSize   = 1.0f;
    s.PopupBorderSize   = 1.0f;
    s.FrameBorderSize   = 1.0f;
    s.TabBorderSize     = 0.0f;

    // Rounding — sharp chrome (0-2px), modals at 4px
    s.WindowRounding    = 0.0f;
    s.ChildRounding     = 0.0f;
    s.FrameRounding     = 2.0f;
    s.PopupRounding     = 4.0f;
    s.ScrollbarRounding = 2.0f;
    s.GrabRounding      = 2.0f;
    s.TabRounding       = 0.0f;

    // Alignment
    s.WindowTitleAlign         = ImVec2(0.0f, 0.5f);
    s.WindowMenuButtonPosition = ImGuiDir_Right;
    s.ButtonTextAlign          = ImVec2(0.5f, 0.5f);

    // Surface ladder
    ImVec4 bgCanvas    = RGB( 10,  10,  15, 255);  // #0A0A0F
    ImVec4 bgPanel     = RGB( 18,  18,  26, 255);  // #12121A
    ImVec4 bgToolbar   = RGB( 26,  26,  38, 255);  // #1A1A26
    ImVec4 bgInput     = RGB( 10,  10,  15, 255);  // #0A0A0F
    ImVec4 bgElevated  = RGB( 26,  26,  38, 255);  // #1A1A26 (modal body)
    ImVec4 bgRowHover  = RGB( 26,  26,  38, 255);  // #1A1A26
    ImVec4 bgRowSel    = RGB(129, 140, 248,  31);  // indigo @ ~12% (rgba(129,140,248,0.12))

    // Borders
    ImVec4 border       = RGB( 42,  42,  61, 255); // #2A2A3D
    ImVec4 border2      = RGB( 54,  54,  80, 255); // #363650
    ImVec4 borderStrong = RGB( 74,  74, 102, 255); // #4A4A66
    (void)borderStrong;

    // Ink
    ImVec4 ink         = RGB(232, 230, 240, 255); // #E8E6F0
    ImVec4 ink2        = RGB(176, 171, 189, 255); // #B0ABBD
    ImVec4 ink3        = RGB(122, 116, 144, 255); // #7A7490
    ImVec4 inkDisabled = RGB( 74,  70,  88, 255); // #4A4658
    (void)ink2; (void)ink3;

    // Accent — indigo selection
    ImVec4 accentSel       = RGB(129, 140, 248, 255); // #818CF8
    ImVec4 accentSelSoft   = RGB(129, 140, 248,  38); // ~15% alpha
    ImVec4 accentSelMid    = RGB(129, 140, 248,  90); // ~35%

    ImVec4 transparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    ImVec4* c = s.Colors;

    c[ImGuiCol_Text]                  = ink;
    c[ImGuiCol_TextDisabled]          = inkDisabled;
    c[ImGuiCol_WindowBg]              = bgPanel;
    c[ImGuiCol_ChildBg]               = bgPanel;
    c[ImGuiCol_PopupBg]               = bgElevated;
    c[ImGuiCol_Border]                = border;
    c[ImGuiCol_BorderShadow]          = transparent;

    c[ImGuiCol_FrameBg]               = bgInput;
    c[ImGuiCol_FrameBgHovered]        = RGB(15, 15, 22, 255);
    c[ImGuiCol_FrameBgActive]         = RGB(20, 20, 30, 255);

    c[ImGuiCol_TitleBg]               = bgToolbar;
    c[ImGuiCol_TitleBgActive]         = bgToolbar;
    c[ImGuiCol_TitleBgCollapsed]      = bgToolbar;

    c[ImGuiCol_MenuBarBg]             = bgToolbar;

    c[ImGuiCol_ScrollbarBg]           = transparent;
    c[ImGuiCol_ScrollbarGrab]         = border;
    c[ImGuiCol_ScrollbarGrabHovered]  = border2;
    c[ImGuiCol_ScrollbarGrabActive]   = accentSel;

    c[ImGuiCol_CheckMark]             = accentSel;
    c[ImGuiCol_SliderGrab]            = accentSel;
    c[ImGuiCol_SliderGrabActive]      = accentSel;

    c[ImGuiCol_Button]                = bgInput;
    c[ImGuiCol_ButtonHovered]         = bgToolbar;
    c[ImGuiCol_ButtonActive]          = accentSelSoft;

    c[ImGuiCol_Header]                = bgRowSel;
    c[ImGuiCol_HeaderHovered]         = bgRowHover;
    c[ImGuiCol_HeaderActive]          = accentSelSoft;

    c[ImGuiCol_Separator]             = border;
    c[ImGuiCol_SeparatorHovered]      = border2;
    c[ImGuiCol_SeparatorActive]       = accentSel;

    c[ImGuiCol_ResizeGrip]            = transparent;
    c[ImGuiCol_ResizeGripHovered]     = accentSelSoft;
    c[ImGuiCol_ResizeGripActive]      = accentSel;

    // Tabs — selected = inset (darker than panel)
    c[ImGuiCol_Tab]                       = bgPanel;
    c[ImGuiCol_TabHovered]                = bgToolbar;
    c[ImGuiCol_TabSelected]               = bgInput;
    c[ImGuiCol_TabSelectedOverline]       = transparent;
    c[ImGuiCol_TabDimmed]                 = bgPanel;
    c[ImGuiCol_TabDimmedSelected]         = bgInput;
    c[ImGuiCol_TabDimmedSelectedOverline] = transparent;

    c[ImGuiCol_DockingPreview]        = accentSelSoft;
    c[ImGuiCol_DockingEmptyBg]        = bgCanvas;

    c[ImGuiCol_PlotLines]             = ink;
    c[ImGuiCol_PlotLinesHovered]      = accentSel;
    c[ImGuiCol_PlotHistogram]         = accentSel;
    c[ImGuiCol_PlotHistogramHovered]  = accentSel;

    c[ImGuiCol_TableHeaderBg]         = bgToolbar;
    c[ImGuiCol_TableBorderStrong]     = border2;
    c[ImGuiCol_TableBorderLight]      = border;
    c[ImGuiCol_TableRowBg]            = transparent;
    c[ImGuiCol_TableRowBgAlt]         = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);

    c[ImGuiCol_TextSelectedBg]        = accentSelMid;
    c[ImGuiCol_DragDropTarget]        = accentSel;

    c[ImGuiCol_NavCursor]             = accentSel;
    c[ImGuiCol_NavWindowingHighlight] = transparent;
    c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
    c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
}

bool PrimaryButton(const char* label) {
    ImGui::PushStyleColor(ImGuiCol_Button,        RGB(129, 140, 248, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RGB(158, 167, 251, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  RGB(107, 119, 232, 255));
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    bool clicked = ImGui::Button(label);
    ImGui::PopStyleColor(4);
    return clicked;
}

bool DangerButton(const char* label) {
    ImGui::PushStyleColor(ImGuiCol_Button,        RGB(239, 95, 95, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RGB(245, 122, 122, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  RGB(218, 75, 75, 255));
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    bool clicked = ImGui::Button(label);
    ImGui::PopStyleColor(4);
    return clicked;
}
