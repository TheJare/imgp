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

#define _CRT_SECURE_NO_WARNINGS

#include "ImagePacker.h"

#include <vector>
#include <algorithm>

#include "Image.h"
#include "GuillotineBinPack.h"

// Round value up to the next power of 2
// From the always fun http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
int NextPower2(int v) {
    if (v <= 0)
        return 1;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

// Another of those functions you wish was standard...
std::string ReplaceString(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

#if defined(_WIN32) || defined(WIN32)
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

std::string basename(const std::string &s) {
    size_t posdot = s.find_last_of('.');
    size_t possep = s.find_last_of(PATH_SEPARATOR);
    if (posdot == std::string::npos || (possep != std::string::npos && posdot < possep)) {
        return s;
    }
    return s.substr(0, posdot);
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
    // Load all images
    std::vector<Image*> images;
    for (const auto &i: options.infiles) {
        Image *img = new Image(i.c_str());
        printf("Input file: %s (%d x %d, %d channels)\n", i.c_str(), img->w, img->h, img->ncomps);
        img->FindFillArea();
        printf("  Fill area is %d,%d x %d,%d\n", img->fillx, img->filly, img->fillw, img->fillh);
        images.push_back(img);
    }

    // Build array of rects corresponding to loaded images
    std::vector<rbp::RectSize> srcRects;
    for (auto i : images) {
        rbp::RectSize r;
        r.width = i->fillw + options.padx;
        r.height = i->fillh + options.pady;
        r.image = i;
        srcRects.push_back(r);
    }

    // Iterate from min size until all images fit
    // Sanitize sizes first
    int w = NextPower2(options.minw);
    int h = NextPower2(options.minh);
    if (options.forceSquare) {
        w = h = std::max(w, h);
    }
    rbp::GuillotineBinPack binPacker;
    while (true) {
        // Add margin to destination because all source images are given a margin,
        // but those ending up on the right or bottom don't need it
        binPacker.Init(w+options.padx, h+options.pady);
        bool allFit = binPacker.Insert(srcRects, true, options.allowFlipping, rbp::GuillotineBinPack::RectBestShortSideFit, rbp::GuillotineBinPack::SplitShorterLeftoverAxis);
        if (allFit) {
            break;
        }
        // Impossible to fit them all, grow the rectangle
        if (options.forceSquare) {
            w *= 2;
            h = w;
        } else if (w > h || (w*2 > options.maxw && h*2 <= options.maxh)) {
            h *= 2;
        } else {
            w *= 2;
        }
    }
    // Recompute actually occupied area
    w = NextPower2(binPacker.GetOccupiedWidth()-options.padx);
    h = NextPower2(binPacker.GetOccupiedHeight()-options.pady);
    if (w > options.maxw || h > options.maxh) {
        fprintf(stderr, "Error: impossible to fit all images. Best case is %d x %d\n", w, h);
        exit(1);
    }

    // Build resulting atlas image
    // Save map file with correct format
    Image dest(w, h, 4);
    std::string mapExtension;
    switch (options.format) {
        case Options::FORMAT_TXT: mapExtension = ".txt"; break;
        case Options::FORMAT_JSON: mapExtension = ".json"; break;
        case Options::FORMAT_PLIST: mapExtension = ".plist"; break;
    }
    std::string outmapfile = basename(options.outfile) + mapExtension;
    FILE *mapf = fopen(outmapfile.c_str(), "wt");
    switch (options.format) {
        case Options::FORMAT_TXT:
            fprintf(mapf, "atlas: \"%s\" %d,%d total %d\n", options.outfile.c_str(), w, h, (int)binPacker.GetUsedRectangles().size());
            break;
        case Options::FORMAT_JSON:
            fputs("{ \"frames\": {\n", mapf);
            break;
        case Options::FORMAT_PLIST:
            fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
                "<plist version=\"1.0\"><dict><key>frames</key><dict>\n", mapf);
            break;
    }
    bool firstImage = true;
    for (const auto &r: binPacker.GetUsedRectangles()) {
        if (r.flipped) {
            r.image->Rotate();
        }
        std::string saneFilename = ReplaceString(r.image->filename, "\\", "/");
        switch (options.format) {
            case Options::FORMAT_TXT:
                fprintf(mapf, "%s: %d,%d x %d,%d offset %d,%d orgsize %d,%d %s\n",
                    saneFilename.c_str(),
                    r.x, r.y, r.image->fillw, r.image->fillh,
                    r.image->fillx, r.image->filly, r.image->w, r.image->h,
                    r.flipped? "rotated" : "original");
                break;
            case Options::FORMAT_JSON:
                fprintf(mapf,
                    "%s\"%s\": { \"frame\": {\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d},\"rotated\":%s,\"trimmed\":%s,\"spriteSourceSize\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d},\"sourceSize\":{\"w\":%d,\"h\":%d}}\n",
                    firstImage? " " : ",",
                    saneFilename.c_str(),
                    r.x, r.y, r.image->fillw, r.image->fillh,
                    r.flipped? "true" : "false",
                    (r.image->w!=r.image->fillw || r.image->h!=r.image->fillh)? "true" : "false",
                    r.image->fillx, r.image->filly, r.image->fillw, r.image->fillh,
                    r.image->w, r.image->h);
                firstImage = false;
                break;
            case Options::FORMAT_PLIST:
               fprintf(mapf,
                    "<key>%s</key><dict><key>frame</key><string>{{%d,%d},{%d,%d}}</string><key>offset</key><string>{%d,%d}</string><key>rotated</key><%s/><key>sourceColorRect</key><string>{{%d,%d},{%d,%d}}</string><key>sourceSize</key><string>{%d,%d}</string></dict>\n",
                    saneFilename.c_str(),
                    r.x, r.y, r.image->fillw, r.image->fillh,
                    r.image->w/2-r.image->fillx, r.image->h/2-r.image->filly,
                    r.flipped? "true" : "false",
                    r.image->fillx, r.image->filly, r.image->fillw, r.image->fillh,
                    r.image->w, r.image->h);
                break;
        }
        dest.Blit(*r.image, r.x, r.y, r.image->fillx, r.image->filly, r.image->fillw, r.image->fillh);
    }
    switch (options.format) {
        case Options::FORMAT_TXT:
            break;
        case Options::FORMAT_JSON:
            fputs("}}\n", mapf);
            break;
        case Options::FORMAT_PLIST:
            fputs("</dict></dict></plist>\n", mapf);
            break;
    }
    fclose(mapf);

    std::string outImageFilename = basename(options.outfile) + ".png";
    dest.Save(outImageFilename.c_str());
}
