#ifndef SIRIUS_COMMAND_STACK_H
#define SIRIUS_COMMAND_STACK_H

#include <vector>
#include <memory>
#include "commands/Command.h"

class SliceStore;

class CommandStack {
public:
    CommandStack();
    ~CommandStack();

    void push(std::unique_ptr<ICommand> cmd, SliceStore& store);
    void pushAlreadyApplied(std::unique_ptr<ICommand> cmd);

    void undo(SliceStore& store);
    void redo(SliceStore& store);

    bool canUndo() const;
    bool canRedo() const;

    const char* topUndoName() const;
    const char* topRedoName() const;

    void clear();

private:
    std::vector< std::unique_ptr<ICommand> > done;
    std::vector< std::unique_ptr<ICommand> > undone;
};

#endif
