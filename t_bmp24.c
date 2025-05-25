#include <stdio.h>
#include <stdlib.h>
#include "t_bmp24.h"

t_pixel ** bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = malloc(height * sizeof(t_pixel *));
    if (pixels == NULL) {
        printf("Erreur d'allocation mémoire pour les lignes de pixels.\n");
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        pixels[i] = malloc(width * sizeof(t_pixel));
        if (pixels[i] == NULL) {
            printf("Erreur d'allocation mémoire pour la ligne %d.\n", i);
            // Libération partielle si erreur
            for (int j = 0; j < i; j++) {
                free(pixels[j]);
            }
            free(pixels);
            return NULL;
        }
    }

    return pixels;
}

void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    if (pixels == NULL) return;
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

t_bmp24 * bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = malloc(sizeof(t_bmp24));
    if (img == NULL) {
        printf("Erreur d'allocation mémoire pour l'image.\n");
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;
    img->data = bmp24_allocateDataPixels(width, height);

    if (img->data == NULL) {
        free(img);
        printf("Erreur lors de l'allocation des données de pixels.\n");
        return NULL;
    }

    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (img != NULL) {
        bmp24_freeDataPixels(img->data, img->height);
        free(img);
    }
}

#include <stdio.h>
#include <stdint.h>

void file_rawRead(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    if (file == NULL || buffer == NULL) {
        printf("Erreur : pointeur de fichier ou buffer NULL.\n");
        return;
    }

    if (fseek(file, position, SEEK_SET) != 0) {
        printf("Erreur : échec du déplacement à la position %u.\n", position);
        return;
    }

    size_t read = fread(buffer, size, n, file);
    if (read != n) {
        printf("Avertissement : lecture partielle (%zu/%zu éléments)\n", read, (size_t)n);
    }
}


void file_rawWrite(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    if (file == NULL || buffer == NULL) {
        printf("Erreur : pointeur de fichier ou de buffer NULL.\n");
        return;
    }

    if (fseek(file, position, SEEK_SET) != 0) {
        printf("Erreur : échec du déplacement à la position %u.\n", position);
        return;
    }

    size_t written = fwrite(buffer, size, n, file);
    if (written != n) {
        printf("Avertissement : écriture partielle (%zu/%zu éléments)\n", written, (size_t)n);
    }
}

void bmp24_readPixelValue(t_bmp24 *img, int x, int y, FILE *file) {
    unsigned char bgr[3];
    fread(bgr, sizeof(unsigned char), 3, file);
    img->data[y][x].blue = bgr[0];
    img->data[y][x].green = bgr[1];
    img->data[y][x].red = bgr[2];
}



void bmp24_readPixelData(t_bmp24 *img, FILE *file) {
    int width = img->width;
    int height = img->height;
    int rowSize = (width * 3 + 3) & ~3; // multiple de 4
    int padding = rowSize - width * 3;

    fseek(file, img->header.offset, SEEK_SET);

    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            bmp24_readPixelValue(img, x, y, file);
        }
        fseek(file, padding, SEEK_CUR);
    }
}



void bmp24_writePixelValue(t_bmp24 *img, int x, int y, FILE *file) {
    unsigned char bgr[3];
    bgr[0] = img->data[y][x].blue;
    bgr[1] = img->data[y][x].green;
    bgr[2] = img->data[y][x].red;
    fwrite(bgr, sizeof(unsigned char), 3, file);
}



void bmp24_writePixelData(t_bmp24 *img, FILE *file) {
    int width = img->width;
    int height = img->height;
    int rowSize = (width * 3 + 3) & ~3;
    int padding = rowSize - width * 3;
    unsigned char pad[3] = {0, 0, 0}; // max 3 octets

    fseek(file, img->header.offset, SEEK_SET);

    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            bmp24_writePixelValue(img, x, y, file);
        }
        fwrite(pad, sizeof(unsigned char), padding, file);
    }
}


t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *file = NULL;

    if (filename == NULL) {
        printf("Erreur : nom de fichier NULL.\n");
        return NULL;
    }

    file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Erreur : impossible d'ouvrir le fichier '%s'\n", filename);
        return NULL;
    }

    int width = 0;
    int height = 0;
    int colorDepth = 0;

    // Lecture des métadonnées dans l'en-tête
    file_rawRead(BITMAP_WIDTH, &width, sizeof(width), 1, file);
    file_rawRead(BITMAP_HEIGHT, &height, sizeof(height), 1, file);
    file_rawRead(BITMAP_DEPTH, &colorDepth, sizeof(colorDepth), 1, file);

    // Allocation de la structure image
    t_bmp24 *bmp24 = bmp24_allocate(width, height, colorDepth);
    if (bmp24 == NULL) {
        fclose(file);
        return NULL;
    }

    // Lecture des en-têtes
    file_rawRead(BITMAP_MAGIC, &(bmp24->header), sizeof(bmp24->header), 1, file);
    file_rawRead(sizeof(bmp24->header), &(bmp24->header_info), sizeof(bmp24->header_info), 1, file);

    // Lecture des pixels
    bmp24_readPixelData(bmp24, file);

    fclose(file);
    return bmp24;
}


void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    FILE *file = NULL;

    if (img == NULL || filename == NULL) {
        printf("Erreur : image ou nom de fichier NULL.\n");
        return;
    }

    file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Erreur lors de l'ouverture du fichier '%s' en écriture.\n", filename);
        return;
    }

    // Écriture de l'en-tête (BITMAP_MAGIC = 0)
    file_rawWrite(BITMAP_MAGIC, &img->header, sizeof(img->header), 1, file);

    // Écriture de l'en-tête info (juste après le header, offset 14)
    file_rawWrite(sizeof(img->header), &img->header_info, sizeof(img->header_info), 1, file);

    // Écriture des données pixel
    bmp24_writePixelData(img, file);

    fclose(file);
    printf("Image enregistree avec succes dans \"%s\"\n", filename);
}


void bmp24_negative(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red   = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue  = 255 - img->data[y][x].blue;
        }
    }
}

void bmp24_grayscale(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t r = img->data[y][x].red;
            uint8_t g = img->data[y][x].green;
            uint8_t b = img->data[y][x].blue;
            uint8_t gray = (r + g + b) / 3;
            img->data[y][x].red = img->data[y][x].green = img->data[y][x].blue = gray;
        }
    }
}

void bmp24_brightness(t_bmp24 *img, int value) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int r = img->data[y][x].red + value;
            int g = img->data[y][x].green + value;
            int b = img->data[y][x].blue + value;

            img->data[y][x].red   = (r > 255) ? 255 : (r < 0) ? 0 : r;
            img->data[y][x].green = (g > 255) ? 255 : (g < 0) ? 0 : g;
            img->data[y][x].blue  = (b > 255) ? 255 : (b < 0) ? 0 : b;
        }
    }
}

t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float **kernel, int kernelSize) {
    int offset = kernelSize / 2;
    float sumR = 0, sumG = 0, sumB = 0;

    for (int ky = -offset; ky <= offset; ky++) {
        for (int kx = -offset; kx <= offset; kx++) {
            int px = x + kx;
            int py = y + ky;

            // Vérifier que les indices sont dans les bornes de l'image
            if (px >= 0 && px < img->width && py >= 0 && py < img->height) {
                t_pixel p = img->data[py][px];
                float coeff = kernel[ky + offset][kx + offset];
                sumR += p.red * coeff;
                sumG += p.green * coeff;
                sumB += p.blue * coeff;
            }
        }
    }

    // Clamp les valeurs dans [0, 255]
    t_pixel result;
    result.red   = (uint8_t)(sumR > 255 ? 255 : (sumR < 0 ? 0 : sumR));
    result.green = (uint8_t)(sumG > 255 ? 255 : (sumG < 0 ? 0 : sumG));
    result.blue  = (uint8_t)(sumB > 255 ? 255 : (sumB < 0 ? 0 : sumB));
    return result;
}

void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize) {
    t_bmp24 *copy = bmp24_allocate(img->width, img->height, img->colorDepth);

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            if (x >= kernelSize / 2 && x < img->width - kernelSize / 2 &&
                y >= kernelSize / 2 && y < img->height - kernelSize / 2) {
                copy->data[y][x] = bmp24_convolution(img, x, y, kernel, kernelSize);
                } else {
                    copy->data[y][x] = img->data[y][x];  // Copie directe en bordure
                }
        }
    }

    // Remplace les données
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x] = copy->data[y][x];
        }
    }

    bmp24_free(copy);
}

void bmp24_boxBlur(t_bmp24 *img) {
    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) kernel[i][j] = 1.0f / 9.0f;
    }
    bmp24_applyFilter(img, kernel, 3);
    for (int i = 0; i < 3; i++) free(kernel[i]);
    free(kernel);
}

void bmp24_gaussianBlur(t_bmp24 *img) {
    float g[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };
    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) kernel[i][j] = g[i][j] / 16.0f;
    }
    bmp24_applyFilter(img, kernel, 3);
    for (int i = 0; i < 3; i++) free(kernel[i]);
    free(kernel);
}


void bmp24_outline(t_bmp24 *img) {
    float values[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };
    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) kernel[i][j] = values[i][j];
    }
    bmp24_applyFilter(img, kernel, 3);
    for (int i = 0; i < 3; i++) free(kernel[i]);
    free(kernel);
}


void bmp24_emboss(t_bmp24 *img) {
    float values[3][3] = {
        {-2, -1, 0},
        {-1,  1, 1},
        { 0,  1, 2}
    };
    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) kernel[i][j] = values[i][j];
    }
    bmp24_applyFilter(img, kernel, 3);
    for (int i = 0; i < 3; i++) free(kernel[i]);
    free(kernel);
}


void bmp24_sharpen(t_bmp24 *img) {
    float values[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };
    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) kernel[i][j] = values[i][j];
    }
    bmp24_applyFilter(img, kernel, 3);
    for (int i = 0; i < 3; i++) free(kernel[i]);
    free(kernel);
}


//  ==================================================== Fonction 3eme partie =============================

void bmp24_computeHistograms(t_bmp24 *img, unsigned int *histR, unsigned int *histG, unsigned int *histB) {
    if (img == NULL || histR == NULL || histG == NULL || histB == NULL) {
        printf("Erreur : paramètres invalides pour le calcul d'histogrammes 24 bits.\n");
        return;
    }

    // Initialiser les histogrammes à zéro
    for (int i = 0; i < 256; i++) {
        histR[i] = 0;
        histG[i] = 0;
        histB[i] = 0;
    }

    // Parcourir chaque pixel et incrémenter les histogrammes
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            histR[img->data[y][x].red]++;
            histG[img->data[y][x].green]++;
            histB[img->data[y][x].blue]++;
        }
    }
}

unsigned int * bmp24_computeCDF(unsigned int * hist) {
    if (hist == NULL) {
        printf("Erreur : histogramme invalide pour le calcul de la CDF.\n");
        return NULL;
    }

    unsigned int *cdf = calloc(256, sizeof(unsigned int));
    if (cdf == NULL) {
        printf("Erreur : échec d'allocation mémoire pour la CDF.\n");
        return NULL;
    }

    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    return cdf;
}

void bmp24_equalize(t_bmp24 * img) {
    if (img == NULL) {
        printf("Erreur : image invalide pour l'egalisation.\n");
        return;
    }

    // Allouer et calculer les histogrammes
    unsigned int *histR = calloc(256, sizeof(unsigned int));
    unsigned int *histG = calloc(256, sizeof(unsigned int));
    unsigned int *histB = calloc(256, sizeof(unsigned int));
    if (histR == NULL || histG == NULL || histB == NULL) {
        printf("Erreur : échec d'allocation mémoire pour les histogrammes.\n");
        free(histR); free(histG); free(histB);
        return;
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            histR[img->data[y][x].red]++;
            histG[img->data[y][x].green]++;
            histB[img->data[y][x].blue]++;
        }
    }

    // Calculer les CDF
    unsigned int *cdfR = calloc(256, sizeof(unsigned int));
    unsigned int *cdfG = calloc(256, sizeof(unsigned int));
    unsigned int *cdfB = calloc(256, sizeof(unsigned int));
    if (cdfR == NULL || cdfG == NULL || cdfB == NULL) {
        printf("Erreur : échec d'allocation mémoir pour les CDF.\n");
        free(histR); free(histG); free(histB);
        free(cdfR); free(cdfG); free(cdfB);
        return;
    }

    cdfR[0] = histR[0];
    cdfG[0] = histG[0];
    cdfB[0] = histB[0];
    for (int i = 1; i < 256; i++) {
        cdfR[i] = cdfR[i - 1] + histR[i];
        cdfG[i] = cdfG[i - 1] + histG[i];
        cdfB[i] = cdfB[i - 1] + histB[i];
    }

    // Trouver CDF min pour chaque canal
    unsigned int cdf_minR = 0, cdf_minG = 0, cdf_minB = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf_minR == 0 && cdfR[i] != 0) cdf_minR = cdfR[i];
        if (cdf_minG == 0 && cdfG[i] != 0) cdf_minG = cdfG[i];
        if (cdf_minB == 0 && cdfB[i] != 0) cdf_minB = cdfB[i];
    }

    unsigned int totalPixels = img->width * img->height;

    // Calculer les LUT (tableaux de correspondance égalisée)
    unsigned char lutR[256], lutG[256], lutB[256];
    for (int i = 0; i < 256; i++) {
        lutR[i] = (unsigned char)(((float)(cdfR[i] - cdf_minR) / (totalPixels - cdf_minR)) * 255.0 + 0.5);
        lutG[i] = (unsigned char)(((float)(cdfG[i] - cdf_minG) / (totalPixels - cdf_minG)) * 255.0 + 0.5);
        lutB[i] = (unsigned char)(((float)(cdfB[i] - cdf_minB) / (totalPixels - cdf_minB)) * 255.0 + 0.5);
    }

    // Appliquer la transformation
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = lutR[img->data[y][x].red];
            img->data[y][x].green = lutG[img->data[y][x].green];
            img->data[y][x].blue = lutB[img->data[y][x].blue];
        }
    }

    // Libérer la mémoire
    free(histR); free(histG); free(histB);
    free(cdfR); free(cdfG); free(cdfB);
}