#ifndef SIRIUS_KEYBINDINGS_MODAL_H
#define SIRIUS_KEYBINDINGS_MODAL_H

class Editor;

class KeybindingsModal {
public:
    KeybindingsModal();
    ~KeybindingsModal();

    void open();
    void draw(Editor& editor);

    bool showing;
    bool isRecording() const { return recordingActionIdx >= 0; }

private:
    bool openRequested;
    int  recordingActionIdx;  // -1 if not recording
};

#endif
