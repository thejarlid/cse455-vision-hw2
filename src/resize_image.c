#include <math.h>
#include "image.h"

#define ABS(X) ((X) < 0 ? -(X) : (X))

#define FOREACH_PIXEL(W, H, FUNC) \
for (int j = 0; j < H; ++j) { \
    for (int i = 0; i < W; ++i)  \
        FUNC \
} \

static image resize(image im, int w, int h, float (*func_ptr)(image, float, float, int)) {
    image new_image = make_image(w, h, im.c);
    for (int c = 0; c < im.c; ++c) {
        FOREACH_PIXEL(w, h, {
            float x = (i + 0.5) / w * im.w - 0.5;
            float y = (j + 0.5) / h * im.h - 0.5;
            set_pixel(new_image, i, j, c, (*func_ptr)(im, x, y, c));
        });
    }
    return new_image;
}

float nn_interpolate(image im, float x, float y, int c)
{
    // TODO Fill in
    return get_pixel(im, round(x), round(y), c);
}

image nn_resize(image im, int w, int h)
{
    // TODO Fill in (also fix that first line)
    return resize(im, w, h, nn_interpolate);
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    // TODO
    float v[4], a[4];
    int index = 0;
    for (int dy = 0; dy <= 1; ++dy) {
        for (int dx = 0; dx <= 1; ++dx) {
            v[index] = get_pixel(im, (int)floor(x) + dx, (int)floor(y) + dy, c);
            a[3 - index] = ABS(((int)floor(x) + dx) - x) * ABS(((int)floor(y) + dy) - y);
            ++index;
        }
    }
    float sum = 0;
    for (index = 0; index < 4; ++index) {
        sum += v[index] * a[index];
    }
    return sum;
}

image bilinear_resize(image im, int w, int h)
{
    // TODO
    return resize(im, w, h, bilinear_interpolate);
}

