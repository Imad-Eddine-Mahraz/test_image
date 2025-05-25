#include "bmp8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================== Fonction de la partie 1 ===============================================
t_bmp8 * bmp8_loadImage(const char * filename) {
    FILE * file = NULL;

    if (filename == NULL) {
        return NULL;
    }
    file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur : impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }
    printf("Laoding %s\n",filename);
    t_bmp8 * bmp8 = calloc( sizeof(t_bmp8),1);
    if (bmp8 == NULL) {
        perror("bmp8_loadImage");
        fclose(file);
        return NULL;
    }
    size_t ret = fread(bmp8->header, 1, HEADER_SIZE, file);
    if (ret != HEADER_SIZE) {
        printf("Error reading header from file '%s' expected: %d,got:%zd\n",filename,HEADER_SIZE,ret);
    }
    bmp8->width = *(unsigned int*)&(bmp8->header)[18];
    bmp8->height = *(unsigned int*)&(bmp8->header[22]);
    bmp8->colorDepth = *(unsigned short*)&(bmp8->header[28]);
    bmp8->dataSize = *(unsigned int*)&(bmp8->header[34]);

    if (bmp8->dataSize==0) {
        unsigned int compression = *(unsigned int*)&(bmp8->header[30]);
        if (compression == 0) {
            bmp8->dataSize = bmp8->width * bmp8->height;
        }
        else {
            printf("Fichier compressé et Datasize à 0");
        }
    }
    ret = fread(bmp8->colorTable, 1, COLOR_TABLE_SIZE, file);
    if (ret!=COLOR_TABLE_SIZE){
        printf("Error reading colorTable from file '%s' exected %d, got: %zd\n",filename, COLOR_TABLE_SIZE, ret);
    }
    bmp8->data = malloc(bmp8->dataSize);
    if (bmp8->data == NULL) {
        perror("malloc data");
        free(bmp8);
        fclose(file);
        return NULL;
    }
    ret = fread(bmp8->data, 1, bmp8->dataSize, file);
    if (ret != bmp8->dataSize) {
        printf("Error reading data from file '%s' exected %d, got: %zd\n",filename, COLOR_TABLE_SIZE, ret);
    }

    fclose(file);
    return bmp8;
}



void bmp8_saveImage(const char * filename, t_bmp8 * img) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Erreur lors de l'ouverture du fichier pour l'ecriture\n");
        return;
    }

    // Écrire l'en-tête BMP (54 octets)
    if (fwrite(img->header, sizeof(unsigned char), 54, file) != 54) {
        printf("Erreur lors de l'écriture de l'en-tête\n");
        fclose(file);
        return;
    }

    // Écrire la table de couleurs (1024 octets pour image 8 bits)
    if (fwrite(img->colorTable, sizeof(unsigned char), 1024, file) != 1024) {
        printf("Erreur lors de l'écriture de la table de couleurs\n");
        fclose(file);
        return;
    }

    // Écrire les données de l'image
    if (fwrite(img->data, sizeof(unsigned char), img->dataSize, file) != img->dataSize) {
        printf("Erreur lors de l'écriture des données d'image\n");
        fclose(file);
        return;
    }

    fclose(file);
    printf("Image enregistree avec succes dans le dossier Images'%s'\n", filename);
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
        printf("Erreur :image ou noyau invalide.\n");
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

// ====================================================== Fonction de la partie 3 ======================================


#define GRAY_LEVELS 256


// 1. Calcul de l’histogramme d’une image en niveaux de gris
unsigned int* bmp8_computeHistogram(t_bmp8* img) {
    if (img == NULL || img->data == NULL) {
        printf("Erreur : image invalide pour le calcul d'hisogramme.\n");
        return NULL;
    }
    unsigned int * hist = calloc(256,sizeof(unsigned int));
    if (hist == NULL) {
        printf("Erreur : échec d'allocation de la mémoire pour l'histograme.\n");
        return NULL;
    }
    for (int i = 0 ; i < img->dataSize ; i++) {
        hist[img->data[i]]++;
    }
    return hist;
}

// 2. Calcul du CDF (histogramme cumulé) et normalisation
unsigned int * bmp8_computeCDF(unsigned int * hist) {
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


// 3. Égalisation de l’image
void bmp8_equalize(t_bmp8 * img, unsigned int * hist, unsigned int * hist_eq) {
    if (img == NULL || img->data == NULL || hist == NULL || hist_eq == NULL) {
        printf("Erreur : paramètres invalides pour l'égalisation.\n");
        return;
    }

    unsigned int *cdf = bmp8_computeCDF(hist);
    if (cdf == NULL) {
        return;
    }

    // Chercher la première valeur de la CDF non nulle (CDF min)
    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] != 0) {
            cdf_min = cdf[i];
            break;
        }
    }

    unsigned int totalPixels = img->width * img->height;

    // Calcul de la transformation : histogramme égalisé
    for (int i = 0; i < 256; i++) {
        hist_eq[i] = (unsigned int)( ((float)(cdf[i] - cdf_min) / (totalPixels - cdf_min)) * 255.0 + 0.5 );
    }

    // Appliquer la transformation à l'image
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (unsigned char)hist_eq[img->data[i]];
    }

    free(cdf);
}
