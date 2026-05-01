#ifndef SIRIUS_SLICE_COMMANDS_H
#define SIRIUS_SLICE_COMMANDS_H

#include <vector>
#include "commands/Command.h"
#include "model/Slice.h"

class CreateSliceCommand : public ICommand {
public:
    CreateSliceCommand(const Slice& slice);

    void redo(SliceStore& store) override;
    void undo(SliceStore& store) override;
    const char* name() const override;

private:
    Slice slice;
};

class DeleteSlicesCommand : public ICommand {
public:
    DeleteSlicesCommand(const std::vector<Slice>& slices);

    void redo(SliceStore& store) override;
    void undo(SliceStore& store) override;
    const char* name() const override;

private:
    std::vector<Slice> deletedSlices;
};

class EditSlicesCommand : public ICommand {
public:
    EditSlicesCommand(const std::vector<Slice>& before, const std::vector<Slice>& after);

    void redo(SliceStore& store) override;
    void undo(SliceStore& store) override;
    const char* name() const override;

private:
    std::vector<Slice> before;
    std::vector<Slice> after;
};

class ReplaceAllCommand : public ICommand {
public:
    ReplaceAllCommand(const std::vector<Slice>& oldSlices, const std::vector<Slice>& newSlices);

    void redo(SliceStore& store) override;
    void undo(SliceStore& store) override;
    const char* name() const override;

private:
    std::vector<Slice> oldSlices;
    std::vector<Slice> newSlices;
};

#endif
