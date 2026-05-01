#include "commands/CommandStack.h"

CommandStack::CommandStack() {
}

CommandStack::~CommandStack() {
}

void CommandStack::push(std::unique_ptr<ICommand> cmd, SliceStore& store) {
    cmd->redo(store);
    done.push_back(std::move(cmd));
    undone.clear();
}

void CommandStack::pushAlreadyApplied(std::unique_ptr<ICommand> cmd) {
    done.push_back(std::move(cmd));
    undone.clear();
}

void CommandStack::undo(SliceStore& store) {
    if (done.empty()) return;
    std::unique_ptr<ICommand> cmd = std::move(done.back());
    done.pop_back();
    cmd->undo(store);
    undone.push_back(std::move(cmd));
}

void CommandStack::redo(SliceStore& store) {
    if (undone.empty()) return;
    std::unique_ptr<ICommand> cmd = std::move(undone.back());
    undone.pop_back();
    cmd->redo(store);
    done.push_back(std::move(cmd));
}

bool CommandStack::canUndo() const {
    return !done.empty();
}

bool CommandStack::canRedo() const {
    return !undone.empty();
}

const char* CommandStack::topUndoName() const {
    if (done.empty()) return nullptr;
    return done.back()->name();
}

const char* CommandStack::topRedoName() const {
    if (undone.empty()) return nullptr;
    return undone.back()->name();
}

void CommandStack::clear() {
    done.clear();
    undone.clear();
}
