#ifndef SIRIUS_RECENT_FILES_H
#define SIRIUS_RECENT_FILES_H

#include <string>
#include <vector>

class RecentFiles {
public:
    static const int kMaxItems = 8;

    RecentFiles();

    void load();
    bool save() const;
    void add(const std::string& path);
    void remove(const std::string& path);
    void clear();

    const std::vector<std::string>& items() const { return items_; }

private:
    std::vector<std::string> items_;
};

#endif
