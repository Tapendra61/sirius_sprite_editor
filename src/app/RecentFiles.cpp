#include "app/RecentFiles.h"

#include <filesystem>
#include <fstream>
#include <system_error>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static const char* CONFIG_PATH = "recent_files.json";

RecentFiles::RecentFiles() {
    load();
}

void RecentFiles::add(const std::string& path) {
    if (path.empty()) return;
    remove(path);
    items_.insert(items_.begin(), path);
    if ((int)items_.size() > kMaxItems) {
        items_.resize(kMaxItems);
    }
    save();
}

void RecentFiles::remove(const std::string& path) {
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i] == path) {
            items_.erase(items_.begin() + i);
            return;
        }
    }
}

void RecentFiles::clear() {
    items_.clear();
    save();
}

bool RecentFiles::save() const {
    json j;
    j["items"] = items_;

    std::string tmp = std::string(CONFIG_PATH) + ".tmp";
    {
        std::ofstream out(tmp);
        if (!out.is_open()) return false;
        out << j.dump(2);
        if (out.fail()) return false;
    }
    std::error_code ec;
    std::filesystem::rename(tmp, CONFIG_PATH, ec);
    if (ec) {
        std::filesystem::remove(tmp, ec);
        return false;
    }
    return true;
}

void RecentFiles::load() {
    items_.clear();
    std::ifstream in(CONFIG_PATH);
    if (!in.is_open()) return;

    json j = json::parse(in, nullptr, false);
    if (j.is_discarded() || !j.is_object()) return;
    if (!j.contains("items") || !j["items"].is_array()) return;

    for (const auto& v : j["items"]) {
        if (v.is_string()) {
            items_.push_back(v.get<std::string>());
            if ((int)items_.size() >= kMaxItems) break;
        }
    }
}
