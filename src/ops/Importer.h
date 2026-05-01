#ifndef SIRIUS_IMPORTER_H
#define SIRIUS_IMPORTER_H

#include <string>

class Project;
struct CanvasView;

class Importer {
public:
    static bool loadProject(Project& project, CanvasView& view, const std::string& path);
    static bool saveProject(const Project& project, const CanvasView& view, const std::string& path);
};

#endif
