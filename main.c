#include <stdio.h>
#include "bmp8.h"

int main() {

    t_bmp8 *img = bmp8_loadImage("C:\\Users\\jules\\CLionProjects\\image_processing_c\\barbara_gray.bmp");
    if (img != NULL) {
        bmp8_printInfo(img);
        bmp8_negative(img);
        int pixel_index = 0;
        unsigned char original_pixel = img->data[pixel_index];
        unsigned char inverted_pixel = 255 - original_pixel;
        printf("Pixel %d avant : %d, apres : %d\n", pixel_index, original_pixel, inverted_pixel);
        bmp8_free(img);
    } else {
        printf("Échec du chargement de l'image.\n");
    }

    return 0;
}
