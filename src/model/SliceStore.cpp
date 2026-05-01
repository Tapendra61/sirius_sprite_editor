#include "model/SliceStore.h"

SliceStore::SliceStore() {
}

SliceStore::~SliceStore() {
}

void SliceStore::add(const Slice& s) {
    slices.push_back(s);
}

void SliceStore::remove(int id) {
    selectRemove(id);
    for (size_t i = 0; i < slices.size(); ++i) {
        if (slices[i].id == id) {
            slices.erase(slices.begin() + i);
            return;
        }
    }
}

void SliceStore::replace(int id, const Slice& s) {
    for (size_t i = 0; i < slices.size(); ++i) {
        if (slices[i].id == id) {
            slices[i] = s;
            slices[i].id = id;
            return;
        }
    }
}

void SliceStore::replaceAll(const std::vector<Slice>& newSlices) {
    selectClear();
    slices = newSlices;
}

void SliceStore::clear() {
    slices.clear();
    selectedIds.clear();
}

Slice* SliceStore::find(int id) {
    for (size_t i = 0; i < slices.size(); ++i) {
        if (slices[i].id == id) {
            return &slices[i];
        }
    }
    return nullptr;
}

const Slice* SliceStore::find(int id) const {
    for (size_t i = 0; i < slices.size(); ++i) {
        if (slices[i].id == id) {
            return &slices[i];
        }
    }
    return nullptr;
}

void SliceStore::selectOnly(int id) {
    selectedIds.clear();
    if (find(id) != nullptr) {
        selectedIds.push_back(id);
    }
}

void SliceStore::selectAdd(int id) {
    if (isSelected(id)) {
        return;
    }
    if (find(id) != nullptr) {
        selectedIds.push_back(id);
    }
}

void SliceStore::selectRemove(int id) {
    for (size_t i = 0; i < selectedIds.size(); ++i) {
        if (selectedIds[i] == id) {
            selectedIds.erase(selectedIds.begin() + i);
            return;
        }
    }
}

void SliceStore::selectClear() {
    selectedIds.clear();
}

bool SliceStore::isSelected(int id) const {
    for (size_t i = 0; i < selectedIds.size(); ++i) {
        if (selectedIds[i] == id) {
            return true;
        }
    }
    return false;
}

int SliceStore::count() const {
    return (int)slices.size();
}

int SliceStore::selectionCount() const {
    return (int)selectedIds.size();
}
