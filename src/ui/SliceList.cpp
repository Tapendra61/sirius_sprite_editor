#include "ui/SliceList.h"

#include "app/Editor.h"
#include "imgui.h"

SliceList::SliceList() {
}

SliceList::~SliceList() {
}

void SliceList::draw(Editor& editor) {
    (void)editor;
    ImGui::Begin("Slices");
    ImGui::TextDisabled("(phase 3 - slice list table)");
    ImGui::End();
}
