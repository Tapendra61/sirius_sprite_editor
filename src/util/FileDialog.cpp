// IMPORTANT: this TU must NOT include raylib (or anything that includes
// raylib) — pfd transitively pulls in <windows.h> on Windows, whose CloseWindow
// / ShowCursor signatures collide with raylib's.
#include "util/FileDialog.h"

#include "portable-file-dialogs.h"

namespace dlg {

std::string openFile(const std::string& title,
                     const std::vector<std::string>& filters) {
    auto picks = pfd::open_file(title, "", filters, pfd::opt::none).result();
    if (picks.empty()) return std::string();
    return picks[0];
}

std::string saveFile(const std::string& title,
                     const std::string& defaultName,
                     const std::vector<std::string>& filters) {
    return pfd::save_file(title, defaultName, filters, pfd::opt::none).result();
}

std::string selectFolder(const std::string& title) {
    return pfd::select_folder(title).result();
}

}  // namespace dlg
