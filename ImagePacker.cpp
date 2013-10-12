// ImagePacker.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include "ImagePacker.h"

#include "stb_image_write.h"
#include "stb_image.c"
#include <stdarg.h>
#include <vector>

Image::Image(int _w, int _h, int _ncomps): data(nullptr), w(0), h(0), ncomps(0) {
    int size = _w*_h*_ncomps;
    if (size > 0) {
        data = (unsigned char *)calloc(size, 1);
        w = _w;
        h = _h;
        ncomps = _ncomps;
    }
}
Image::~Image() {
    free(data);
}
void Image::Read(const char *filename) {
    data = stbi_load(filename, &w, &h, &ncomps, 0);
}

void Options::AddInfile(const char *filename) {
    // MSVC: rely on linking with setargv.obj for globbing
    // http://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
    printf("input file: %s\n", filename);
    infiles.push_back(filename);
}


void ImagePack(const Options &options)
{
    std::vector<Image*> images;

    for (auto i = std::begin(options.infiles); i != std::end(options.infiles); ++i) {
        Image *img = new Image(i->c_str());
    }
}
