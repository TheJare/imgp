/*
 * Copyright (c) 2013, Javier Arevalo
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef INCLUDE_IMAGE_H
#define INCLUDE_IMAGE_H

#include <string>
#include <memory>

struct Image {
    std::shared_ptr<unsigned char> data;
    int w;
    int h;
    int ncomps;
    std::string filename;
    int fillx, filly, fillw, fillh;

    Image(): data(nullptr), w(0), h(0), ncomps(0) { ResetFillArea(); }
    Image(const char *_filename): data(nullptr), w(0), h(0), ncomps(0), fillw(0), fillh(0) {
        Read(_filename);
    }
    Image(int _w, int _h, int _ncomps);

    void Read(const char *_filename);
    void Save(const char *_filename);

    void Rotate();

    void ResetFillArea() {
         fillx = 0;
         filly = 0;
         fillw = w;
         fillh = h;
    }
    void FindFillArea();

    void Blit(const Image &src, int x, int y, int srcx, int srcy, int srcw, int srch);
    void Blit(const Image &src, int x, int y) { Blit(src, x, y, 0, 0, src.w, src.h); }

    unsigned char *at(int x, int y) { return data.get() + (y*w + x)*ncomps; }
    const unsigned char *at(int x, int y) const { return data.get() + (y*w + x)*ncomps; }
};

#endif //INCLUDE_IMAGE_H
