#include "ui/Palette.h"

namespace pal {

const ImVec4 INK    = ImVec4(0.910f, 0.902f, 0.941f, 1.0f);  // #E8E6F0
const ImVec4 INK_2  = ImVec4(0.690f, 0.671f, 0.741f, 1.0f);  // #B0ABBD
const ImVec4 INK_3  = ImVec4(0.478f, 0.455f, 0.565f, 1.0f);  // #7A7490
const ImVec4 ACCENT = ImVec4(0.506f, 0.549f, 0.973f, 1.0f);  // #818CF8
const ImVec4 PIVOT  = ImVec4(0.961f, 0.620f, 0.420f, 1.0f);  // #F59E6B

const ImU32 INK_U32    = IM_COL32(232, 230, 240, 255);
const ImU32 INK2_U32   = IM_COL32(176, 171, 189, 255);
const ImU32 INK3_U32   = IM_COL32(122, 116, 144, 255);
const ImU32 ACCENT_U32 = IM_COL32(129, 140, 248, 255);
const ImU32 PIVOT_U32  = IM_COL32(245, 158, 107, 255);

}  // namespace pal

bool GhostButton(const char* label) {
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.06f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4(pal::ACCENT.x, pal::ACCENT.y, pal::ACCENT.z, 0.20f));
    ImGui::PushStyleColor(ImGuiCol_Border,
                          ImVec4(0.21f, 0.21f, 0.31f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, pal::INK_2);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,    ImVec2(10.0f, 4.0f));
    bool r = ImGui::Button(label);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);
    return r;
}
