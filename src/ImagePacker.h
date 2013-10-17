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

#ifndef INCLUDE_IMAGEPACKER_H
#define INCLUDE_IMAGEPACKER_H

#include <vector>
#include <string>

struct Options {
    enum Format {
        FORMAT_TXT,
        FORMAT_JSON,
        FORMAT_PLIST,
    };

    int minw;
    int minh;
    int maxw;
    int maxh;
    int padx;
    int pady;
    bool allowFlipping;
    bool forceSquare;
    Format format;

    std::vector<std::string> infiles;
    std::string outfile;

    Options() {
        minw = 64;
        minh = 64;
        maxw = 4096;
        maxh = 4096;
        padx = 1;
        pady = 1;
        allowFlipping = false;
        forceSquare = false;
        format = FORMAT_PLIST;
    }

    void AddInfile(const char *filename);
};

void ImagePack(const Options &options);


#endif //INCLUDE_IMAGEPACKER_H
