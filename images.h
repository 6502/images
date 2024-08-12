#if !defined(IMAGES_H_INCLUDED)
#define IMAGES_H_INCLUDED

#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdexcept>

struct ImageError : std::runtime_error {
    ImageError(const char *what) : runtime_error(what) {}
};

template<typename T>
struct Image {
    int w, h;
    std::vector<T> data;

    Image(int w, int h) : w(w), h(h), data(w*h) {}

    T operator()(int x, int y) const {
        return x>=0 && y>=0 && x<w && y<h ? data[y*w+x] : T();
    }

    void operator()(int x, int y, T v) {
        if (x>=0 && y>=0 && x<w && y<h) data[y*w+x] = v;
    }

    T operator[](int index) const { return data[index]; }
    T& operator[](int index) { return data[index]; }

    template<typename U>
    Image(const Image<U>& other)
        : w(other.w), h(other.h),
          data(other.data.begin(), other.data.end())
    { }
};

template<typename T>
Image<T> loadImage(const std::string& name);

template<>
Image<unsigned char> loadImage<unsigned char>(const std::string& fname) {
    struct F {
        FILE *f;
        bool convert;
        F(const std::string& fname) {
            if (fname.size() > 3 && fname.substr(fname.size()-4) == ".pgm") {
                convert = false;
                f = fopen(fname.c_str(), "rb");
            } else {
                convert = true;
                f = popen(("ffmpeg -y -loglevel 0 -i " + fname + " -f image2pipe -vcodec pgm - ").c_str(), "r");
            }
            if (!f) {
                perror("loadImage<unsigned char>");
                throw ImageError("Error opening image file");
            }
        }
        ~F() {
            if (f) (convert ? pclose : fclose)(f);
        }
        operator FILE *() { return f; };
    } f(fname);

    int w, h, maxv;
    if (fgetc(f) != 'P' || fgetc(f) != '5') throw ImageError("Not a PGM file");
    int c;
    while ((c = fgetc(f)) != EOF && isspace(c));
    if (c == EOF) throw ImageError("Not a PGM file");
    while(c == '#') {
        while ((c = fgetc(f)) != EOF && c != '\n') ;
        c = fgetc(f);
    }
    if (c != EOF) ungetc(c, f);
    if (fscanf(f, "%i %i %i%*c", &w, &h, &maxv)!=3 || w<0 || h<0 || maxv>255) {
        throw ImageError("Not a 8bpp-PGM file");
    }
    Image<unsigned char> img(w, h);
    if (fread(&img[0], 1, w*h, f) != (unsigned)w*h) throw ImageError("I/O error loading PGM file");
    if (maxv < 255) for (auto& v : img.data) v = v*255/maxv;
    return img;
}

template<>
Image<unsigned> loadImage<unsigned>(const std::string& fname) {
    struct F {
        FILE *f;
        bool convert;
        F(const std::string& fname) {
            if (fname.size() > 4 && fname.substr(fname.size()-4) == ".pam") {
                convert = false;
                f = fopen(fname.c_str(), "rb");
            } else {
                convert = true;
                f = popen(("ffmpeg -y -loglevel 0 -i " + fname + " -f image2pipe -vcodec pam - ").c_str(), "r");
            }
            if (!f) {
                perror("loadImage<unsigned>");
                throw ImageError("Error opening image file");
            }
        }
        ~F() {
            if (f) (convert ? pclose : fclose)(f);
        }
        operator FILE *() { return f; };
    } f(fname);
    int w=-1, h=-1, maxv=-1, depth=-1;
    if (fgetc(f) != 'P' || fgetc(f) != '7' || fgetc(f) != '\n') throw ImageError("Not a PAM file");
    char buf[256];
    while (fgets(buf, 256, f)) {
        if (strncmp(buf, "WIDTH ", 6) == 0) w = atoi(buf+6);
        if (strncmp(buf, "HEIGHT ", 7) == 0) h = atoi(buf+7);
        if (strncmp(buf, "DEPTH ", 6) == 0) depth = atoi(buf+6);
        if (strncmp(buf, "MAXVAL ", 7) == 0) maxv = atoi(buf+7);
        if (strncmp(buf, "ENDHDR", 6) == 0) break;
    }
    if ((depth != 1 && depth != 3 && depth != 4) || (maxv > 255)) throw ImageError("Not an yu8/rgb24/argb32 image");
    Image<unsigned int> img(w, h);
    std::vector<unsigned char> row(w*depth);
    for (int y=0; y<h; y++) {
        if (fread(&row[0], w*depth, 1, f) != 1) throw ImageError("I/O error loading PAM file");
        if (depth == 1) {
            for (int x=0; x<w; x++) img[y*w+x] = row[x]*255/maxv * 0x010101 + 0xFF000000;
        } else {
            for (int x=0; x<w; x++) {
                unsigned r = row[x*depth]*255/maxv, g = row[x*depth+1]*255/maxv, b = row[x*depth+2]*255/maxv,
                         a = (depth == 3) ? 255 : row[x*depth+3]*255/maxv;
                img[y*w+x] = (a<<24) + (r<<16) + (g<<8) + b;
            }
        }
    }
    return img;
}

template<typename T>
void saveImage(const Image<T>& img, const std::string& filename);

template<>
void saveImage<unsigned char>(const Image<unsigned char>& img, const std::string& fname) {
    struct F {
        FILE *f;
        bool convert;
        F(const std::string& fname) {
            if (fname.size() && fname.substr(fname.size()-4) == ".pgm") {
                convert = false;
                f = fopen(fname.c_str(), "wb");
            } else {
                convert = true;
                f = popen(("ffmpeg -y -loglevel 0 -i - -f image2pipe " + fname).c_str(), "w");
            }
            if (!f) {
                perror("saveImage<unsigned char>");
                throw ImageError("Error saving image");
            }
        }
        ~F() {
            if (f) (convert ? pclose : fclose)(f);
        }
        operator FILE *() { return f; };
    } f(fname);
    fprintf(f, "P5\n%i %i 255\n", img.w, img.h);
    fwrite(&img.data[0], 1, img.w*img.h, f);
}

template<>
void saveImage<unsigned>(const Image<unsigned>& img, const std::string& fname) {
    struct F {
        FILE *f;
        bool convert;
        F(const std::string& fname) {
            if (fname.size()>4 && fname.substr(fname.size()-4) == ".ppm") {
                convert = false;
                f = fopen(fname.c_str(), "wb");
            } else {
                convert = true;
                f = popen(("ffmpeg -y -loglevel 0 -i - -f image2pipe " + fname).c_str(), "w");
            }
            if (!f) {
                perror("saveImage<unsigned>");
                throw ImageError("Error saving image");
            }
        }
        ~F() {
            if (f) (convert ? pclose : fclose)(f);
        }
        operator FILE *() { return f; };
    } f(fname);

    fprintf(f, "P6\n%i %i 255\n", img.w, img.h);
    std::vector<unsigned char> row(img.w*3);
    for (int y=0; y<img.h; y++) {
        for (int x=0; x<img.w; x++) {
            row[x*3] = (img[y*img.w+x]>>16) & 255;
            row[x*3+1] = (img[y*img.w+x]>>8) & 255;
            row[x*3+2] = img[y*img.w+x] & 255;
        }
        fwrite(&row[0], 1, img.w*3, f);
    }
}

#endif
