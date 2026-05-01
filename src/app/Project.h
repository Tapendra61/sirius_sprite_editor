#ifndef SIRIUS_PROJECT_H
#define SIRIUS_PROJECT_H

#include <string>
#include "raylib.h"
#include "model/SliceStore.h"

class Project {
public:
    Project();
    ~Project();

    bool loadImage(const std::string& path);
    void unloadImage();
    bool isImageLoaded() const;

    int nextId();
    int getIdCounter() const { return idCounter; }
    void setIdCounter(int v) { idCounter = v; }

    bool isDirty() const;
    void markDirty();
    void clearDirty();

    std::string imagePath;
    std::string projectPath;

    Image image;
    Texture2D texture;

    SliceStore slices;

private:
    int idCounter;
    bool dirty;
    bool imageLoaded;
};

#endif
