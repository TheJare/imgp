// ImagePacker.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include "ImagePacker.h"

#include "stb_image_write.h"
#include "stb_image.c"
#include <stdarg.h>
#include <vector>

// ------------------
// Image
// ------------------
Image::Image(int _w, int _h, int _ncomps): data(nullptr), w(0), h(0), ncomps(0) {
    int size = _w*_h*_ncomps;
    if (size > 0) {
        data = std::shared_ptr<unsigned char>((unsigned char *)calloc(size, 1), free);
        w = _w;
        h = _h;
        ncomps = _ncomps;
    }
}
void Image::Read(const char *filename) {
    data = std::shared_ptr<unsigned char>(stbi_load(filename, &w, &h, &ncomps, 0), free);
}

void Image::Blit(const Image &src, int x, int y, int srcx, int srcy, int srcw, int srch) {

}

// ------------------
// Options
// ------------------
void Options::AddInfile(const char *filename) {
    // MSVC: rely on linking with setargv.obj for globbing
    // http://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
    //printf("input file: %s\n", filename);
    infiles.push_back(filename);
}

// ------------------
// Packing
// ------------------
void ImagePack(const Options &options)
{
    std::vector<Image*> images;

    for (auto i = std::begin(options.infiles); i != std::end(options.infiles); ++i) {
        Image *img = new Image(i->c_str());
        printf("Input file: %s (%d x %d, %d channels)\n", i->c_str(), img->w, img->h, img->ncomps);

        images.push_back(img);
    }
}
