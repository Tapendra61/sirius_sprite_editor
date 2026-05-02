#ifndef SIRIUS_PALETTE_H
#define SIRIUS_PALETTE_H

#include "imgui.h"

// Shared color tokens (mirror of design tokens in docs/project_plan/ui_html).
// Defined once in Palette.cpp; reference everywhere via these globals.
namespace pal {

// Ink (foreground text)
extern const ImVec4 INK;        // #E8E6F0  primary
extern const ImVec4 INK_2;      // #B0ABBD  secondary / labels
extern const ImVec4 INK_3;      // #7A7490  muted / hints

// Accent
extern const ImVec4 ACCENT;     // #818CF8  selection indigo
extern const ImVec4 PIVOT;      // #F59E6B  pivot orange / dirty marker

// Pre-converted U32 versions for ImDrawList::Add* calls.
extern const ImU32 INK_U32;
extern const ImU32 INK2_U32;
extern const ImU32 INK3_U32;
extern const ImU32 ACCENT_U32;
extern const ImU32 PIVOT_U32;

}  // namespace pal

// Reusable widgets that read from the palette.
bool GhostButton(const char* label);

#endif
