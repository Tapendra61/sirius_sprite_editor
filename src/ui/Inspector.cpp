#include "ui/Inspector.h"

#include "app/Editor.h"
#include "imgui.h"

Inspector::Inspector() {
}

Inspector::~Inspector() {
}

void Inspector::draw(Editor& editor) {
    (void)editor;
    ImGui::Begin("Inspector");
    ImGui::TextDisabled("(phase 6 - slice properties)");
    ImGui::End();
}
