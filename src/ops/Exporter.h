#ifndef SIRIUS_EXPORTER_H
#define SIRIUS_EXPORTER_H

#include <string>

class Project;

class Exporter {
public:
    static bool exportAtlasJson(const Project& project, const std::string& path, bool selectedOnly);
    static bool exportPngs(const Project& project, const std::string& outDir, bool selectedOnly);
    static bool exportCsv(const Project& project, const std::string& path, bool selectedOnly);
    static bool exportUnityMeta(const Project& project, const std::string& path, bool selectedOnly);
    static bool exportTexturePackerJson(const Project& project, const std::string& path, bool selectedOnly);
};

#endif
