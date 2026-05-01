#include "ops/Trim.h"

Rectangle trimTransparentEdges(const Image& image, Rectangle r, int alphaThreshold) {
    if (image.data == nullptr) return r;

    int x0 = (int)r.x;
    int y0 = (int)r.y;
    int x1 = x0 + (int)r.width;
    int y1 = y0 + (int)r.height;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > image.width)  x1 = image.width;
    if (y1 > image.height) y1 = image.height;
    if (x0 >= x1 || y0 >= y1) return r;

    int left = x1, right = x0 - 1, top = y1, bottom = y0 - 1;
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            Color c = GetImageColor(image, x, y);
            if ((int)c.a > alphaThreshold) {
                if (x < left)   left   = x;
                if (x > right)  right  = x;
                if (y < top)    top    = y;
                if (y > bottom) bottom = y;
            }
        }
    }

    if (right < left || bottom < top) return r;

    Rectangle out;
    out.x = (float)left;
    out.y = (float)top;
    out.width  = (float)(right - left + 1);
    out.height = (float)(bottom - top + 1);
    return out;
}
