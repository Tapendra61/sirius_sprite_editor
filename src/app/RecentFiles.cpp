#include "app/RecentFiles.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include "util/ConfigDir.h"
#include "util/JsonFile.h"

using json = nlohmann::json;

static const char* CONFIG_NAME = "recent_files.json";

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

    return writeFileAtomic(configPath(CONFIG_NAME), j.dump(2)).empty();
}

void RecentFiles::load() {
    items_.clear();
    std::ifstream in(configPath(CONFIG_NAME));
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
