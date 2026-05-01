#include "app/Project.h"

Project::Project()
    : image{},
      texture{},
      idCounter(0),
      dirty(false),
      imageLoaded(false) {
}

Project::~Project() {
    unloadImage();
}

bool Project::loadImage(const std::string& path) {
    unloadImage();

    Image img = LoadImage(path.c_str());
    if (img.data == nullptr) {
        return false;
    }

    Texture2D tex = LoadTextureFromImage(img);
    if (tex.id == 0) {
        UnloadImage(img);
        return false;
    }

    SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    image = img;
    texture = tex;
    imagePath = path;
    imageLoaded = true;
    return true;
}

void Project::unloadImage() {
    if (imageLoaded) {
        UnloadTexture(texture);
        UnloadImage(image);
    }
    image = {};
    texture = {};
    imageLoaded = false;
    imagePath.clear();
}

bool Project::isImageLoaded() const {
    return imageLoaded;
}

int Project::nextId() {
    idCounter = idCounter + 1;
    return idCounter;
}

bool Project::isDirty() const {
    return dirty;
}

void Project::markDirty() {
    dirty = true;
}

void Project::clearDirty() {
    dirty = false;
}
