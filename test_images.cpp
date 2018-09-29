#include "images.h"

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        printf("Syntax: test_images <source> <dest>\n");
        return 1;
    }
    saveImage(loadImage<unsigned>(argv[1]), argv[2]);
    return 0;
}
