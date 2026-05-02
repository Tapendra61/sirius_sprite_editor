#ifndef SIRIUS_FILE_DIALOG_H
#define SIRIUS_FILE_DIALOG_H

#include <string>
#include <vector>

// Thin wrapper around portable-file-dialogs so the rest of the codebase never
// has to include pfd's header. pfd pulls in <windows.h> on Windows, which
// declares functions like CloseWindow/ShowCursor with signatures that conflict
// with raylib's. Keeping pfd in its own translation unit avoids the clash.
//
// `filters` is a flat list of pairs:
//   { "Image Files (*.png *.jpg)", "*.png *.jpg",
//     "All Files",                  "*"          }
namespace dlg {

// Returns the chosen path (or empty string on cancel).
std::string openFile(const std::string& title,
                     const std::vector<std::string>& filters);

// Returns the chosen path (or empty string on cancel).
std::string saveFile(const std::string& title,
                     const std::string& defaultName,
                     const std::vector<std::string>& filters);

// Returns the chosen folder (or empty string on cancel).
std::string selectFolder(const std::string& title);

}  // namespace dlg

#endif
