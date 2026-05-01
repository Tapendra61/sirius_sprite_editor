#include "ui/AutoSliceModal.h"

#include <memory>
#include <string>
#include "app/Editor.h"
#include "commands/SliceCommands.h"
#include "imgui.h"
#include "ops/AutoSlicer.h"

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
}

AutoSliceModal::~AutoSliceModal() {
}

void AutoSliceModal::open() {
    openRequested = true;
    needsRecompute = true;
}

static void rebuildPreviewSlices(std::vector<Slice>& preview, const std::vector<Rectangle>& rects) {
    preview.clear();
    preview.reserve(rects.size());
    for (size_t i = 0; i < rects.size(); ++i) {
        Slice s;
        s.id = -1;
        s.rect = rects[i];
        s.pivot = { 0.5f, 0.5f };
        s.border = { 0.0f, 0.0f, 0.0f, 0.0f };
        preview.push_back(s);
    }
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
        rebuildPreviewSlices(preview, cachedRects);
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

    bool canApply = !cachedRects.empty();
    if (!canApply) ImGui::BeginDisabled();
    bool applyClicked = ImGui::Button("Apply");
    if (!canApply) ImGui::EndDisabled();

    ImGui::SameLine();
    bool cancelClicked = ImGui::Button("Cancel");

    if (applyClicked) {
        std::vector<Slice> oldSlices = editor.project.slices.slices;
        std::vector<Slice> newSlices;
        if (!replaceAll) {
            newSlices = oldSlices;
        }
        for (size_t i = 0; i < cachedRects.size(); ++i) {
            Slice s;
            s.id = editor.project.nextId();
            s.name = std::string("auto_") + std::to_string(s.id);
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
