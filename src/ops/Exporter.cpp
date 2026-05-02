#include "ops/Exporter.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "app/Project.h"
#include "util/Guid.h"
#include "util/JsonFile.h"
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
    return writeFileAtomic(path, contents).empty();
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

// Unity .meta export.
//
// Coordinate conversion:
//   Sirius rect.y measures from the image TOP-LEFT (image-pixel space).
//   Unity rect.y measures from the image BOTTOM-LEFT.
//   Unity y = imageHeight - (sliceY + sliceHeight)
//
// Pivot conversion:
//   Sirius pivot.y is 0..1 from the slice TOP.
//   Unity pivot.y is 0..1 from the slice BOTTOM.
//   Unity pivotY = 1 - sirius pivotY
//
// Border:
//   Sirius stores L/T/R/B in border.x/y/width/height.
//   Unity packs as {x: L, y: B, z: R, w: T}.
//
// Alignment is always 9 (Custom) so the explicit pivot value is honored
// without Unity snapping to a named alignment.
//
// IDs:
//   - Each sprite gets a fresh 32-hex `spriteID` and an even `internalID`
//     starting at 21300000 (Unity's convention for type 213 = Sprite).
//   - The whole file gets a fresh asset GUID.
bool Exporter::exportUnityMeta(const Project& project, const std::string& path, bool selectedOnly) {
    if (!project.isImageLoaded()) return false;

    const auto& slices = project.slices.slices;
    const int imgH = project.image.height;

    // Filter slices for export.
    std::vector<const Slice*> picks;
    picks.reserve(slices.size());
    for (size_t i = 0; i < slices.size(); ++i) {
        const Slice& s = slices[i];
        if (selectedOnly && !project.slices.isSelected(s.id)) continue;
        picks.push_back(&s);
    }

    std::ostringstream out;
    out << "fileFormatVersion: 2\n";
    out << "guid: " << makeGuid32() << "\n";
    out << "TextureImporter:\n";

    // internalIDToNameTable — Unity uses this to keep stable IDs when sprites
    // are renamed. Each entry maps {213 (= Sprite class id): internalID} -> name.
    out << "  internalIDToNameTable:\n";
    for (size_t i = 0; i < picks.size(); ++i) {
        long long internalId = 21300000LL + 2LL * (long long)i;
        out << "  - first:\n";
        out << "      213: " << internalId << "\n";
        out << "    second: " << picks[i]->name << "\n";
    }

    out << "  externalObjects: {}\n";
    out << "  serializedVersion: 13\n";
    out << "  mipmaps:\n";
    out << "    mipMapMode: 0\n";
    out << "    enableMipMap: 0\n";
    out << "    sRGBTexture: 1\n";
    out << "    linearTexture: 0\n";
    out << "    fadeOut: 0\n";
    out << "    borderMipMap: 0\n";
    out << "    mipMapsPreserveCoverage: 0\n";
    out << "    alphaTestReferenceValue: 0.5\n";
    out << "    mipMapFadeDistanceStart: 1\n";
    out << "    mipMapFadeDistanceEnd: 3\n";
    out << "  bumpmap:\n";
    out << "    convertToNormalMap: 0\n";
    out << "    externalNormalMap: 0\n";
    out << "    heightScale: 0.25\n";
    out << "    normalMapFilter: 0\n";
    out << "  isReadable: 0\n";
    out << "  streamingMipmaps: 0\n";
    out << "  streamingMipmapsPriority: 0\n";
    out << "  vTOnly: 0\n";
    out << "  ignoreMasterTextureLimit: 0\n";
    out << "  grayScaleToAlpha: 0\n";
    out << "  generateCubemap: 6\n";
    out << "  cubemapConvolution: 0\n";
    out << "  seamlessCubemap: 0\n";
    out << "  textureFormat: 1\n";
    out << "  maxTextureSize: 2048\n";
    out << "  textureSettings:\n";
    out << "    serializedVersion: 2\n";
    out << "    filterMode: 0\n";       // Point — sprites are typically pixel art
    out << "    aniso: 1\n";
    out << "    mipBias: 0\n";
    out << "    wrapU: 1\n";             // Clamp
    out << "    wrapV: 1\n";
    out << "    wrapW: 1\n";
    out << "  nPOTScale: 0\n";
    out << "  lightmap: 0\n";
    out << "  compressionQuality: 50\n";
    out << "  spriteMode: 2\n";          // Multiple
    out << "  spriteExtrude: 1\n";
    out << "  spriteMeshType: 1\n";      // Tight
    out << "  alignment: 0\n";
    out << "  spritePivot: {x: 0.5, y: 0.5}\n";
    out << "  spritePixelsToUnits: 100\n";
    out << "  spriteBorder: {x: 0, y: 0, z: 0, w: 0}\n";
    out << "  spriteGenerateFallbackPhysicsShape: 1\n";
    out << "  alphaUsage: 1\n";
    out << "  alphaIsTransparency: 1\n";
    out << "  spriteTessellationDetail: -1\n";
    out << "  textureType: 8\n";         // Sprite (2D and UI)
    out << "  textureShape: 1\n";
    out << "  singleChannelComponent: 0\n";
    out << "  flipbookRows: 1\n";
    out << "  flipbookColumns: 1\n";
    out << "  maxTextureSizeSet: 0\n";
    out << "  compressionQualitySet: 0\n";
    out << "  textureFormatSet: 0\n";
    out << "  ignorePngGamma: 0\n";
    out << "  applyGammaDecoding: 0\n";
    out << "  cookieLightType: 0\n";
    out << "  platformSettings:\n";
    out << "  - serializedVersion: 3\n";
    out << "    buildTarget: DefaultTexturePlatform\n";
    out << "    maxTextureSize: 2048\n";
    out << "    resizeAlgorithm: 0\n";
    out << "    textureFormat: -1\n";
    out << "    textureCompression: 1\n";
    out << "    compressionQuality: 50\n";
    out << "    crunchedCompression: 0\n";
    out << "    allowsAlphaSplitting: 0\n";
    out << "    overridden: 0\n";
    out << "    ignorePlatformSupport: 0\n";
    out << "    androidETC2FallbackOverride: 0\n";
    out << "    forceMaximumCompressionQuality_BC6H_BC7: 0\n";

    out << "  spriteSheet:\n";
    out << "    serializedVersion: 2\n";
    out << "    sprites:\n";
    for (size_t i = 0; i < picks.size(); ++i) {
        const Slice& s = *picks[i];

        // Image-space (top-left) → Unity-space (bottom-left).
        int rx = (int)s.rect.x;
        int ry = imgH - (int)(s.rect.y + s.rect.height);
        int rw = (int)s.rect.width;
        int rh = (int)s.rect.height;

        // Pivot Y flip (slice-local 0..1).
        float px = s.pivot.x;
        float py = 1.0f - s.pivot.y;

        // Border L/T/R/B → Unity {x:L, y:B, z:R, w:T}.
        int bL = (int)s.border.x;
        int bT = (int)s.border.y;
        int bR = (int)s.border.width;
        int bB = (int)s.border.height;

        long long internalId = 21300000LL + 2LL * (long long)i;

        out << "    - serializedVersion: 2\n";
        out << "      name: " << s.name << "\n";
        out << "      rect:\n";
        out << "        serializedVersion: 2\n";
        out << "        x: " << rx << "\n";
        out << "        y: " << ry << "\n";
        out << "        width: "  << rw << "\n";
        out << "        height: " << rh << "\n";
        out << "      alignment: 9\n";  // Custom — honor explicit pivot
        out << "      pivot: {x: " << px << ", y: " << py << "}\n";
        out << "      border: {x: " << bL << ", y: " << bB
            << ", z: "  << bR << ", w: " << bT << "}\n";
        out << "      outline: []\n";
        out << "      physicsShape: []\n";
        out << "      tessellationDetail: 0\n";
        out << "      bones: []\n";
        out << "      spriteID: " << makeGuid32() << "\n";
        out << "      internalID: " << internalId << "\n";
        out << "      vertices: []\n";
        out << "      indices:\n";
        out << "      edges: []\n";
        out << "      weights: []\n";
    }
    out << "    outline: []\n";
    out << "    physicsShape: []\n";
    out << "    bones: []\n";
    out << "    spriteID:\n";
    out << "    internalID: 0\n";
    out << "    vertices: []\n";
    out << "    indices:\n";
    out << "    edges: []\n";
    out << "    weights: []\n";
    out << "    secondaryTextures: []\n";

    out << "    nameFileIdTable:\n";
    for (size_t i = 0; i < picks.size(); ++i) {
        long long internalId = 21300000LL + 2LL * (long long)i;
        out << "      " << picks[i]->name << ": " << internalId << "\n";
    }

    out << "  mipmapLimitGroupName:\n";
    out << "  pSDRemoveMatte: 0\n";
    out << "  userData:\n";
    out << "  assetBundleName:\n";
    out << "  assetBundleVariant:\n";

    return atomicWrite(path, out.str());
}

bool Exporter::exportTexturePackerJson(const Project& project, const std::string& path, bool selectedOnly) {
    (void)project;
    (void)path;
    (void)selectedOnly;
    return false;  // v2
}
