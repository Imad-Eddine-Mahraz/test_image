// a compléter à l'avenir
#include "t_bmp24.h"
#include <stdio.h>
#include <stdlib.h>




t_pixel ** bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = malloc(height * sizeof(t_pixel *));
    if (!pixels) {
        printf("Erreur : échec de l'allocation de la mémoire pour les lignes de pixels.\n");
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        pixels[i] = malloc(width * sizeof(t_pixel));
        if (!pixels[i]) {
            printf("Erreur : échec de l'allocation de la mémoire pour la ligne %d.\n", i);
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
    if (!img) {
        printf("Erreur : échec de l'allocation de l'image.\n");
        return NULL;
    }
	img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;
	img->data = bmp24_allocateDataPixels(width, height);
    if (!img->data) {
        free(img);
        return NULL;
    }
return img;
}


void bmp24_free(t_bmp24 *img) {
    if (img == NULL) return;
	bmp24_freeDataPixels(img->data, img->height);
    free(img);
}

t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }

    t_bmp24 *img = malloc(sizeof(t_bmp24));
    if (!img) {
        printf("Erreur : allocation mémoire échouée pour l'image.\n");
        fclose(file);
        return NULL;
    }

    file_rawRead(BITMAP_MAGIC, &img->header, sizeof(t_bmp_header), 1, file);
    file_rawRead(HEADER_SIZE, &img->header_info, sizeof(t_bmp_info), 1, file);


    if (img->header.type != BMP_TYPE || img->header_info.bits != DEFAULT_DEPTH) {
        printf("Erreur : format BMP invalide ou profondeur ≠ 24 bits.\n");
        fclose(file);
        free(img);
        return NULL;
    }

    img->width = img->header_info.width;
    img->height = img->header_info.height;
    img->colorDepth = img->header_info.bits;

    img->data = bmp24_allocateDataPixels(img->width, img->height);
    if (!img->data) {
        fclose(file);
        free(img);
        return NULL;
    }

    // Lire les données pixel par pixel
    int padding = (4 - (img->width * 3) % 4) % 4;
    uint8_t rgb[3];
    fseek(file, img->header.offset, SEEK_SET);

    for (int i = img->height - 1; i >= 0; i--) {
        for (int j = 0; j < img->width; j++) {
            fread(rgb, sizeof(uint8_t), 3, file);
            img->data[i][j].blue = rgb[0];
            img->data[i][j].green = rgb[1];
            img->data[i][j].red = rgb[2];
        }
        fseek(file, padding, SEEK_CUR);
    }
	fclose(file);
    return img;
}



void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erreur :impossible de créer le fichier %s\n", filename);
        return;
    }

    file_rawWrite(BITMAP_MAGIC, &img->header, sizeof(t_bmp_header), 1, file);
    file_rawWrite(HEADER_SIZE, &img->header_info, sizeof(t_bmp_info), 1, file);

    int padding = (4 - (img->width * 3) % 4) % 4;
    uint8_t pad[3] = {0, 0, 0};
    uint8_t rgb[3];

    fseek(file, img->header.offset, SEEK_SET);

    for (int i = img->height - 1; i >= 0; i--) {
        for (int j = 0; j < img->width; j++) {
            rgb[0] = img->data[i][j].blue;
            rgb[1] = img->data[i][j].green;
            rgb[2] = img->data[i][j].red;
            fwrite(rgb, sizeof(uint8_t), 3, file);
        }
        fwrite(pad, sizeof(uint8_t), padding, file);
    }

    fclose(file);
}