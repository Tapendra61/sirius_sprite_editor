#include "ops/AutoSlicer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "util/Geometry.h"

class UnionFind {
public:
    std::vector<int> parent;
    std::vector<int> rank_;

    UnionFind(int n) {
        parent.resize(n);
        rank_.resize(n, 0);
        for (int i = 0; i < n; ++i) parent[i] = i;
    }

    int find(int x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (rank_[a] < rank_[b]) std::swap(a, b);
        parent[b] = a;
        if (rank_[a] == rank_[b]) rank_[a]++;
    }
};

struct Box {
    int xMin;
    int yMin;
    int xMax;
    int yMax;
};

static bool rectCompare(const Rectangle& a, const Rectangle& b) {
    if (std::abs(a.y - b.y) > 4.0f) return a.y < b.y;
    return a.x < b.x;
}

static std::vector<Rectangle> mergeNearbyRects(std::vector<Rectangle> rects, int distance) {
    if (distance <= 0) return rects;

    bool merged = true;
    while (merged) {
        merged = false;
        for (size_t i = 0; i < rects.size(); ++i) {
            for (size_t j = i + 1; j < rects.size(); ++j) {
                Rectangle inflated = rects[i];
                inflated.x -= (float)distance;
                inflated.y -= (float)distance;
                inflated.width  += 2.0f * (float)distance;
                inflated.height += 2.0f * (float)distance;

                if (RectsOverlap(inflated, rects[j])) {
                    float xMin = std::min(rects[i].x, rects[j].x);
                    float yMin = std::min(rects[i].y, rects[j].y);
                    float xMax = std::max(rects[i].x + rects[i].width,
                                          rects[j].x + rects[j].width);
                    float yMax = std::max(rects[i].y + rects[i].height,
                                          rects[j].y + rects[j].height);
                    rects[i].x = xMin;
                    rects[i].y = yMin;
                    rects[i].width  = xMax - xMin;
                    rects[i].height = yMax - yMin;
                    rects.erase(rects.begin() + j);
                    merged = true;
                    break;
                }
            }
            if (merged) break;
        }
    }
    return rects;
}

std::vector<Rectangle> autoSlice(const Image& image, const AutoSliceParams& params) {
    std::vector<Rectangle> result;
    if (image.data == nullptr) return result;

    int W = image.width;
    int H = image.height;
    int N = W * H;
    if (N <= 0) return result;

    Color* pixels = LoadImageColors(image);
    if (pixels == nullptr) return result;

    // Step 1: build opacity mask, then free the colors immediately.
    std::vector<uint8_t> mask(N, 0);
    for (int i = 0; i < N; ++i) {
        if (pixels[i].a > params.alphaThreshold) mask[i] = 1;
    }
    UnloadImageColors(pixels);

    // Step 2: union-find connected components.
    UnionFind uf(N);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            int i = y * W + x;
            if (!mask[i]) continue;

            if (x > 0 && mask[i - 1]) {
                uf.unite(i, i - 1);
            }
            if (y > 0 && mask[i - W]) {
                uf.unite(i, i - W);
            }
            if (params.eightConnected) {
                if (x > 0 && y > 0 && mask[i - W - 1]) {
                    uf.unite(i, i - W - 1);
                }
                if (x < W - 1 && y > 0 && mask[i - W + 1]) {
                    uf.unite(i, i - W + 1);
                }
            }
        }
    }

    // Step 3: bounding boxes per component root.
    std::map<int, Box> boxes;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            int i = y * W + x;
            if (!mask[i]) continue;
            int root = uf.find(i);

            std::map<int, Box>::iterator it = boxes.find(root);
            if (it == boxes.end()) {
                Box b = { x, y, x, y };
                boxes[root] = b;
            } else {
                Box& b = it->second;
                if (x < b.xMin) b.xMin = x;
                if (x > b.xMax) b.xMax = x;
                if (y < b.yMin) b.yMin = y;
                if (y > b.yMax) b.yMax = y;
            }
        }
    }

    // Step 4: filter by min size, apply padding, clamp, emit.
    for (std::map<int, Box>::iterator it = boxes.begin(); it != boxes.end(); ++it) {
        const Box& b = it->second;
        int boxW = b.xMax - b.xMin + 1;
        int boxH = b.yMax - b.yMin + 1;
        if (boxW < params.minSize || boxH < params.minSize) continue;

        Rectangle r;
        r.x = (float)(b.xMin - params.padding);
        r.y = (float)(b.yMin - params.padding);
        r.width  = (float)(boxW + 2 * params.padding);
        r.height = (float)(boxH + 2 * params.padding);

        if (r.x < 0.0f) { r.width  += r.x; r.x = 0.0f; }
        if (r.y < 0.0f) { r.height += r.y; r.y = 0.0f; }
        if (r.x + r.width  > (float)W) r.width  = (float)W - r.x;
        if (r.y + r.height > (float)H) r.height = (float)H - r.y;

        if (r.width > 0.0f && r.height > 0.0f) {
            result.push_back(r);
        }
    }

    // Step 5: optional merge of nearby boxes.
    if (params.mergeDistance > 0) {
        result = mergeNearbyRects(result, params.mergeDistance);
    }

    // Sort top-to-bottom, left-to-right for stable naming.
    std::sort(result.begin(), result.end(), rectCompare);

    return result;
}
