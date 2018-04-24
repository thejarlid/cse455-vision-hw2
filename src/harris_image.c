#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"
#include <time.h>

#define FOREACH_PIXEL(W, H, FUNC) \
for (int j = 0; j < H; ++j) { \
    for (int i = 0; i < W; ++i)  \
        FUNC \
} \

#define SQUARE(X) ((X) * (X))

// Frees an array of descriptors.
// descriptor *d: the array.
// int n: number of elements in array.
void free_descriptors(descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        free(d[i].data);
    }
    free(d);
}

// Create a feature descriptor for an index in an image.
// image im: source image.
// int i: index in image for the pixel we want to describe.
// returns: descriptor for that index.
descriptor describe_index(image im, int i)
{
    int w = 5;
    descriptor d;
    d.p.x = i%im.w;
    d.p.y = i/im.w;
    d.data = calloc(w*w*im.c, sizeof(float));
    d.n = w*w*im.c;
    int c, dx, dy;
    int count = 0;
    // If you want you can experiment with other descriptors
    // This subtracts the central value from neighbors
    // to compensate some for exposure/lighting changes.
    for(c = 0; c < im.c; ++c){
        float cval = im.data[c*im.w*im.h + i];
        for(dx = -w/2; dx < (w+1)/2; ++dx){
            for(dy = -w/2; dy < (w+1)/2; ++dy){
                float val = get_pixel(im, i%im.w+dx, i/im.w+dy, c);
                d.data[count++] = cval - val;
            }
        }
    }
    return d;
}

// Marks the spot of a point in an image.
// image im: image to mark.
// ponit p: spot to mark in the image.
void mark_spot(image im, point p)
{
    int x = p.x;
    int y = p.y;
    int i;
    for(i = -9; i < 10; ++i){
        set_pixel(im, x+i, y, 0, 1);
        set_pixel(im, x, y+i, 0, 1);
        set_pixel(im, x+i, y, 1, 0);
        set_pixel(im, x, y+i, 1, 0);
        set_pixel(im, x+i, y, 2, 1);
        set_pixel(im, x, y+i, 2, 1);
    }
}

// Marks corners denoted by an array of descriptors.
// image im: image to mark.
// descriptor *d: corners in the image.
// int n: number of descriptors to mark.
void mark_corners(image im, descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        mark_spot(im, d[i].p);
    }
}

// Creates a 1d Gaussian filter.
// float sigma: standard deviation of Gaussian.
// returns: single row image of the filter.
image make_1d_gaussian(float sigma)
{
    // TODO: optional, make separable 1d Gaussian.
    int w = (int)ceil(6 * sigma);
    w = w & 1 ? w : w + 1;
    int center = w / 2;
    image img = make_image(w, 1, 1);
    for (int i = 0; i < w; ++i) {
        // without multiplying 1 / (TWOPI * SQUARE(sigma))
        set_pixel(img, i, 0, 0, exp(-SQUARE(i - center) / (2 * SQUARE(sigma))));
    }
    return img;
}

// Smooths an image using separable Gaussian filter.
// image im: image to smooth.
// float sigma: std dev. for Gaussian.
// returns: smoothed image.
image smooth_image(image im, float sigma)
{
    if(1){
        image g = make_gaussian_filter(sigma);
        image s = convolve_image(im, g, 1);
        free_image(g);
        return s;
    } else {
        // TODO: optional, use two convolutions with 1d gaussian filter.
        // If you implement, disable the above if check.
        image g = make_1d_gaussian(sigma);
        image s1 = convolve_image(im, g, 1);
        g.h = g.w;
        g.w = 1;
        image s2 = convolve_image(s1, g, 1);
        double k = 1 / (TWOPI * SQUARE(sigma));
        for (int i = 0; i < s1.w * s1.h; ++i) {
            s1.data[i] *= k;
        }
        free_image(g);
        free_image(s1);
        return s2;
    }
}

// Calculate the structure matrix of an image.
// image im: the input image.
// float sigma: std dev. to use for weighted sum.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          third channel is IxIy.
image structure_matrix(image im, float sigma)
{
    image S = make_image(im.w, im.h, 3);
    // TODO: calculate structure matrix for im.
    image gx = make_gx_filter();
    image gy = make_gy_filter();
    image ix = convolve_image(im, gx, 0);
    image iy = convolve_image(im, gy, 0);
    free_image(gx);
    free_image(gy);
    FOREACH_PIXEL(im.w, im.h, {
        float x = get_pixel(ix, i, j, 0);
        float y = get_pixel(iy, i, j, 0);
        set_pixel(S, i, j, 0, x * x);
        set_pixel(S, i, j, 1, y * y);
        set_pixel(S, i, j, 2, x * y);
    });
    free_image(ix);
    free_image(iy);
    image result = smooth_image(S, sigma);
    free_image(S);
    return result;
}

// Estimate the cornerness of each pixel given a structure matrix S.
// image S: structure matrix for an image.
// returns: a response map of cornerness calculations.
image cornerness_response(image S)
{
    image R = make_image(S.w, S.h, 1);
    // TODO: fill in R, "cornerness" for each pixel using the structure matrix.
    // We'll use formulation det(S) - alpha * trace(S)^2, alpha = .06.
    float alpha = 0.06;
    FOREACH_PIXEL(S.w, S.h, {
        float xx = get_pixel(S, i, j, 0);
        float yy = get_pixel(S, i, j, 1);
        float xy = get_pixel(S, i, j, 2);
        float det = xx * yy - xy * xy;
        float trace = xx + yy;
        set_pixel(R, i, j, 0, det - alpha * trace * trace);
    });
    return R;
}

// Perform non-max supression on an image of feature responses.
// image im: 1-channel image of feature responses.
// int w: distance to look for larger responses.
// returns: image with only local-maxima responses within w pixels.
image nms_image(image im, int w)
{
    image r = copy_image(im);
    // TODO: perform NMS on the response map.
    // for every pixel in the image:
    //     for neighbors within w:
    //         if neighbor response greater than pixel response:
    //             set response to be very low (I use -999999 [why not 0??])
    int width = 2 * w + 1;
    int center_x = w;
    int center_y = w;
    FOREACH_PIXEL(r.w, r.h, {
        int cur_i = i;
        int cur_j = j;
        float pixel = get_pixel(im, cur_i, cur_j, 0);
        FOREACH_PIXEL(width, width, {
            if (get_pixel(im, cur_i - (center_x - i), cur_j - (center_y - j), 0) > pixel) {
                set_pixel(r, cur_i, cur_j, 0, -999999);
                goto suppressed;
            }
        });
suppressed:
        ;
    });
    return r;
}

// Perform harris corner detection and extract features from the corners.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
// int *n: pointer to number of corners detected, should fill in.
// returns: array of descriptors of the corners in the image.
descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms, int *n)
{
    // Calculate structure matrix
    image S = structure_matrix(im, sigma);

    // Estimate cornerness
    image R = cornerness_response(S);

    // Run NMS on the responses
    image Rnms = nms_image(R, nms);

    //TODO: count number of responses over threshold
    int count = 0; // change this
    int capacity = 64;
    int* arr = (int*)malloc(capacity * sizeof(int));
    FOREACH_PIXEL(Rnms.w, Rnms.h, {
        if (get_pixel(Rnms, i, j, 0) >= thresh) {
            ++count;
            if (count > capacity) {
                capacity <<= 1;
                int* new_arr = (int*)malloc(capacity * sizeof(int));
                memcpy(new_arr, arr, (capacity >> 1) * sizeof(int));
                free(arr);
                arr = new_arr;
            }
            arr[count - 1] = i + j * Rnms.w;
        }
    });
    
    *n = count; // <- set *n equal to number of corners in image.
    descriptor *d = calloc(count, sizeof(descriptor));
    //TODO: fill in array *d with descriptors of corners, use describe_index.
    descriptor *ptr = d;
    for (int i = 0; i < count; ++i) {
        *ptr++ = describe_index(im, arr[i]);
    }
    free(arr);

    free_image(S);
    free_image(R);
    free_image(Rnms);
    return d;
}

// Find and draw corners on an image.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
void detect_and_draw_corners(image im, float sigma, float thresh, int nms)
{
    int n = 0;
    descriptor *d = harris_corner_detector(im, sigma, thresh, nms, &n);
    mark_corners(im, d, n);
}
