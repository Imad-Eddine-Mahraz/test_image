#include "bmp8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


t_bmp8 * bmp8_loadImage(const char * filename){
FILE *file = fopen(filename, "rb");
if (file == NULL) {
    printf("Erreur : impossible d'ouvrir le fichier %s\n", filename);
    return NULL;
}

t_bmp8 *img = (t_bmp8 *)malloc(sizeof(t_bmp8));
if (img == NULL) {
   printf("Erreur : échec de l'allocation mémoire pour l'image\n");
   fclose(file);
   return NULL;
}

if (fread(img->header, sizeof(unsigned char), 54, file) != 54) {
   printf("Erreur : échec de lecture de l'en-tête BMP\n");
   fclose(file);
   free(img);
   return NULL;
}
    if (img->header[0] != 'B' || img->header[1] != 'M') {
        printf("Erreur : le fichier n'est pas un fichier BMP valide\n");
        fclose(file);
        free(img);
        return NULL;
    }
    img->width       = *(unsigned int *)&img->header[18];
    img->height      = *(unsigned int *)&img->header[22];
    img->colorDepth  = *(unsigned short *)&img->header[28];
    img->dataSize    = *(unsigned int *)&img->header[34];
    unsigned int offset = *(unsigned int *)&img->header[10];
    // Vérifier que c’est une image 8 bits
    if (img->colorDepth != 8) {
        printf("Erreur : l'image doit être en niveaux de gris (8 bits par pixel)\n");
        fclose(file);
        free(img);
        return NULL;
    }
    // Lire la table de couleurs (1024 octets pour 8 bits)
    if (fread(img->colorTable, sizeof(unsigned char), 1024, file) != 1024) {
        printf("Erreur : échec de lecture de la table de couleurs\n");
        fclose(file);
        free(img);
        return NULL;
    }
    img->data = (unsigned char *)malloc(img->dataSize * sizeof(unsigned char));
    if (img->data == NULL) {
        printf("Erreur : échec de l'allocation mémoire pour les données de l'image\n");
        fclose(file);
        free(img);
        return NULL;
    }
    fseek(file, offset, SEEK_SET);

    if (fread(img->data, sizeof(unsigned char), img->dataSize, file) != img->dataSize) {
        printf("Erreur : échec de lecture des données de l'image\n");
        fclose(file);
        free(img->data);
        free(img);
        return NULL;
    }
    fclose(file);
    return img;
}

void bmp8_free(t_bmp8 *img) {
    if (img != NULL) {
        if (img->data != NULL) {
            free(img->data);
        }
        free(img);
    }
}

void bmp8_printInfo(t_bmp8 *img) {
    if (img == NULL) {
        printf("L'image est vide ou invalide.\n");
        return;
    }
    printf("Image Info:\n");
    printf("Width: %u\n", img->width);
    printf("Height: %u\n", img->height);
    printf("Color Depth: %u\n", img->colorDepth);
    printf("Data Size: %u\n", img->dataSize);
}

void bmp8_negative(t_bmp8 *img) {
    // Vérification si l'image est valide
    if (img == NULL || img->data == NULL) {
        printf("Erreur : image invalide pour inversion\n");
        return;
    }
    unsigned int i;
    for (i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

void bmp8_brightness(t_bmp8 *img, int value) {
    // Vérification si l'image est valide
    if (img == NULL || img->data == NULL) {
        printf("Erreur : image invalide.\n");
        return;
    }
    for (int i = 0; i < img->dataSize; i++) {
        int new_pixel_value = img->data[i] + value;
        // Ne pas dépasser les limites
        if (new_pixel_value > 255) {
            img->data[i] = 255;
        } else if (new_pixel_value < 0) {
            img->data[i] = 0;
        } else {
            img->data[i] = (unsigned char)new_pixel_value;
        }
    }
}

void bmp8_threshold(t_bmp8 *img, int threshold) {
    // Vérification si l'image est valide
    if (img == NULL || img->data == NULL) {
        printf("Erreur : image invalide.\n");
        return;
    }

    for (int i = 0; i < img->dataSize; i++) {
        if (img->data[i] >= threshold) {
            img->data[i] = 255;
        } else {
            img->data[i] = 0;
        }
    }
}

void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize) {
    if (img == NULL || img->data == NULL || kernel == NULL) {
        printf("Erreur : image ou noyau invalide.\n");
        return;
    }
    int width = img->width;
    int height = img->height;
    int n = kernelSize / 2;  // n est la moitié du noyau


    unsigned char *originalData = (unsigned char *)malloc(img->dataSize * sizeof(unsigned char));
    if (originalData == NULL) {
        printf("Erreur : échec de l'allocation mémoire pour la copie de l'image.\n");
        return;
    }
    // Copier les données à la main (au lieu de memcpy)
    for (unsigned int i = 0; i < img->dataSize; i++) {
        originalData[i] = img->data[i];
    }

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            float sum = 0.0f;

            // Appliquer le noyau sur les voisins
            for (int i = -n; i <= n; i++) {
                for (int j = -n; j <= n; j++) {
                    int neighborX = x + j;
                    int neighborY = y + i;
                    unsigned char pixelValue = originalData[neighborY * width + neighborX];
                    sum += pixelValue * kernel[i + n][j + n];
                }
            }

            if (sum < 0) sum = 0;
            if (sum > 255) sum = 255;

            img->data[y * width + x] = (unsigned char)sum;
        }
    }

    free(originalData);
}
