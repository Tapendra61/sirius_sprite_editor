#include "ui/ExportModal.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "app/Editor.h"
#include "imgui.h"
#include "ops/Exporter.h"
#include "ui/Palette.h"
#include "ui/Theme.h"
#include "util/FileDialog.h"

ExportModal::ExportModal()
    : showing(false),
      openRequested(false),
      formatIdx((int)ExportFormat::AtlasJson),
      selectedOnly(false),
      lastResultShown(false),
      lastResultOk(false) {
    pathBuf[0] = '\0';
}

ExportModal::~ExportModal() {
}

void ExportModal::open() {
    openRequested = true;
    lastResultShown = false;
}

static std::string defaultMetaName(const std::string& imagePath) {
    if (imagePath.empty()) return "sprites.png.meta";
    // Strip the directory; keep the filename + ".meta".
    size_t slash = imagePath.find_last_of("/\\");
    std::string base = (slash == std::string::npos)
        ? imagePath : imagePath.substr(slash + 1);
    return base + ".meta";
}

static void runBrowse(int formatIdx, const std::string& imagePath,
                      char* pathBuf, size_t cap) {
    std::string sel;
    if (formatIdx == (int)ExportFormat::AtlasJson) {
        std::vector<std::string> filters = {
            "JSON", "*.json",
            "All Files", "*"
        };
        sel = dlg::saveFile("Save Atlas JSON", "atlas.json", filters);
    } else if (formatIdx == (int)ExportFormat::Pngs) {
        sel = dlg::selectFolder("Choose output folder");
    } else if (formatIdx == (int)ExportFormat::Csv) {
        std::vector<std::string> filters = {
            "CSV", "*.csv",
            "All Files", "*"
        };
        sel = dlg::saveFile("Save CSV", "atlas.csv", filters);
    } else if (formatIdx == (int)ExportFormat::UnityMeta) {
        std::vector<std::string> filters = {
            "Unity Meta", "*.meta",
            "All Files", "*"
        };
        sel = dlg::saveFile("Save Unity Sprite Meta",
                            defaultMetaName(imagePath), filters);
    }

    if (!sel.empty()) {
        std::strncpy(pathBuf, sel.c_str(), cap - 1);
        pathBuf[cap - 1] = '\0';
    }
}

using pal::INK_3;

void ExportModal::draw(Editor& editor) {
    if (openRequested) {
        ImGui::OpenPopup("Export");
        openRequested = false;
    }

    showing = ImGui::IsPopupOpen("Export");

    if (!ImGui::BeginPopupModal("Export", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    // Format
    bool hasImage = editor.project.isImageLoaded();

    ImGui::TextColored(INK_3, "FORMAT");
    ImGui::RadioButton("Atlas JSON (custom schema, default)", &formatIdx, (int)ExportFormat::AtlasJson);
    ImGui::RadioButton("Individual PNGs (one file per slice)", &formatIdx, (int)ExportFormat::Pngs);
    ImGui::RadioButton("CSV", &formatIdx, (int)ExportFormat::Csv);

    if (!hasImage) ImGui::BeginDisabled();
    ImGui::RadioButton("Unity Sprite Meta (.png.meta)", &formatIdx, (int)ExportFormat::UnityMeta);
    if (!hasImage) {
        ImGui::EndDisabled();
        if (formatIdx == (int)ExportFormat::UnityMeta) {
            formatIdx = (int)ExportFormat::AtlasJson;
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Output path
    ImGui::TextColored(INK_3, "OUTPUT PATH");
    ImGui::PushItemWidth(280.0f);
    ImGui::InputText("##path", pathBuf, sizeof(pathBuf));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        runBrowse(formatIdx, editor.project.imagePath, pathBuf, sizeof(pathBuf));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Options
    int selCount = editor.project.slices.selectionCount();
    char selLabel[64];
    std::snprintf(selLabel, sizeof(selLabel),
                  "Include only selected slices (%d)", selCount);
    if (selCount == 0) ImGui::BeginDisabled();
    ImGui::Checkbox(selLabel, &selectedOnly);
    if (selCount == 0) {
        ImGui::EndDisabled();
        if (selectedOnly) selectedOnly = false;
    }

    // Last result feedback
    if (lastResultShown) {
        ImGui::Spacing();
        if (lastResultOk) {
            ImGui::TextColored(ImVec4(0.42f, 0.80f, 0.47f, 1.0f),
                               "%s", lastResultMsg.c_str());  // success green
        } else {
            ImGui::TextColored(ImVec4(0.94f, 0.37f, 0.37f, 1.0f),
                               "%s", lastResultMsg.c_str());  // error red
        }
    }

    ImGui::Separator();

    // Footer buttons
    bool cancelClicked = ImGui::Button("Cancel");
    ImGui::SameLine();

    bool pathOk = pathBuf[0] != '\0';
    bool hasSlices = editor.project.slices.count() > 0;
    bool canExport = pathOk && hasSlices;

    if (!canExport) ImGui::BeginDisabled();
    bool exportClicked = PrimaryButton("Export");
    if (!canExport) ImGui::EndDisabled();

    if (cancelClicked) {
        ImGui::CloseCurrentPopup();
        lastResultShown = false;
    } else if (exportClicked) {
        bool ok = false;
        std::string path(pathBuf);
        switch ((ExportFormat)formatIdx) {
            case ExportFormat::AtlasJson:
                ok = Exporter::exportAtlasJson(editor.project, path, selectedOnly);
                break;
            case ExportFormat::Pngs:
                ok = Exporter::exportPngs(editor.project, path, selectedOnly);
                break;
            case ExportFormat::Csv:
                ok = Exporter::exportCsv(editor.project, path, selectedOnly);
                break;
            case ExportFormat::UnityMeta:
                ok = Exporter::exportUnityMeta(editor.project, path, selectedOnly);
                break;
        }
        lastResultShown = true;
        lastResultOk = ok;
        if (ok) {
            char msg[256];
            std::snprintf(msg, sizeof(msg), "Exported to %s", path.c_str());
            lastResultMsg = msg;
        } else {
            lastResultMsg = "Export failed.";
        }
    }

    ImGui::EndPopup();
}
