# images
Minimal image files support (PGM/PPM native, other formats invoking imagemagick convert)

    template<typename T>
    struct Image {
        int w, h;               // Width and height
        std::vector<T> data;    // Pixel values

        // Creates a empty (0) image
        Image(int w, int h) : w(w), h(h), data(w*h) {}

        // Safe read access, returns T(0) for out-of-bounds
        T operator()(int x, int y) const {
            return x>=0 && y>=0 && x<w && y<h ? data[y*w+x] : 0;
        }

        // Safe write address, ignores out of bounds writes
        void operator()(int x, int y, T v) {
            if (x>=0 && y>=0 && x<w && y<h) data[y*w+x] = v;
        }

        // Pixel direct access
        T operator[](int index) const { return data[index]; }
        T& operator[](int index) { return data[index]; }

        // Conversion from image with a different pixel type
        template<typename U>
        Image(const Image<U>& other)
            : w(other.w), h(other.h),
              data(other.data.begin(), other.data.end())
        { }
    };

Loaders/savers are templates on the pixel type

    // Loaders
    template<typename T>
    Image<T> loadImage(const std::string& name);

    // Savers
    template<typename T>
    void saveImage(const Image<T>& img, const std::string& filename);

Specializations already implemented for 8-bpp and 32-bpp (8 ignored)

    // Load an 8-bpp grayscale image from a file
    template<>
    Image<unsigned char> loadImage<unsigned char>(const std::string& fname);

    // Loads a 32-bpp color image (blue + 256*green + 65536*red, 4th byte 0) from a file;
    template<>
    Image<unsigned> loadImage<unsigned>(const std::string& fname);

    // Saves a 8-bpp grayscale image to a file
    template<>
    void saveImage<unsigned char>(const Image<unsigned char>& img, const std::string& fname);

    // Saves a 32-bpp color image to a file
    template<>
    void saveImage<unsigned>(const Image<unsigned>& img, const std::string& fname);

Most image file formats supported, PGM and PPM natively and ImageMagick `convert` utility invoked
in case the file format is a different one.
