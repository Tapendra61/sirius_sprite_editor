#include "ui/AutoSliceModal.h"

#include <cstring>
#include <memory>
#include <string>
#include "app/Editor.h"
#include "commands/SliceCommands.h"
#include "imgui.h"
#include "ops/AutoSlicer.h"
#include "ui/SliceModalUtil.h"
#include "ui/Theme.h"

AutoSliceModal::AutoSliceModal()
    : showing(false),
      openRequested(false),
      alphaThreshold(0),
      minSize(4),
      mergeDistance(0),
      padding(0),
      eightConnected(true),
      replaceAll(true),
      needsRecompute(true) {
    std::strncpy(namePattern, "auto_{i}", sizeof(namePattern) - 1);
    namePattern[sizeof(namePattern) - 1] = '\0';
}

AutoSliceModal::~AutoSliceModal() {
}

void AutoSliceModal::open() {
    openRequested = true;
    needsRecompute = true;
}

void AutoSliceModal::draw(Editor& editor) {
    if (openRequested) {
        ImGui::OpenPopup("Auto Slice");
        openRequested = false;
    }

    showing = ImGui::IsPopupOpen("Auto Slice");

    if (!ImGui::BeginPopupModal("Auto Slice", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!showing) {
            preview.clear();
            cachedRects.clear();
        }
        return;
    }

    bool changed = false;
    changed |= ImGui::SliderInt("Alpha threshold", &alphaThreshold, 0, 255);
    changed |= ImGui::InputInt("Min size",        &minSize);
    changed |= ImGui::InputInt("Merge distance",  &mergeDistance);
    changed |= ImGui::InputInt("Padding",         &padding);
    changed |= ImGui::Checkbox("8-connected",     &eightConnected);

    ImGui::Separator();
    ImGui::InputText("Name Pattern", namePattern, sizeof(namePattern));
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Tokens: {i}=index");
    }
    ImGui::Checkbox("Replace existing slices", &replaceAll);

    if (alphaThreshold < 0)   { alphaThreshold = 0;   changed = true; }
    if (alphaThreshold > 255) { alphaThreshold = 255; changed = true; }
    if (minSize       < 1)    { minSize       = 1;   changed = true; }
    if (mergeDistance < 0)    { mergeDistance = 0;   changed = true; }
    if (padding       < 0)    { padding       = 0;   changed = true; }

    if (changed) needsRecompute = true;

    if (needsRecompute && editor.project.isImageLoaded()) {
        AutoSliceParams p;
        p.alphaThreshold = alphaThreshold;
        p.minSize        = minSize;
        p.mergeDistance  = mergeDistance;
        p.padding        = padding;
        p.eightConnected = eightConnected;
        cachedRects = autoSlice(editor.project.image, p);
        buildPreviewSlices(preview, cachedRects);
        needsRecompute = false;
    } else if (!editor.project.isImageLoaded()) {
        cachedRects.clear();
        preview.clear();
    }

    ImGui::Separator();
    if (!editor.project.isImageLoaded()) {
        ImGui::TextDisabled("Load an image first.");
    } else {
        ImGui::Text("Found %d sprites", (int)preview.size());
    }

    ImGui::Separator();

    bool cancelClicked = ImGui::Button("Cancel");
    ImGui::SameLine();

    bool canApply = !cachedRects.empty();
    if (!canApply) ImGui::BeginDisabled();
    bool applyClicked = PrimaryButton("Apply");
    if (!canApply) ImGui::EndDisabled();

    if (applyClicked) {
        std::vector<Slice> oldSlices = editor.project.slices.slices;
        std::vector<Slice> newSlices;
        if (!replaceAll) {
            newSlices = oldSlices;
        }
        std::string pattern(namePattern);
        if (pattern.empty()) pattern = "auto_{i}";
        int total = (int)cachedRects.size();
        for (size_t i = 0; i < cachedRects.size(); ++i) {
            Slice s;
            s.id = editor.project.nextId();
            s.name = expandNamePattern(pattern, (int)i + 1, 0, 0, total);
            s.rect = cachedRects[i];
            s.pivot = { 0.5f, 0.5f };
            s.border = { 0.0f, 0.0f, 0.0f, 0.0f };
            newSlices.push_back(s);
        }
        std::unique_ptr<ReplaceAllCommand> cmd(new ReplaceAllCommand(oldSlices, newSlices));
        editor.commands.push(std::move(cmd), editor.project.slices);
        editor.project.markDirty();

        preview.clear();
        cachedRects.clear();
        ImGui::CloseCurrentPopup();
    } else if (cancelClicked) {
        preview.clear();
        cachedRects.clear();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
