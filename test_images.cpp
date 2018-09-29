#include "images.h"
#include "argv.h"

int main(int argc, const char *argv[]) {
    PARM(std::string, src, "Source filename", "source.jpg");
    PARM(std::string, dest, "Destination filename", "dest.bmp");

    parse_argv("testimages", argc, argv);

    saveImage(loadImage<unsigned>(src), dest);

    return 0;
}
