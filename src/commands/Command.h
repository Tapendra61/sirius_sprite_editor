#ifndef SIRIUS_COMMAND_H
#define SIRIUS_COMMAND_H

class SliceStore;

class ICommand {
public:
    virtual ~ICommand() {}

    virtual void redo(SliceStore& store) = 0;
    virtual void undo(SliceStore& store) = 0;
    virtual const char* name() const = 0;
};

#endif
