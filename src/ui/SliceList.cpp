#include "ui/SliceList.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include "app/Editor.h"
#include "ui/Palette.h"
#include "ui/Theme.h"
#include "imgui.h"

SliceList::SliceList() {
}

SliceList::~SliceList() {
}

using pal::INK_3;

// Case-insensitive substring match. Empty `needle` matches everything.
static bool nameMatches(const std::string& name, const char* needle) {
    if (needle == nullptr || needle[0] == '\0') return true;
    if (name.empty()) return false;
    size_t hayLen = name.size();
    size_t nLen = std::strlen(needle);
    if (nLen > hayLen) return false;
    for (size_t i = 0; i + nLen <= hayLen; ++i) {
        bool ok = true;
        for (size_t j = 0; j < nLen; ++j) {
            unsigned char a = (unsigned char)name[i + j];
            unsigned char b = (unsigned char)needle[j];
            if (std::tolower(a) != std::tolower(b)) { ok = false; break; }
        }
        if (ok) return true;
    }
    return false;
}

// Pill-style search field with a leading magnifier glyph and a clear button on
// the right when the buffer is non-empty. Uses ImDrawList for the magnifier so
// it stays sharp at any DPI without needing an icon font.
static void drawSearchField(char* buf, size_t cap) {
    ImGuiStyle& style = ImGui::GetStyle();
    float h = ImGui::GetFontSize() + 12.0f;       // taller than default
    float availW = ImGui::GetContentRegionAvail().x;

    // Reserve space for an inline clear button when the buffer has text.
    bool hasText = buf[0] != '\0';
    float clearW = hasText ? (h - 4.0f) : 0.0f;
    float gap    = hasText ? 6.0f : 0.0f;
    float inputW = availW - clearW - gap;

    // Style: rounded pill, soft border, indigo focus ring.
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, h * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(h * 0.5f + 4.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        IM_COL32(15, 15, 22, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(20, 20, 30, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  IM_COL32(20, 20, 30, 255));
    ImGui::PushStyleColor(ImGuiCol_Border,         IM_COL32(42, 42, 61, 255));

    ImVec2 cur = ImGui::GetCursorScreenPos();

    ImGui::PushItemWidth(inputW);
    ImGui::InputTextWithHint("##slice_search", "Search slices...",
                             buf, cap);
    ImGui::PopItemWidth();

    // Magnifier glyph drawn over the input's left padding (purely visual).
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float gx = cur.x + 12.0f;
    float gy = cur.y + h * 0.5f;
    float r  = 4.5f;
    ImU32 glyph = IM_COL32(122, 116, 144, 255);  // ink-3
    dl->AddCircle(ImVec2(gx, gy), r, glyph, 16, 1.5f);
    dl->AddLine(ImVec2(gx + r * 0.7f, gy + r * 0.7f),
                ImVec2(gx + r * 0.7f + 4.0f, gy + r * 0.7f + 4.0f),
                glyph, 1.5f);

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);

    // Clear button (×).
    if (hasText) {
        ImGui::SameLine(0.0f, gap);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, (h - 4.0f) * 0.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Button,        IM_COL32(20, 20, 30, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(40, 40, 60, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  IM_COL32(50, 50, 70, 255));
        ImGui::PushStyleColor(ImGuiCol_Text,          IM_COL32(176, 171, 189, 255));
        if (ImGui::Button("\xc3\x97", ImVec2(clearW, h - 4.0f))) {  // ×
            buf[0] = '\0';
        }
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
    }
    (void)style;
}

void SliceList::draw(Editor& editor) {
    ImGui::Begin("Slices");

    int total = editor.project.slices.count();
    int selCount = editor.project.slices.selectionCount();

    if (total == 0) {
        searchBuf[0] = '\0';
        ImGui::TextColored(INK_3, "No Slices Created");
        ImGui::End();
        return;
    }

    // Search / filter input.
    drawSearchField(searchBuf, sizeof(searchBuf));
    ImGui::Spacing();

    bool searching = searchBuf[0] != '\0';

    // Pre-count matches so we can show "X of Y" when the search is active.
    int matchCount = 0;
    if (searching) {
        const std::vector<Slice>& all = editor.project.slices.slices;
        for (size_t i = 0; i < all.size(); ++i) {
            if (nameMatches(all[i].name, searchBuf)) ++matchCount;
        }
    }

    if (searching) {
        ImGui::TextColored(INK_3, "%d of %d slice%s  \xc2\xb7  %d selected",
                           matchCount, total,
                           total == 1 ? "" : "s", selCount);
    } else {
        ImGui::TextColored(INK_3, "%d slice%s  \xc2\xb7  %d selected",
                           total, total == 1 ? "" : "s", selCount);
    }
    ImGui::Separator();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg
                               | ImGuiTableFlags_NoBordersInBody
                               | ImGuiTableFlags_ScrollY;

    const float kThumbBox = 32.0f;
    bool hasImage = editor.project.isImageLoaded();

    if (ImGui::BeginTable("##slicelist", 3, tableFlags)) {
        ImGui::TableSetupColumn("##thumb", ImGuiTableColumnFlags_WidthFixed, kThumbBox + 4.0f);
        ImGui::TableSetupColumn("##name",  ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##size",  ImGuiTableColumnFlags_WidthFixed, 56.0f);

        const std::vector<Slice>& slices = editor.project.slices.slices;
        for (size_t i = 0; i < slices.size(); ++i) {
            const Slice& s = slices[i];
            if (searching && !nameMatches(s.name, searchBuf)) continue;
            bool sel = editor.project.slices.isSelected(s.id);

            ImGui::TableNextRow();

            // Col 0: thumbnail. Draw the image with ImDrawList (visual only),
            // then claim the full kThumbBox area with a Dummy for layout. This
            // avoids ImGui's "SetCursorPos extends boundaries — please add
            // Dummy" warning that bare SetCursorPos + Image triggers.
            ImGui::TableSetColumnIndex(0);
            if (hasImage && s.rect.width > 0 && s.rect.height > 0) {
                float ar = s.rect.width / s.rect.height;
                float dw = kThumbBox, dh = kThumbBox;
                if (ar > 1.0f) dh = kThumbBox / ar;
                else           dw = kThumbBox * ar;
                float padTop  = (kThumbBox - dh) * 0.5f;
                float padLeft = (kThumbBox - dw) * 0.5f;

                const Texture2D& tex = editor.project.texture;
                ImVec2 cur = ImGui::GetCursorScreenPos();
                ImVec2 imgMin(cur.x + padLeft, cur.y + padTop);
                ImVec2 imgMax(imgMin.x + dw, imgMin.y + dh);
                ImVec2 uv0(s.rect.x / (float)tex.width,
                           s.rect.y / (float)tex.height);
                ImVec2 uv1((s.rect.x + s.rect.width)  / (float)tex.width,
                           (s.rect.y + s.rect.height) / (float)tex.height);
                ImGui::GetWindowDrawList()->AddImage(
                    (ImTextureID)(intptr_t)tex.id, imgMin, imgMax, uv0, uv1);
            }
            ImGui::Dummy(ImVec2(kThumbBox, kThumbBox));

            // Col 1: name / rename / selectable
            ImGui::TableSetColumnIndex(1);

            if (renamingId == s.id) {
                char inputId[32];
                std::snprintf(inputId, sizeof(inputId), "##rename%d", s.id);
                if (renameFocusPending) {
                    ImGui::SetKeyboardFocusHere();
                    renameFocusPending = false;
                }
                ImGui::PushItemWidth(-1.0f);
                bool submit = ImGui::InputText(inputId, renameBuffer, sizeof(renameBuffer),
                                               ImGuiInputTextFlags_EnterReturnsTrue
                                               | ImGuiInputTextFlags_AutoSelectAll);
                ImGui::PopItemWidth();
                bool deactivated = ImGui::IsItemDeactivated();
                if (submit || deactivated) {
                    std::string newName(renameBuffer);
                    if (!newName.empty()) {
                        editor.renameSlice(s.id, newName);
                    }
                    renamingId = 0;
                }
            } else {
                char label[256];
                std::snprintf(label, sizeof(label), "%s##sl%d", s.name.c_str(), s.id);

                if (ImGui::Selectable(label, sel, ImGuiSelectableFlags_SpanAllColumns)) {
                    ImGuiIO& io = ImGui::GetIO();
                    bool additive = io.KeyShift || io.KeyCtrl || io.KeySuper;
                    if (additive) {
                        if (sel) editor.project.slices.selectRemove(s.id);
                        else     editor.project.slices.selectAdd(s.id);
                    } else {
                        editor.project.slices.selectOnly(s.id);
                    }
                }

                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    if (!sel) editor.project.slices.selectOnly(s.id);
                    contextMenuId = s.id;
                    ImGui::OpenPopup("##slice_ctx");
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    renamingId = s.id;
                    renameFocusPending = true;
                    std::strncpy(renameBuffer, s.name.c_str(), sizeof(renameBuffer) - 1);
                    renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                }
            }

            ImGui::TableSetColumnIndex(2);
            if (g_FontMono) ImGui::PushFont(g_FontMono);
            ImGui::TextColored(INK_3, "%d\xc3\x97%d",
                               (int)s.rect.width, (int)s.rect.height);
            if (g_FontMono) ImGui::PopFont();
        }

        if (ImGui::BeginPopup("##slice_ctx")) {
            int targetId = contextMenuId;
            const Slice* target = editor.project.slices.find(targetId);
            int selN = editor.project.slices.selectionCount();
            bool canRename = target != nullptr && selN <= 1;

            if (ImGui::MenuItem("Rename", nullptr, false, canRename)) {
                renamingId = targetId;
                renameFocusPending = true;
                std::strncpy(renameBuffer, target->name.c_str(), sizeof(renameBuffer) - 1);
                renameBuffer[sizeof(renameBuffer) - 1] = '\0';
            }
            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                editor.duplicateSelected();
            }
            if (ImGui::MenuItem("Trim Transparent Edges", nullptr, false,
                                editor.project.isImageLoaded())) {
                editor.trimSelected();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del")) {
                editor.deleteSelected();
            }
            ImGui::EndPopup();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
