#ifndef SIRIUS_SLICE_STORE_H
#define SIRIUS_SLICE_STORE_H

#include <vector>
#include "model/Slice.h"

class SliceStore {
public:
    SliceStore();
    ~SliceStore();

    void add(const Slice& s);
    void remove(int id);
    void replace(int id, const Slice& s);
    void replaceAll(const std::vector<Slice>& newSlices);
    void clear();

    Slice* find(int id);
    const Slice* find(int id) const;

    void selectOnly(int id);
    void selectAdd(int id);
    void selectRemove(int id);
    void selectClear();
    bool isSelected(int id) const;

    int count() const;
    int selectionCount() const;

    std::vector<Slice> slices;
    std::vector<int> selectedIds;
};

#endif
