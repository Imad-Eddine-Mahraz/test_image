#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"
#include "t_bmp24.h"

// ----------- Réutilisation des noyaux 3x3 -----------
float kernel_box_blur[3][3] = {
    {1.0/9, 1.0/9, 1.0/9},
    {1.0/9, 1.0/9, 1.0/9},
    {1.0/9, 1.0/9, 1.0/9}
};

float kernel_gaussian_blur[3][3] = {
    {1.0/16, 2.0/16, 1.0/16},
    {2.0/16, 4.0/16, 2.0/16},
    {1.0/16, 2.0/16, 1.0/16}
};

float kernel_outline[3][3] = {
    {-1, -1, -1},
    {-1,  8, -1},
    {-1, -1, -1}
};

float kernel_emboss[3][3] = {
    {-2, -1, 0},
    {-1,  1, 1},
    { 0,  1, 2}
};

float kernel_sharpen[3][3] = {
    { 0, -1,  0},
    {-1,  5, -1},
    { 0, -1,  0}
};

float **toFloatPtr(float kernel[3][3]) {
    float **result = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        result[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) {
            result[i][j] = kernel[i][j];
        }
    }
    return result;
}

void freeKernel(float **kernel) {
    for (int i = 0; i < 3; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

// ----------- Fonctions de chaque "main" -----------
void runBMP8BaseFilters() {
    const char *input = "../image/barbara_gray.bmp";

    t_bmp8 *img = bmp8_loadImage(input);
    if (!img) {
        printf("Erreur de chargement de l'image.\n");
        return;
    }

    bmp8_negative(img);
    bmp8_saveImage("../Image/barbara_gray_negative.bmp", img);

    img = bmp8_loadImage(input);
    bmp8_brightness(img, 50);
    bmp8_saveImage("../Image/barbara_gray_brightness.bmp", img);

    img = bmp8_loadImage(input);
    bmp8_threshold(img, 128);
    bmp8_saveImage("../Image/barbara_gray_threshold.bmp", img);

    bmp8_free(img);
    printf("Filtres de base BMP8 appliquess.\n");
}

void runBMP8ConvolutionFilters() {
    const char *input = "../image/barbara_gray.bmp";
    t_bmp8 *img;
    float **k;

    img = bmp8_loadImage(input);
    k = toFloatPtr(kernel_box_blur);
    bmp8_applyFilter(img, k, 3);
    bmp8_saveImage("../Image/barbara_gray_boxblur.bmp", img);
    bmp8_free(img);
    freeKernel(k);

    img = bmp8_loadImage(input);
    k = toFloatPtr(kernel_gaussian_blur);
    bmp8_applyFilter(img, k, 3);
    bmp8_saveImage("../Image/barbara_gray_gaussian.bmp", img);
    bmp8_free(img);
    freeKernel(k);

    img = bmp8_loadImage(input);
    k = toFloatPtr(kernel_outline);
    bmp8_applyFilter(img, k, 3);
    bmp8_saveImage("../Image/barbara_gray_outline.bmp", img);
    bmp8_free(img);
    freeKernel(k);

    img = bmp8_loadImage(input);
    k = toFloatPtr(kernel_emboss);
    bmp8_applyFilter(img, k, 3);
    bmp8_saveImage("../Image/barbara_gray_emboss.bmp", img);
    bmp8_free(img);
    freeKernel(k);

    img = bmp8_loadImage(input);
    k = toFloatPtr(kernel_sharpen);
    bmp8_applyFilter(img, k, 3);
    bmp8_saveImage("../Image/barbara_gray_sharpen.bmp", img);
    bmp8_free(img);
    freeKernel(k);

    printf("Filtres convolution BMP8 appliques.\n");
}

void runBMP8Equalization() {
    t_bmp8 *img = bmp8_loadImage("../image/barbara_gray.bmp");
    if (!img) {
        printf("Erreur de chargement.\n");
        return;
    }

    unsigned int *hist = bmp8_computeHistogram(img);
    unsigned int hist_eq[256] = {0};

    bmp8_equalize(img, hist, hist_eq);
    bmp8_saveImage("../Image/barbara_gray_equalized.bmp", img);

    free(hist);
    bmp8_free(img);

    printf("Egalisation BMP8 appliquee.\n");
}

void runBMP24Filters() {
    const char *input = "../image/flowers_color.bmp";

    t_bmp24 *img = bmp24_loadImage(input);
    if (!img) {
        printf("Erreur chargement image couleur.\n");
        return;
    }

    img = bmp24_loadImage(input);
    bmp24_negative(img);
    bmp24_saveImage(img, "../Image/flowers_negative.bmp");

    img = bmp24_loadImage(input);
    bmp24_grayscale(img);
    bmp24_saveImage(img, "../Image/flowers_grayscale.bmp");

    img = bmp24_loadImage(input);
    bmp24_brightness(img, 50);
    bmp24_saveImage(img, "../Image/flowers_brightness.bmp");

    float **k;

    img = bmp24_loadImage(input);
    k = toFloatPtr(kernel_box_blur);
    bmp24_applyFilter(img, k, 3);
    bmp24_saveImage(img, "../Image/flowers_boxblur.bmp");
    freeKernel(k);

    img = bmp24_loadImage(input);
    k = toFloatPtr(kernel_gaussian_blur);
    bmp24_applyFilter(img, k, 3);
    bmp24_saveImage(img, "../Image/flowers_gaussian.bmp");
    freeKernel(k);

    img = bmp24_loadImage(input);
    k = toFloatPtr(kernel_outline);
    bmp24_applyFilter(img, k, 3);
    bmp24_saveImage(img, "../Image/flowers_outline.bmp");
    freeKernel(k);

    img = bmp24_loadImage(input);
    k = toFloatPtr(kernel_emboss);
    bmp24_applyFilter(img, k, 3);
    bmp24_saveImage(img, "../Image/flowers_emboss.bmp");
    freeKernel(k);

    img = bmp24_loadImage(input);
    k = toFloatPtr(kernel_sharpen);
    bmp24_applyFilter(img, k, 3);
    bmp24_saveImage(img, "../Image/flowers_sharpen.bmp");
    freeKernel(k);

    bmp24_free(img);

    printf("Filtres BMP24 appliqués.\n");
}

void runBMP24Equalization() {
    t_bmp24 *img = bmp24_loadImage("../image/flowers_color.bmp");
    if (!img) {
        printf("Erreur chargement BMP24.\n");
        return;
    }
    bmp24_equalize(img);
    bmp24_saveImage(img, "../Image/flowers_equalize.bmp");
    bmp24_free(img);

    printf(" Egalisation BMP24 appliquee.\n");
}

// ----------- Main principal avec menu -----------
int main() {
    int choix = -1;

    do {
        printf("\n=== MENU PRINCIPAL ===\n");
        printf("1. Filtres simples BMP8 (negatif, luminosite, seuillage)\n");
        printf("2. Filtres convolutifs BMP8\n");
        printf("3. Egalisation d histogramme BMP8\n");
        printf("4. Filtres BMP24 (couleur)\n");
        printf("5. Egalisation BMP24 (couleur)\n");
        printf("0. Quitter\n> ");
        scanf("%d", &choix);

        switch (choix) {
            case 1: runBMP8BaseFilters(); break;
            case 2: runBMP8ConvolutionFilters(); break;
            case 3: runBMP8Equalization(); break;
            case 4: runBMP24Filters(); break;
            case 5: runBMP24Equalization(); break;
            case 0: printf("Fin du programme.\n"); break;
            default: printf("Choix invalide.\n"); break;
        }
    } while (choix != 0);

    return 0;
}

