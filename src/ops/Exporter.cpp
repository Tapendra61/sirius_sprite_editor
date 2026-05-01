#include "ops/Exporter.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>
#include "app/Project.h"
#include "model/Slice.h"
#include "raylib.h"

static std::string escapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;
            case '\r': out += "\\r";  break;
            default:
                if ((unsigned char)c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

static std::string sanitizeFilename(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (size_t i = 0; i < name.size(); ++i) {
        char c = name[i];
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
            c == '"' || c == '<' || c == '>' || c == '|' ||
            (unsigned char)c < 0x20) {
            out += '_';
        } else {
            out += c;
        }
    }
    if (out.empty()) out = "slice";
    return out;
}

static bool atomicWrite(const std::string& path, const std::string& contents) {
    std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary);
        if (!out.is_open()) return false;
        out.write(contents.data(), (std::streamsize)contents.size());
        if (out.fail()) return false;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(tmp, ec);
        return false;
    }
    return true;
}

static std::vector<Slice> selectSlicesForExport(const Project& project, bool selectedOnly) {
    if (!selectedOnly) return project.slices.slices;

    std::vector<Slice> out;
    const std::vector<int>& selIds = project.slices.selectedIds;
    out.reserve(selIds.size());
    for (size_t i = 0; i < selIds.size(); ++i) {
        const Slice* s = project.slices.find(selIds[i]);
        if (s != nullptr) out.push_back(*s);
    }
    return out;
}

bool Exporter::exportAtlasJson(const Project& project, const std::string& path, bool selectedOnly) {
    std::vector<Slice> slices = selectSlicesForExport(project, selectedOnly);

    std::ostringstream out;
    int imgW = project.isImageLoaded() ? project.image.width  : 0;
    int imgH = project.isImageLoaded() ? project.image.height : 0;

    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"image\": \"" << escapeJson(project.imagePath) << "\",\n";
    out << "  \"size\": [" << imgW << ", " << imgH << "],\n";
    out << "  \"slices\": [\n";

    for (size_t i = 0; i < slices.size(); ++i) {
        const Slice& s = slices[i];
        out << "    {\n";
        out << "      \"name\":   \"" << escapeJson(s.name) << "\",\n";
        out << "      \"rect\":   ["
            << (int)std::round(s.rect.x) << ", "
            << (int)std::round(s.rect.y) << ", "
            << (int)std::round(s.rect.width) << ", "
            << (int)std::round(s.rect.height) << "],\n";
        out << "      \"pivot\":  [" << s.pivot.x << ", " << s.pivot.y << "],\n";
        out << "      \"border\": ["
            << (int)std::round(s.border.x) << ", "
            << (int)std::round(s.border.y) << ", "
            << (int)std::round(s.border.width) << ", "
            << (int)std::round(s.border.height) << "]\n";
        out << "    }";
        if (i + 1 < slices.size()) out << ",";
        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";

    return atomicWrite(path, out.str());
}

bool Exporter::exportPngs(const Project& project, const std::string& outDir, bool selectedOnly) {
    if (!project.isImageLoaded()) return false;

    std::error_code ec;
    std::filesystem::create_directories(outDir, ec);
    if (ec && ec != std::errc::file_exists) return false;

    std::vector<Slice> slices = selectSlicesForExport(project, selectedOnly);
    int success = 0;

    for (size_t i = 0; i < slices.size(); ++i) {
        const Slice& s = slices[i];

        Rectangle r;
        r.x      = std::round(s.rect.x);
        r.y      = std::round(s.rect.y);
        r.width  = std::round(s.rect.width);
        r.height = std::round(s.rect.height);

        if (r.x < 0.0f) { r.width  += r.x; r.x = 0.0f; }
        if (r.y < 0.0f) { r.height += r.y; r.y = 0.0f; }
        if (r.x + r.width  > (float)project.image.width)  r.width  = (float)project.image.width  - r.x;
        if (r.y + r.height > (float)project.image.height) r.height = (float)project.image.height - r.y;
        if (r.width <= 0.0f || r.height <= 0.0f) continue;

        Image sub = ImageFromImage(project.image, r);
        std::string filename = outDir + "/" + sanitizeFilename(s.name) + ".png";
        if (ExportImage(sub, filename.c_str())) {
            success++;
        }
        UnloadImage(sub);
    }

    return slices.empty() || success > 0;
}

bool Exporter::exportCsv(const Project& project, const std::string& path, bool selectedOnly) {
    std::vector<Slice> slices = selectSlicesForExport(project, selectedOnly);

    std::ostringstream out;
    out << "name,x,y,w,h,pivotX,pivotY,borderL,borderT,borderR,borderB\n";

    for (size_t i = 0; i < slices.size(); ++i) {
        const Slice& s = slices[i];
        out << s.name << ","
            << (int)std::round(s.rect.x) << ","
            << (int)std::round(s.rect.y) << ","
            << (int)std::round(s.rect.width) << ","
            << (int)std::round(s.rect.height) << ","
            << s.pivot.x << "," << s.pivot.y << ","
            << (int)std::round(s.border.x) << ","
            << (int)std::round(s.border.y) << ","
            << (int)std::round(s.border.width) << ","
            << (int)std::round(s.border.height) << "\n";
    }

    return atomicWrite(path, out.str());
}

bool Exporter::exportUnityMeta(const Project& project, const std::string& path, bool selectedOnly) {
    (void)project;
    (void)path;
    (void)selectedOnly;
    return false;  // v2
}

bool Exporter::exportTexturePackerJson(const Project& project, const std::string& path, bool selectedOnly) {
    (void)project;
    (void)path;
    (void)selectedOnly;
    return false;  // v2
}
