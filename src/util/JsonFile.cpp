#include "util/JsonFile.h"

#include <filesystem>
#include <fstream>
#include <system_error>

std::string writeFileAtomic(const std::string& path, const std::string& contents) {
    std::string tmp = path + ".tmp";
    {
        // Binary mode so we don't get CRLF translation on Windows for JSON/CSV.
        std::ofstream out(tmp, std::ios::binary);
        if (!out.is_open()) return "open tmp failed";
        out.write(contents.data(), (std::streamsize)contents.size());
        if (out.fail()) return "write tmp failed";
    }
    std::error_code ec;
    std::filesystem::rename(tmp, path, ec);
    if (ec) {
        std::filesystem::remove(tmp, ec);
        return "rename failed";
    }
    return "";
}
