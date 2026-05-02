#include "ui/SliceModalUtil.h"

#include <cstdio>

std::string expandNamePattern(const std::string& pat, int idx, int row, int col, int total) {
    int width = 1;
    int t = total;
    while (t >= 10) { t /= 10; ++width; }

    std::string out;
    out.reserve(pat.size() + 8);
    char buf[32];

    for (size_t k = 0; k < pat.size(); ++k) {
        if (pat[k] == '{' && k + 2 < pat.size() && pat[k + 2] == '}') {
            char tok = pat[k + 1];
            if (tok == 'i') {
                std::snprintf(buf, sizeof(buf), "%0*d", width, idx);
                out += buf;
                k += 2;
                continue;
            }
            if (tok == 'r') {
                std::snprintf(buf, sizeof(buf), "%d", row);
                out += buf;
                k += 2;
                continue;
            }
            if (tok == 'c') {
                std::snprintf(buf, sizeof(buf), "%d", col);
                out += buf;
                k += 2;
                continue;
            }
        }
        out += pat[k];
    }
    return out;
}

void buildPreviewSlices(std::vector<Slice>& out, const std::vector<Rectangle>& rects) {
    out.clear();
    out.reserve(rects.size());
    for (size_t i = 0; i < rects.size(); ++i) {
        Slice s;
        s.id = -1;
        s.rect = rects[i];
        s.pivot = { 0.5f, 0.5f };
        s.border = { 0.0f, 0.0f, 0.0f, 0.0f };
        out.push_back(s);
    }
}
