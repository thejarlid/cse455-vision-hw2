#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

#define FOREACH_PIXEL(W, H, FUNC) \
for (int j = 0; j < H; ++j) { \
    for (int i = 0; i < W; ++i)  \
        FUNC \
} \

#define SQUARE(X) ((X) * (X))
#define ABS(X) (fabs(X))

void l1_normalize(image im)
{
    // TODO
    for (int c = 0; c < im.c; ++c) {
        float sum = 0;
        FOREACH_PIXEL(im.w, im.h,
            sum += get_pixel(im, i, j, c);
        )
        if (sum != 0) {
            FOREACH_PIXEL(im.w, im.h, 
                set_pixel(im, i, j, c, get_pixel(im, i, j, c) / sum);
            )
        }
    }
}

image make_box_filter(int w)
{
    // TODO
    image im = make_image(w, w, 1);
    FOREACH_PIXEL(im.w, im.h,
        set_pixel(im, i, j, 0, 1.0 / w / w);
    )
    return im;
}

image convolve_image(image im, image filter, int preserve)
{
    // TODO
    assert(im.c == filter.c || filter.c == 1);
    image new_img = make_image(im.w, im.h, preserve ? im.c : 1);
    assert(filter.w % 2);
    int center_x = filter.w / 2, center_y = filter.h / 2;
    for (int c = 0; c < im.c; ++c) {
        FOREACH_PIXEL(new_img.w, new_img.h, {
            int cur_i = i;
            int cur_j = j;
            float q = preserve ? 0 : get_pixel(new_img, cur_i, cur_j, 0);
            FOREACH_PIXEL(filter.w, filter.h, 
                q += get_pixel(im, cur_i - (center_x - i), cur_j - (center_y - j), c) * get_pixel(filter, i, j, filter.c == 1 ? 0 : c);
            );
            set_pixel(new_img, cur_i, cur_j, preserve ? c : 0, q);
        });
    }
    return new_img;
}

image make_highpass_filter()
{
    // TODO
    image img = make_image(3, 3, 1);
    float data[9] = {0, -1, 0, 
                     -1, 4, -1,
                     0, -1, 0};
    memcpy(img.data, data, sizeof(data));
    return img;
}

image make_sharpen_filter()
{
    // TODO
    image img = make_image(3, 3, 1);
    float data[9] = {0, -1, 0, 
                     -1, 5, -1,
                     0, -1, 0};
    memcpy(img.data, data, sizeof(data));
    return img;
}

image make_emboss_filter()
{
    // TODO
    image img = make_image(3, 3, 1);
    float data[9] = {-2, -1, 0, 
                     -1, 1, 1,
                     0, 1, 2};
    memcpy(img.data, data, sizeof(data));
    return img;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: We should se preserve for sharpen and emboss filters. The highpass filter emphasizes only the edges, so the color is not necessary. 

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: We should use image clamp for all three filters. All three filters may cause our pixel value exceed the piexel value bounds.

image make_gaussian_filter(float sigma)
{
    // TODO
    int w = (int)ceil(6 * sigma);
    w = w & 1 ? w : w + 1;
    int center = w / 2;
    image img = make_image(w, w, 1);
    FOREACH_PIXEL(img.w, img.w, 
        set_pixel(img, i, j, 0, 1 / (TWOPI * SQUARE(sigma)) * exp(-(SQUARE(i - center) + SQUARE(j - center)) / (2 * SQUARE(sigma))));
    );
    return img;
}

image add_image(image a, image b)
{
    // TODO
    assert(a.w == b.w && a.h == b.h && a.c == b.c);
    image img = make_image(a.w, a.h, a.c);
    for (int c = 0; c < a.c; ++c) {
        FOREACH_PIXEL(a.w, a.h, {
            set_pixel(img, i, j, c, get_pixel(a, i, j, c) + get_pixel(b, i, j, c));
        });
    }
    return img;
}

image sub_image(image a, image b)
{
    // TODO
    assert(a.w == b.w && a.h == b.h && a.c == b.c);
    image img = make_image(a.w, a.h, a.c);
    for (int c = 0; c < a.c; ++c) {
        FOREACH_PIXEL(a.w, a.h, {
            set_pixel(img, i, j, c, (float)get_pixel(a, i, j, c) - get_pixel(b, i, j, c));
        });
    }
    return img;
}

image make_gx_filter()
{
    // TODO
    image img = make_image(3, 3, 1);
    float data[9] = {-1, 0, 1, 
                     -2, 0, 2,
                     -1, 0, 1};
    memcpy(img.data, data, sizeof(data));
    return img;
}

image make_gy_filter()
{
    // TODO
    image img = make_image(3, 3, 1);
    float data[9] = {-1, -2, -1, 
                     0, 0, 0,
                     1, 2, 1};
    memcpy(img.data, data, sizeof(data));
    return img;
}

void feature_normalize(image im)
{
    // TODO
    float min, max;
    min = max = get_pixel(im, 0, 0, 0);
    for (int c = 0; c < im.c; ++c) {
        FOREACH_PIXEL(im.w, im.h, {
            float p = get_pixel(im, i, j, c);
            if (p < min) min = p; else if (p > max) max = p;
        });
    }
    float range = max - min;
    for (int c = 0; c < im.c; ++c) {
        FOREACH_PIXEL(im.w, im.h, {
            float p = get_pixel(im, i, j, c);
            set_pixel(im, i, j, c, range == 0 ? 0 : (p - min) / range);
        });
    }
}

image *sobel_image(image im)
{
    // TODO
    image * result = calloc(2, sizeof(image));
    image gx_filter = make_gx_filter();
    image gy_filter = make_gy_filter();
    image gx = convolve_image(im, gx_filter, 0);
    image gy = convolve_image(im, gy_filter, 0);
    free_image(gx_filter);
    free_image(gy_filter);
    result[0] = make_image(gx.w, gx.h, 1);
    result[1] = make_image(gx.w, gx.h, 1);
    FOREACH_PIXEL(gx.w, gx.h, {
        set_pixel(result[0], i, j, 0, sqrt(SQUARE(get_pixel(gx, i, j, 0)) + SQUARE(get_pixel(gy, i, j, 0))));
        float angle = atan2(get_pixel(gy, i, j, 0), get_pixel(gx, i, j, 0));
        set_pixel(result[1], i, j, 0, angle);
    });
    free_image(gx);
    free_image(gy);
    return result;
}

image colorize_sobel(image im)
{
    // TODO
    image gx_filter = make_gx_filter();
    image gy_filter = make_gy_filter();
    image gx = convolve_image(im, gx_filter, 0);
    image gy = convolve_image(im, gy_filter, 0);
    free_image(gx_filter);
    free_image(gy_filter);
    image result = make_image(gx.w, gx.h, 3);
    FOREACH_PIXEL(gx.w, gx.h, {
        set_pixel(result, i, j, 0, atan2(get_pixel(gy, i, j, 0),  get_pixel(gx, i, j, 0)));
        set_pixel(result, i, j, 1, sqrt(SQUARE(get_pixel(gx, i, j, 0)) + SQUARE(get_pixel(gy, i, j, 0))));
        set_pixel(result, i, j, 2, sqrt(SQUARE(get_pixel(gx, i, j, 0)) + SQUARE(get_pixel(gy, i, j, 0))));
    });
    free_image(gx);
    free_image(gy);
    feature_normalize(result);
    hsv_to_rgb(result);
    return result;
}
