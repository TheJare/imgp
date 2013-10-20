# ImagePacker

A command line tool to pack a set of images into a single, larger image

Copyright (c) 2013, Javier Arevalo. Licensed under the MIT license.

Uses Sean Barret's image libraries from [http://nothings.org/](http://nothings.org/) 

Uses a modified version of the Guillotine bin packing algorithm by Jukka Jylänki from [https://github.com/juj/RectangleBinPack](https://github.com/juj/RectangleBinPack).

## Usage

    Usage: imgp in_files [-options]
      Options:
        -o, --output-filename filename    Output filename (mandatory)
        -minw, --min-width    number      Minimum output width [64]
        -maxw, --max-width    number      Maximum output width [4096]
        -minh, --min-height   number      Minimum output height [64]
        -maxh, --max-height   number      Maximum output height [4096]
        -padx, --padding-x    number      Horizontal space between images [1]
        -pady, --padding-y    number      Vertical space between images [1]
        -fmt, --map-format    format      Format of the map file [plist]
        -rot, --allow-rotate              Images can be rotated 90 deg
        -sq, --force-square               Output must be square
      Valid formats: plist, json-array, json-hash, txt

The output filename determines where the resulting image (always .png) and map file will be saved.

Examples:

    imgp Bitmaps/* -o Atlas/atlas.png -fmt json -flip
    imgp Bitmaps/* -o=Atlas/atlas -fmt=txt -sq -flip -minw=256 -minh=256

Resulting PNG files are not optimally compressed. I recommend using something like
[optipng](http://optipng.sourceforge.net/) or [pngcrush](http://pmt.sourceforge.net/pngcrush/)

Note: Not heavily tested yet, output formats may be still bogus.

## Building

Get the latest source code from [https://github.com/TheJare/imgp.git](https://github.com/TheJare/imgp.git)

On Windows, you can use the provided VS2012 project & solution, or run `mkvc.bat` on the command-line. For OSX and Linux you can run `./mkclang` or `./mkgcc` depending on your compiler.

The source code uses C++11 features and therefore requires a recent compiler. Tested with Visual Studio 2012 & 2013, clang 3.3/OSX and gcc 4.7/Ubuntu.

## License

Copyright (c) 2013, Javier Arevalo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
