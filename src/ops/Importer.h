#ifndef SIRIUS_IMPORTER_H
#define SIRIUS_IMPORTER_H

#include <string>

class Project;

class Importer {
public:
    static bool loadProject(Project& project, const std::string& path);
    static bool loadImage(Project& project, const std::string& path);
};

#endif
