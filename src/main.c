#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>


void print_help() {
    printf("\nffjpeg\n\n");
    printf("usage:\n");
    printf("  ffjpeg input output [flags]\n\n");
    printf("flags:\n");
    printf("  --help              Show this help\n");
    printf("  --info              Print image info\n");
    printf("  --quality N         JPEG quality (1-100)\n");
    printf("  --grayscale         Convert to grayscale\n");
    printf("  --flip-y            Flip image vertically\n");
    printf("  --colors N          Limit total colors\n");
    printf("  --resize WxH        Resize image (example: 1920x1080)\n\n");
    printf("examples:\n");
    printf("  ffjpeg in.jpg out.jpg --resize 23040x12960 --quality 90\n");
    printf("  ffjpeg in.jpg out.jpg --colors 32 --quality 10\n\n");
}


EXPORT void flip_y(unsigned char *img, int w, int h, int c) {
    for (int y = 0; y < h / 2; y++) {
        unsigned char *row1 = img + y * w * c;
        unsigned char *row2 = img + (h - y - 1) * w * c;
		
		#pragma omp parallel for
        for (int x = 0; x < w * c; x++) {
            unsigned char t = row1[x];
            row1[x] = row2[x];
            row2[x] = t;
        }
    }
}

EXPORT unsigned char *to_grayscale(unsigned char *img, int w, int h, int *c) {
    if (*c < 3) return img;

    unsigned char *g = malloc(w * h);
    if (!g) return img;
	
	#pragma omp parallel for
    for (int i = 0; i < w * h; i++) {
        int r = img[i * (*c) + 0];
        int gch = img[i * (*c) + 1];
        int b = img[i * (*c) + 2];
        g[i] = (unsigned char)(0.299f * r + 0.587f * gch + 0.114f * b);
    }

    *c = 1;
    stbi_image_free(img);
    return g;
}

EXPORT void limit_colors(unsigned char *img, int w, int h, int c, int total_colors) {
    if (c < 3 || total_colors < 2) return;

    int levels = (int)round(cbrt((double)total_colors));
    if (levels < 2) levels = 2;
    if (levels > 256) levels = 256;

    float step = 255.0f / (levels - 1);
	
	#pragma omp parallel for
    for (int i = 0; i < w * h; i++) {
        for (int ch = 0; ch < 3; ch++) {
            float v = img[i * c + ch];
            int q = (int)round(v / step);
            float nv = q * step;
            if (nv < 0) nv = 0;
            if (nv > 255) nv = 255;
            img[i * c + ch] = (unsigned char)nv;
        }
    }
}

EXPORT unsigned char *resize_bilinear(
    unsigned char *src,
    int sw, int sh, int c,
    int dw, int dh
) {
    unsigned char *dst = malloc(dw * dh * c);
    if (!dst) return NULL;

    float x_ratio = (float)(sw - 1) / dw;
    float y_ratio = (float)(sh - 1) / dh;
	
	#pragma omp parallel for
    for (int y = 0; y < dh; y++) {
        for (int x = 0; x < dw; x++) {
            float gx = x * x_ratio;
            float gy = y * y_ratio;

            int x0 = (int)gx;
            int y0 = (int)gy;
            int x1 = x0 + 1 < sw ? x0 + 1 : x0;
            int y1 = y0 + 1 < sh ? y0 + 1 : y0;

            float dx = gx - x0;
            float dy = gy - y0;

            for (int ch = 0; ch < c; ch++) {
                int i00 = (y0 * sw + x0) * c + ch;
                int i10 = (y0 * sw + x1) * c + ch;
                int i01 = (y1 * sw + x0) * c + ch;
                int i11 = (y1 * sw + x1) * c + ch;

                float v =
                    src[i00] * (1 - dx) * (1 - dy) +
                    src[i10] * dx * (1 - dy) +
                    src[i01] * (1 - dx) * dy +
                    src[i11] * dx * dy;

                dst[(y * dw + x) * c + ch] = (unsigned char)(v);
            }
        }
    }

    return dst;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    int quality = 90;
    int do_gray = 0;
    int do_flip = 0;
    int show_info = 0;
    int color_limit = -1;
    int resize_w = 0, resize_h = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            print_help();
            return 0;
        }
    }

    if (argc < 3) {
        print_help();
        return 0;
    }

    for (int i = 3; i < argc; i++) {
        if (!strcmp(argv[i], "--quality"))
            quality = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--grayscale"))
            do_gray = 1;
        else if (!strcmp(argv[i], "--flip-y"))
            do_flip = 1;
        else if (!strcmp(argv[i], "--info"))
            show_info = 1;
        else if (!strcmp(argv[i], "--colors"))
            color_limit = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--resize"))
            sscanf(argv[++i], "%dx%d", &resize_w, &resize_h);
    }

    int w, h, c;
    unsigned char *img = stbi_load(argv[1], &w, &h, &c, 0);
    if (!img) {
        printf("failed to load image\n");
        return 1;
    }

    if (show_info)
        printf("width=%d height=%d channels=%d\n", w, h, c);

    if (do_flip)
        flip_y(img, w, h, c);

    if (do_gray)
        img = to_grayscale(img, w, h, &c);

    if (color_limit > 0)
        limit_colors(img, w, h, c, color_limit);

    if (resize_w > 0 && resize_h > 0) {
        unsigned char *resized =
            resize_bilinear(img, w, h, c, resize_w, resize_h);

        if (resized) {
            stbi_image_free(img);
            img = resized;
            w = resize_w;
            h = resize_h;
        }
    }

    if (quality < 1) quality = 1;
    if (quality > 100) quality = 100;

    if (!stbi_write_jpg(argv[2], w, h, c, img, quality))
        printf("failed to write image\n");

    stbi_image_free(img);
    return 0;
}
