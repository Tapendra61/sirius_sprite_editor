#include "ui/Toolbar.h"

#include "app/Editor.h"
#include "imgui.h"

Toolbar::Toolbar() : mode(ToolMode::Select) {
}

Toolbar::~Toolbar() {
}

void Toolbar::draw(Editor& editor) {
    (void)editor;
    ImGui::Begin("Toolbar");
    ImGui::TextDisabled("(phase 3 - mode buttons)");
    ImGui::End();
}
