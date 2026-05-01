#include "ops/Importer.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <nlohmann/json.hpp>
#include "app/Project.h"
#include "model/Slice.h"
#include "raylib.h"
#include "util/Coords.h"

using json = nlohmann::json;

bool Importer::loadProject(Project& project, CanvasView& view, const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    json j = json::parse(in, nullptr, false);
    if (j.is_discarded() || !j.is_object()) return false;

    // Reset existing state
    project.unloadImage();
    project.slices.clear();

    // Image — stored relative to project file
    std::string imageRel = j.value("image", std::string(""));
    std::filesystem::path projAbs = std::filesystem::absolute(path);
    std::filesystem::path projDir = projAbs.parent_path();

    if (!imageRel.empty()) {
        std::filesystem::path imgAbs = (projDir / imageRel).lexically_normal();
        project.loadImage(imgAbs.string());  // OK if it fails — just no image
    }

    // Slices
    if (j.contains("slices") && j["slices"].is_array()) {
        for (size_t i = 0; i < j["slices"].size(); ++i) {
            const json& sj = j["slices"][i];
            Slice s;
            s.id = sj.value("id", 0);
            s.name = sj.value("name", std::string(""));

            if (sj.contains("rect") && sj["rect"].is_object()) {
                s.rect.x      = sj["rect"].value("x", 0.0f);
                s.rect.y      = sj["rect"].value("y", 0.0f);
                s.rect.width  = sj["rect"].value("w", 0.0f);
                s.rect.height = sj["rect"].value("h", 0.0f);
            }
            if (sj.contains("pivot") && sj["pivot"].is_object()) {
                s.pivot.x = sj["pivot"].value("x", 0.5f);
                s.pivot.y = sj["pivot"].value("y", 0.5f);
            } else {
                s.pivot = { 0.5f, 0.5f };
            }
            if (sj.contains("border") && sj["border"].is_object()) {
                s.border.x      = sj["border"].value("l", 0.0f);
                s.border.y      = sj["border"].value("t", 0.0f);
                s.border.width  = sj["border"].value("r", 0.0f);
                s.border.height = sj["border"].value("b", 0.0f);
            } else {
                s.border = { 0.0f, 0.0f, 0.0f, 0.0f };
            }

            project.slices.slices.push_back(s);
        }
    }

    // ID counter — never reuse old IDs even after load
    int nextId = j.value("nextSliceId", 0);
    project.setIdCounter(nextId);

    // View state
    if (j.contains("view") && j["view"].is_object()) {
        view.camera.target.x = j["view"].value("panX", 0.0f);
        view.camera.target.y = j["view"].value("panY", 0.0f);
        view.camera.zoom     = j["view"].value("zoom", 1.0f);
    } else {
        view.camera.target = { 0.0f, 0.0f };
        view.camera.zoom   = 1.0f;
    }

    project.projectPath = path;
    project.clearDirty();
    return true;
}

static std::string atomicWriteJson(const std::string& path, const std::string& contents) {
    std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp);
        if (!out.is_open()) return "could not open temp file";
        out << contents;
        if (out.fail()) return "write failed";
    }
    std::error_code ec;
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(tmp, ec);
        return "rename failed";
    }
    return "";  // success
}

bool Importer::saveProject(const Project& project, const CanvasView& view, const std::string& path) {
    // Image path relative to the project file (forward slashes for portability)
    std::string imageRel;
    if (!project.imagePath.empty()) {
        std::filesystem::path projAbs = std::filesystem::absolute(path);
        std::filesystem::path projDir = projAbs.parent_path();
        std::filesystem::path imgAbs(project.imagePath);
        std::error_code ec;
        std::filesystem::path rel = std::filesystem::relative(imgAbs, projDir, ec);
        if (!ec) {
            imageRel = rel.generic_string();
        } else {
            imageRel = project.imagePath;
        }
    }

    json j;
    j["version"]      = 1;
    j["image"]        = imageRel;
    j["nextSliceId"]  = project.getIdCounter();

    j["view"]         = json::object();
    j["view"]["zoom"] = view.camera.zoom;
    j["view"]["panX"] = view.camera.target.x;
    j["view"]["panY"] = view.camera.target.y;

    j["slices"] = json::array();
    for (size_t i = 0; i < project.slices.slices.size(); ++i) {
        const Slice& s = project.slices.slices[i];
        json sj;
        sj["id"]   = s.id;
        sj["name"] = s.name;

        sj["rect"] = json::object();
        sj["rect"]["x"] = (int)s.rect.x;
        sj["rect"]["y"] = (int)s.rect.y;
        sj["rect"]["w"] = (int)s.rect.width;
        sj["rect"]["h"] = (int)s.rect.height;

        sj["pivot"] = json::object();
        sj["pivot"]["x"] = s.pivot.x;
        sj["pivot"]["y"] = s.pivot.y;

        sj["border"] = json::object();
        sj["border"]["l"] = (int)s.border.x;
        sj["border"]["t"] = (int)s.border.y;
        sj["border"]["r"] = (int)s.border.width;
        sj["border"]["b"] = (int)s.border.height;

        j["slices"].push_back(sj);
    }

    std::string err = atomicWriteJson(path, j.dump(2));
    return err.empty();
}
