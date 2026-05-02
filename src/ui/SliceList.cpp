#include "ui/SliceList.h"

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

void SliceList::draw(Editor& editor) {
    ImGui::Begin("Slices");

    int total = editor.project.slices.count();
    int selCount = editor.project.slices.selectionCount();

    if (total == 0) {
        ImGui::TextColored(INK_3, "No Slices Created");
        ImGui::End();
        return;
    }

    ImGui::TextColored(INK_3, "%d slice%s%s%d selected",
                       total, total == 1 ? "" : "s",
                       selCount > 0 ? "  \xc2\xb7  " : "  \xc2\xb7  ",
                       selCount);
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
