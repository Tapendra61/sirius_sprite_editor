#include "commands/SliceCommands.h"

#include "model/SliceStore.h"

CreateSliceCommand::CreateSliceCommand(const Slice& s) : slice(s) {
}

void CreateSliceCommand::redo(SliceStore& store) {
    store.add(slice);
}

void CreateSliceCommand::undo(SliceStore& store) {
    store.remove(slice.id);
}

const char* CreateSliceCommand::name() const {
    return "Create Slice";
}

DeleteSlicesCommand::DeleteSlicesCommand(const std::vector<Slice>& slices)
    : deletedSlices(slices) {
}

void DeleteSlicesCommand::redo(SliceStore& store) {
    for (size_t i = 0; i < deletedSlices.size(); ++i) {
        store.remove(deletedSlices[i].id);
    }
}

void DeleteSlicesCommand::undo(SliceStore& store) {
    for (size_t i = 0; i < deletedSlices.size(); ++i) {
        store.add(deletedSlices[i]);
    }
}

const char* DeleteSlicesCommand::name() const {
    return "Delete Slices";
}

EditSlicesCommand::EditSlicesCommand(const std::vector<Slice>& b, const std::vector<Slice>& a)
    : before(b), after(a) {
}

void EditSlicesCommand::redo(SliceStore& store) {
    for (size_t i = 0; i < after.size(); ++i) {
        store.replace(after[i].id, after[i]);
    }
}

void EditSlicesCommand::undo(SliceStore& store) {
    for (size_t i = 0; i < before.size(); ++i) {
        store.replace(before[i].id, before[i]);
    }
}

const char* EditSlicesCommand::name() const {
    return "Edit Slices";
}

ReplaceAllCommand::ReplaceAllCommand(const std::vector<Slice>& oldS, const std::vector<Slice>& newS)
    : oldSlices(oldS), newSlices(newS) {
}

void ReplaceAllCommand::redo(SliceStore& store) {
    store.replaceAll(newSlices);
}

void ReplaceAllCommand::undo(SliceStore& store) {
    store.replaceAll(oldSlices);
}

const char* ReplaceAllCommand::name() const {
    return "Replace All Slices";
}
