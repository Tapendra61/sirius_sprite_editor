#include "ui/SliceList.h"

#include <cstdio>
#include "app/Editor.h"
#include "ui/Theme.h"
#include "imgui.h"

SliceList::SliceList() {
}

SliceList::~SliceList() {
}

static const ImVec4 INK_3 = ImVec4(0.478f, 0.455f, 0.565f, 1.0f);

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

    if (ImGui::BeginTable("##slicelist", 2, tableFlags)) {
        ImGui::TableSetupColumn("##name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##size", ImGuiTableColumnFlags_WidthFixed, 56.0f);

        const std::vector<Slice>& slices = editor.project.slices.slices;
        for (size_t i = 0; i < slices.size(); ++i) {
            const Slice& s = slices[i];
            bool sel = editor.project.slices.isSelected(s.id);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

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

            ImGui::TableSetColumnIndex(1);
            if (g_FontMono) ImGui::PushFont(g_FontMono);
            ImGui::TextColored(INK_3, "%d\xc3\x97%d",
                               (int)s.rect.width, (int)s.rect.height);
            if (g_FontMono) ImGui::PopFont();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
