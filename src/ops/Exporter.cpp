#include "ops/Exporter.h"

#include "app/Project.h"

bool Exporter::exportAtlasJson(const Project& project, const std::string& path) {
    (void)project;
    (void)path;
    return false;
}

bool Exporter::exportPngs(const Project& project, const std::string& outDir) {
    (void)project;
    (void)outDir;
    return false;
}

bool Exporter::exportCsv(const Project& project, const std::string& path) {
    (void)project;
    (void)path;
    return false;
}

bool Exporter::exportUnityMeta(const Project& project, const std::string& path) {
    (void)project;
    (void)path;
    return false;
}

bool Exporter::exportTexturePackerJson(const Project& project, const std::string& path) {
    (void)project;
    (void)path;
    return false;
}
