
#ifndef INCLUDE_IMAGEPACKER_H
#define INCLUDE_IMAGEPACKER_H

#include <vector>
#include <memory>

struct Image {
    std::shared_ptr<unsigned char> data;
    int w;
    int h;
    int ncomps;

    Image(): data(nullptr), w(0), h(0), ncomps(0) {}
    Image(const char *filename): data(nullptr), w(0), h(0), ncomps(0) {
        Read(filename);
    }
    Image(int _w, int _h, int _ncomps);

    ~Image();

    void Read(const char *filename);

    void Blit(const Image &src, int x, int y, int srcx, int srcy, int srcw, int srch);
    void Blit(const Image &src, int x, int y) { Blit(src, x, y, 0, 0, src.w, src.h); }
};

struct Options {
    int minw;
    int minh;
    int maxw;
    int maxh;

    std::vector<std::string> infiles;
    std::string outfile;

    Options() {
        minw = 256;
        minh = 256;
        maxw = 4096;
        maxh = 4096;
    }

    void AddInfile(const char *filename);
};

void ImagePack(const Options &options);


#endif //INCLUDE_IMAGEPACKER_H
