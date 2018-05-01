#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

static void clamp(int *a, int min, int max) {
    *a = *a < min ? min : *a;
    *a = *a > max ? max : *a;
}

float get_pixel(image im, int x, int y, int c)
{
    // TODO Fill this in
    clamp(&x, 0, im.w - 1);
    clamp(&y, 0, im.h - 1);
    clamp(&c, 0, im.c - 1);
    return im.data[c * im.w * im.h + y * im.w + x];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    // TODO Fill this in
    if (x < 0 || x >= im.w) return;
    if (y < 0 || y >= im.h) return;
    if (c < 0 || c >= im.c) return;
    im.data[c * im.w * im.h + y * im.w + x] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    // TODO Fill this in
    memcpy(copy.data, im.data, im.w * im.h * im.c * sizeof(float));
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    // TODO Fill this in
    for (int i = 0; i < im.w * im.h; ++i) {
        gray.data[i] = 0.299 * im.data[i] + 0.587 * im.data[i + im.w * im.h] + 0.114 * im.data[i + 2 * im.w * im.h];
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    // TODO Fill this in
    for (int i = 0; i < im.w * im.h; ++i) {
        im.data[im.w * im.h * c + i] += v;
    }
}

void clamp_image(image im)
{
    // TODO Fill this in
    for (int i = 0; i < im.w * im.h * im.c; ++i) {
        im.data[i] = im.data[i] < 0 ? 0 : im.data[i];
        im.data[i] = im.data[i] > 1 ? 1 : im.data[i];
    }
}

// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    // TODO Fill this in
    for (int i = 0; i < im.w * im.h; ++i) {
        float r = im.data[i];
        float g = im.data[i + im.w * im.h];
        float b = im.data[i + 2 * im.w * im.h];
        float v = three_way_max(r, g, b);
        float m = three_way_min(r, g, b);
        float c = v - m;
        float s = v != 0 ? c / v : 0;
        float h;
        if (c == 0) {
            h = 0;
        } else {
            if (v == r) {
                h = (g - b) / c;
            } else if (v == g) {
                h = (b - r) / c + 2;
            } else { // v == b
                h = (r - g) / c + 4;
            }
        }
        im.data[i] = h < 0 ? h / 6 + 1 : h / 6;
        im.data[i + im.w * im.h] = s;
        im.data[i + 2 * im.w * im.h] = v;
    }
}

void hsv_to_rgb(image im)
{
    // TODO Fill this in
    for (int i = 0; i < im.w * im.h; ++i) {
        float h = im.data[i] * 6;
        float s = im.data[i + im.w * im.h];
        float v = im.data[i + 2 * im.w * im.h];
        float r = 0, g = 0, b = 0;
        if (s == 0) {
            r = g = b = v;
        } else {
            int index;
            double f, p, q, t;

            index = (int)h;
            f = h - index;

            p = v * (1 - s);
            q = v * (1 - (s * f));
            t = v * (1 - (s * (1 - f)));

            switch(index) {
                case 0:
                    r = v;
                    g = t;
                    b = p;
                    break;
                case 1:
                    r = q;
                    g = v;
                    b = p;
                    break;
                case 2:
                    r = p;
                    g = v;
                    b = t;
                    break;
                case 3:
                    r = p;
                    g = q;
                    b = v;
                    break;

                case 4:
                    r = t;
                    g = p;
                    b = v;
                    break;
                default:
                    r = v;
                    g = p;
                    b = q;
                    break;
            }
        }
        im.data[i] = r;
        im.data[i + im.w * im.h] = g;
        im.data[i + 2 * im.w * im.h] = b;
    }
}
