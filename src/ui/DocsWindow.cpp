#include "ui/DocsWindow.h"

#include "imgui.h"
#include "ui/Theme.h"

DocsWindow::DocsWindow() : visible(false), selectedSection(0) {
}

DocsWindow::~DocsWindow() {
}

void DocsWindow::show() {
    visible = true;
}

static const ImVec4 INK_2 = ImVec4(0.690f, 0.671f, 0.741f, 1.0f);
static const ImVec4 INK_3 = ImVec4(0.478f, 0.455f, 0.565f, 1.0f);
static const ImVec4 ACCENT = ImVec4(0.506f, 0.549f, 0.973f, 1.0f); // #818CF8

static void heading(const char* text) {
    ImGui::Spacing();
    ImGui::TextColored(ACCENT, "%s", text);
    ImGui::Separator();
    ImGui::Spacing();
}

static void subhead(const char* text) {
    ImGui::Spacing();
    ImGui::TextColored(INK_2, "%s", text);
}

static void bullet(const char* fmt, ...) {
    ImGui::Bullet();
    va_list args;
    va_start(args, fmt);
    ImGui::TextWrappedV(fmt, args);
    va_end(args);
}

static void key(const char* k) {
    if (g_FontMono) ImGui::PushFont(g_FontMono);
    ImGui::TextColored(INK_2, "%s", k);
    if (g_FontMono) ImGui::PopFont();
}

static void keyRow(const char* k, const char* desc) {
    if (g_FontMono) ImGui::PushFont(g_FontMono);
    ImGui::TextColored(INK_2, "%-22s", k);
    if (g_FontMono) ImGui::PopFont();
    ImGui::SameLine(0.0f, 12.0f);
    ImGui::TextWrapped("%s", desc);
}

// ---- Section renderers ----

static void renderGettingStarted() {
    heading("Getting Started");
    ImGui::TextWrapped(
        "Sirius is a sprite-sheet slicer. Load an image, define rectangles "
        "around individual sprites, and export those rectangles as JSON, PNG, "
        "or CSV for your game engine.");

    subhead("Quick start");
    bullet("Open an image: File \xe2\x86\x92 Open Image, or Ctrl+Shift+I");
    bullet("Pick the Rectangle tool (R) and drag on the canvas to make a slice");
    bullet("Save your work as a project: File \xe2\x86\x92 Save (Ctrl+S)");
    bullet("Project files use the .srsprite extension");
    bullet("Recent projects are listed under File \xe2\x86\x92 Open Recent");
}

static void renderTools() {
    heading("Tools");
    ImGui::TextWrapped(
        "The toolbar at the top of the window holds three tools. Pick a tool "
        "with the toolbar buttons or its keyboard shortcut.");

    subhead("Select (V)");
    bullet("Click a slice to select it");
    bullet("Click empty space and drag to marquee-select multiple slices");
    bullet("Shift / Cmd / Ctrl-click to add or remove from selection");
    bullet("Click overlapping slices repeatedly to cycle through stacked slices");

    subhead("Move (H)");
    bullet("Left-click drag pans the canvas (an alternative to scroll-pan)");

    subhead("Rectangle (R)");
    bullet("Drag on the canvas to draw a new slice rectangle");
    bullet("Hold Cmd / Ctrl while dragging to snap to integer pixels");
}

static void renderSlicing() {
    heading("Slicing an Image");
    ImGui::TextWrapped(
        "Beyond manual rectangles, two batch operations let you cut a sheet "
        "into many slices at once.");

    subhead("Grid Slice (Ctrl+G)");
    bullet("By Size: specify cell width / height; Sirius tiles the image");
    bullet("By Count: specify columns and rows; Sirius computes cell size");
    bullet("Offset shifts the grid origin from the top-left of the image");
    bullet("Padding adds gaps between cells");
    bullet("Replace existing slices: clear current slices before adding");

    subhead("Auto Slice (Ctrl+Shift+A)");
    bullet("Detects sprites by alpha-channel connectivity");
    bullet("Alpha threshold: pixels with alpha \xe2\x89\xa4 threshold are background");
    bullet("Min size: discards slices smaller than this on either axis");
    bullet("Merge distance: combines blobs that sit within N pixels of each other");
    bullet("Padding: adds N pixels around each detected blob");
    bullet("8-connected: diagonal pixels count as part of the same sprite");

    subhead("Naming patterns");
    ImGui::TextWrapped(
        "Both modals have a Name Pattern field with token expansion:");
    ImGui::Indent();
    keyRow("{i}", "1-based index, zero-padded to the total count");
    keyRow("{r}", "row number (Grid Slice only, 1-based)");
    keyRow("{c}", "column number (Grid Slice only, 1-based)");
    ImGui::Unindent();
    ImGui::Spacing();
    ImGui::TextWrapped("Example: \"walk_{i}\" \xe2\x86\x92 walk_01, walk_02, walk_03 \xe2\x80\xa6");
}

static void renderEditing() {
    heading("Editing Slices");

    subhead("On the canvas");
    bullet("Drag a slice's body to move it");
    bullet("Drag a corner or edge handle to resize");
    bullet("Hold Cmd / Ctrl while dragging to snap to integer pixels");
    bullet("Resizing a slice below 1\xc3\x97" "1 deletes it");

    subhead("Inspector panel");
    bullet("Identity: rename, see slice ID");
    bullet("Geometry: X / Y / W / H drag-to-scrub fields");
    bullet("Pivot: 3\xc3\x97" "3 preset grid plus custom 0\xe2\x80\x93" "1 floats");
    bullet("9-Slice Border: L / T / R / B inset values");
    bullet("Multi-select shows mixed values as \xe2\x80\x94");

    subhead("Operations");
    keyRow("Ctrl+Z",       "Undo");
    keyRow("Ctrl+Shift+Z", "Redo");
    keyRow("Ctrl+D",       "Duplicate selected (offset by 8 px)");
    keyRow("Del",          "Delete selected");
    keyRow("Esc",          "Deselect");
    keyRow("Ctrl+A",       "Select all");
}

static void renderSliceList() {
    heading("Slice List Panel");
    ImGui::TextWrapped(
        "The Slices panel on the right shows every slice in the project with "
        "a thumbnail, name, and pixel size.");

    subhead("Interactions");
    bullet("Click a row to select; Shift / Cmd-click to toggle");
    bullet("Double-click the name to rename inline");
    bullet("Right-click any row for the context menu");

    subhead("Right-click menu");
    bullet("Rename \xe2\x80\x94 enter inline edit mode");
    bullet("Duplicate \xe2\x80\x94 copy the slice with an 8 px offset");
    bullet("Trim Transparent Edges \xe2\x80\x94 shrink the rect to fit non-transparent pixels");
    bullet("Delete \xe2\x80\x94 remove with undo support");
}

static void renderNavigation() {
    heading("Canvas Navigation");

    subhead("Pan");
    bullet("Two-finger swipe on a trackpad");
    bullet("Middle-mouse drag");
    bullet("Move tool (H) + left-mouse drag");

    subhead("Zoom");
    bullet("Cmd / Ctrl + scroll wheel: zoom toward cursor");
    bullet("Trackpad pinch may not be available; use modifier+scroll instead");

    subhead("Zoom shortcuts");
    keyRow("F", "Zoom to fit (image fills the canvas)");
    keyRow("1", "Zoom to 100%");
    keyRow("=", "Zoom in");
    keyRow("-", "Zoom out");
}

static void renderTrim() {
    heading("Trim Transparent Edges");
    ImGui::TextWrapped(
        "Shrinks each selected slice's rectangle to the bounding box of its "
        "non-transparent pixels. Useful when grid-sliced cells have lots of "
        "empty padding.");

    subhead("Where to find it");
    bullet("Slice menu \xe2\x86\x92 Trim Selected");
    bullet("Right-click on a slice in the list panel");
    bullet("Right-click on the canvas");

    subhead("Notes");
    bullet("Requires an image to be loaded");
    bullet("Slices that are entirely transparent are left unchanged");
    bullet("Operation is fully undoable (Ctrl+Z)");
}

static void renderExport() {
    heading("Exporting");
    ImGui::TextWrapped(
        "File \xe2\x86\x92 Export... opens the export modal. Pick a format, choose "
        "an output folder, and apply.");

    subhead("Formats");
    bullet("Atlas JSON \xe2\x80\x94 a single JSON file with every slice's rect, pivot, and 9-slice border");
    bullet("Per-Slice PNG \xe2\x80\x94 one image file per slice, named after the slice");
    bullet("CSV \xe2\x80\x94 tab-separated rows for spreadsheet workflows");

    subhead("Options");
    bullet("Selected only \xe2\x80\x94 export just the currently selected slices");
}

static void renderKeybindings() {
    heading("Keybindings");
    ImGui::TextWrapped(
        "Edit \xe2\x86\x92 Keybindings\xe2\x80\xa6 opens a remapping panel. Click any "
        "binding's chord button to record a new chord.");

    subhead("Storage");
    bullet("Custom bindings persist to keybindings.json next to the executable");
    bullet("Reset arrows on each row restore that row's default");
    bullet("Chord recording captures Ctrl, Shift, Alt, and Cmd modifiers");

    subhead("Default highlights");
    keyRow("Ctrl+O",        "Open Project");
    keyRow("Ctrl+S",        "Save");
    keyRow("Ctrl+Shift+S",  "Save As");
    keyRow("Ctrl+Shift+I",  "Open Image");
    keyRow("Ctrl+E",        "Export");
    keyRow("Ctrl+G",        "Grid Slice");
    keyRow("Ctrl+Shift+A",  "Auto Slice");
    keyRow("Ctrl+Z / Y",    "Undo / Redo");
    keyRow("V / H / R",     "Select / Move / Rectangle tool");
    keyRow("F / 1 / = / -", "Zoom fit / 100% / in / out");
}

static void renderFiles() {
    heading("Project Files");

    subhead(".srsprite");
    bullet("The project file. JSON describing the image path, slices, and camera state");
    bullet("Save: Ctrl+S, Save As: Ctrl+Shift+S");
    bullet("Title bar shows the current file name; an asterisk marks unsaved changes");

    subhead("Sidecar files (kept next to the executable)");
    bullet("keybindings.json \xe2\x80\x94 custom keyboard shortcuts");
    bullet("recent_files.json \xe2\x80\x94 the Open Recent list");
}

static void renderTips() {
    heading("Tips & Tricks");
    bullet("Click stacked slices: each click cycles to the slice underneath \xe2\x80\x94 useful for picking a small slice hidden under a big one");
    bullet("Hold Cmd / Ctrl during a drag to snap to whole pixels");
    bullet("Drag a label in the Inspector to scrub its value (no need to type)");
    bullet("Use Trim Transparent Edges right after Grid Slice to remove empty padding around each cell");
    bullet("The Slice menu's Trim Selected operates on the current selection \xe2\x80\x94 select-all + trim works as a single batch op");
    bullet("The Move tool just changes drag behavior \xe2\x80\x94 trackpad and middle-mouse pan still work in any tool");
}

struct Section {
    const char* title;
    void (*render)();
};

static const Section kSections[] = {
    { "Getting Started",   renderGettingStarted },
    { "Tools",             renderTools          },
    { "Slicing an Image",  renderSlicing        },
    { "Editing Slices",    renderEditing        },
    { "Slice List Panel",  renderSliceList      },
    { "Canvas Navigation", renderNavigation     },
    { "Trim Transparent",  renderTrim           },
    { "Exporting",         renderExport         },
    { "Keybindings",       renderKeybindings    },
    { "Project Files",     renderFiles          },
    { "Tips & Tricks",     renderTips           },
};
static const int kNumSections = (int)(sizeof(kSections) / sizeof(kSections[0]));

void DocsWindow::draw() {
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(820.0f, 620.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Documentation", &visible)) {
        ImGui::End();
        return;
    }

    // Sidebar nav
    ImGui::BeginChild("##doc_nav", ImVec2(220.0f, 0.0f), ImGuiChildFlags_Borders);
    ImGui::TextColored(INK_3, "Sections");
    ImGui::Separator();
    ImGui::Spacing();
    for (int i = 0; i < kNumSections; ++i) {
        bool sel = (i == selectedSection);
        if (ImGui::Selectable(kSections[i].title, sel)) {
            selectedSection = i;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Content
    ImGui::BeginChild("##doc_content", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
    if (selectedSection >= 0 && selectedSection < kNumSections) {
        kSections[selectedSection].render();
    }
    ImGui::EndChild();

    ImGui::End();
}
