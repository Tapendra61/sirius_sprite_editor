#include "app/CanvasOptions.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include "util/ConfigDir.h"
#include "util/JsonFile.h"

using json = nlohmann::json;

static const char* CONFIG_NAME = "canvas_options.json";

bool CanvasOptions::save() const {
    json j;
    j["checkerVisible"]   = checkerVisible;
    j["checkerSize"]      = checkerSize;
    j["checkerLight"]     = { checkerLight[0], checkerLight[1], checkerLight[2] };
    j["checkerDark"]      = { checkerDark[0],  checkerDark[1],  checkerDark[2] };
    j["gridSnapEnabled"]  = gridSnapEnabled;
    j["gridSnapX"]        = gridSnapX;
    j["gridSnapY"]        = gridSnapY;
    j["pixelGridVisible"] = pixelGridVisible;
    j["pixelGridZoomMin"] = pixelGridZoomMin;

    return writeFileAtomic(configPath(CONFIG_NAME), j.dump(2)).empty();
}

bool CanvasOptions::load() {
    std::ifstream in(configPath(CONFIG_NAME));
    if (!in.is_open()) return false;

    json j = json::parse(in, nullptr, false);
    if (j.is_discarded() || !j.is_object()) return false;

    auto getBool = [&](const char* k, bool& dst) {
        if (j.contains(k) && j[k].is_boolean()) dst = j[k].get<bool>();
    };
    auto getInt = [&](const char* k, int& dst) {
        if (j.contains(k) && j[k].is_number_integer()) dst = j[k].get<int>();
    };
    auto getFloat = [&](const char* k, float& dst) {
        if (j.contains(k) && j[k].is_number()) dst = j[k].get<float>();
    };

    getBool ("checkerVisible",   checkerVisible);
    getInt  ("checkerSize",      checkerSize);
    getBool ("gridSnapEnabled",  gridSnapEnabled);
    getInt  ("gridSnapX",        gridSnapX);
    getInt  ("gridSnapY",        gridSnapY);
    getBool ("pixelGridVisible", pixelGridVisible);
    getFloat("pixelGridZoomMin", pixelGridZoomMin);

    auto getRgb = [&](const char* k, int dst[3]) {
        if (j.contains(k) && j[k].is_array() && j[k].size() == 3) {
            for (int i = 0; i < 3; ++i) {
                if (j[k][i].is_number_integer()) {
                    int v = j[k][i].get<int>();
                    if (v < 0) v = 0;
                    if (v > 255) v = 255;
                    dst[i] = v;
                }
            }
        }
    };
    getRgb("checkerLight", checkerLight);
    getRgb("checkerDark",  checkerDark);
    return true;
}
