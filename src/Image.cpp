/*
 * Copyright (c) 2013, Javier Arevalo
 * https://github.com/TheJare/imgp.git
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

#include "Image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image.c"

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
        ResetFillArea();
    }
}
void Image::Read(const char *_filename) {
    data = std::shared_ptr<unsigned char>(stbi_load(_filename, &w, &h, &ncomps, 0), free);
    ResetFillArea();
    filename = _filename;
}

void Image::Save(const char *_filename) {
    stbi_write_png(_filename, w, h, ncomps, data.get(), 0);
    filename = _filename;
}

// Rotate clocwise 90 degrees
void Image::Rotate() {
    unsigned char *newdata = (unsigned char *)malloc(w*h*ncomps);
    unsigned char *pd = newdata;
    for (int i = 0; i < w; ++i) {
        const unsigned char *ps = at(i, h-1);
        for (int j = 0; j < h; ++j) {
            *pd++ = *ps++;
            *pd++ = *ps++;
            *pd++ = *ps++;
            if (ncomps == 4) *pd++ = *ps++;
            ps -= (w+1)*ncomps;
        }
    }
    data = std::shared_ptr<unsigned char>(newdata, free);
    int noy = filly;
    filly = fillx;
    fillx = h - noy - fillh;
    std::swap(w, h);
    std::swap(fillw, fillh);
}

void Image::FindFillArea() {
    if (ncomps != 4) {
        return;
    }
    // top
    const unsigned char *ps = data.get()+3;
    filly = -1;
    for (int i = 0; i < h && filly < 0; ++i) {
        for (int j = 0; j < w; ++j, ps += 4) {
            if (*ps != 0) {
                filly = i;
                break;
            }
        }
    }
    if (filly < 0) {
        filly = h;
    }
    // bottom
    ps = at(0, h)-1;
    fillh = -1;
    for (int i = h; i > filly && fillh < 0; --i) {
        for (int j = 0; j < w; ++j, ps -= 4) {
            if (*ps != 0) {
                fillh = i - filly;
                break;
            }
        }
    }
    if (fillh < 0) {
        filly = 0;
    }
    // Left
    fillx = -1;
    for (int j = 0; j < w && fillx < 0; ++j) {
        ps = at(j, filly)+3;
        for (int i = 0; i < fillh; ++i, ps += w*ncomps) {
            if (*ps != 0) {
                fillx = j;
                break;
            }
        }
    }
    if (fillx < 0) {
        fillx = w;
    }
    // Right
    fillw = -1;
    for (int j = w; j > fillx && fillw < 0; --j) {
        ps = at(j, filly)-1;
        for (int i = 0; i < fillh; ++i, ps += w*ncomps) {
            if (*ps != 0) {
                fillw = j - fillx;
                break;
            }
        }
    }
    if (fillw < 0) {
        fillw = 0;
    }
}

void Image::Blit(const Image &src, int x, int y, int srcx, int srcy, int srcw, int srch) {
    // Clip source
    if (srcx < 0) { x += -srcx; srcw -= -srcx; srcx = 0; }
    if (srcy < 0) { y += -srcy; srch -= -srcy; srcy = 0; }
    if (srcx + srcw > src.w) srcw = src.w - srcx;
    if (srcy + srch > src.h) srch = src.h - srcy;
    // Clip to destination.
    if (x < 0) { srcx += -x; srcw -= -x; x = 0; }
    if (y < 0) { srcy += -y; srch -= -y; y = 0; }
    if (x + srcw > w) srcw = w - x;
    if (y + srch > h) srch = h - y;
    if (srcw <= 0 || srch <= 0) {
        return;
    }

    unsigned char *pd = at(x, y);
    const unsigned char *ps = src.at(srcx, srcy);
    for (int i = 0; i < srch; ++i) {
        // Blit the line, considering different bitdepth combinations
        if (ncomps == src.ncomps) {
            memcpy(pd, ps, srcw*ncomps);
            pd += w*ncomps;
            ps += src.w*src.ncomps;
        } else {
            if (ncomps == 3 && src.ncomps == 4) {
                for (int j = 0; j < srcw; ++j, ++ps) {
                    *pd++ = *ps++;
                    *pd++ = *ps++;
                    *pd++ = *ps++;
                }
            } else if (ncomps == 4 && src.ncomps == 3) {
                for (int j = 0; j < srcw; ++j) {
                    *pd++ = *ps++;
                    *pd++ = *ps++;
                    *pd++ = *ps++;
                    *pd++ = 255;
                }
            }
            // Advance pointers
            pd += (w - srcw)*ncomps;
            ps += (src.w - srcw)*src.ncomps;
        }
    }
}
