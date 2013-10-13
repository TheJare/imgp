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

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>

#include "ImagePacker.h"

void help(FILE *out) {
    fputs(
        "Usage: imgp in_files [-options]\n"
        "  Options:\n"
        "    -o, --output-filename filename    Output filename (mandatory)\n"
        "    -minw, --min-width    number      Minimum output width [64]\n"
        "    -maxw, --max-width    number      Maximum output width [4096]\n"
        "    -minh, --min-height   number      Minimum output height [64]\n"
        "    -maxh, --max-height   number      Maximum output height [4096]\n"
        "    -padx, --padding-x    number      Horizontal space between images [1]\n"
        "    -pady, --padding-y    number      Vertical space between images [1]\n"
        "    -fmt, --map-format    [json,plist,txt] Format of the map file [plist]\n"
        "    -flip, --allow-flip               Images can be flipped\n"
        "    -sq, --force-square               Output must be square\n"
	, out);
}

void error(const char *fmt, ...) {
    fputs("Error: ", stderr);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    help(stderr);
    exit(1);
}

const char *FindParam(int argc, char *argv[], const std::string &arg, int &i, const char *paramStr) {
    if (!paramStr && i+1 < argc) {
        paramStr = argv[++i];
    }
    if (paramStr && *paramStr) {
        return paramStr;
    } else {
        error("Invalid or missing argument for parameter %s: %s", arg.c_str(), paramStr);
    }
    return nullptr; // unreachable
}

int main(int argc, char* argv[])
{
    if (argc == 1) {
        help(stdout);
        return 0;
    }

    Options options;

    for (int i = 1; i < argc; ++i) {
        // If it's not an option then it is an input file
        if (argv[i][0] != '-') {
            options.AddInfile(argv[i]);
        } else {
            // Parameters to a command-line argument can be as follows:
            // -arg param
            // -arg=param
            const char *sep = strchr(argv[i], '=');
            const char *paramStr = NULL;
            std::string arg(argv[i], (sep == NULL)? strlen(argv[i]) : sep-argv[i]);
            if (sep != NULL) {
                paramStr = sep+1;
            }

            // Check and store options and their parameters
            if (arg.compare("-minw") == 0 || arg.compare("--min-width") == 0) {
                options.minw = atoi(FindParam(argc, argv, arg, i, paramStr));
            } else if (arg.compare("-maxw") == 0 || arg.compare("--max-width") == 0) {
                options.maxw = atoi(FindParam(argc, argv, arg, i, paramStr));
            } else if (arg.compare("-minh") == 0 || arg.compare("--min-height") == 0) {
                options.minh = atoi(FindParam(argc, argv, arg, i, paramStr));
            } else if (arg.compare("-max") == 0 || arg.compare("--max-height") == 0) {
                options.maxh = atoi(FindParam(argc, argv, arg, i, paramStr));
            } else if (arg.compare("-o") == 0 || arg.compare("--output-filename") == 0) {
                const char *param = FindParam(argc, argv, arg, i, paramStr);
                if (!options.outfile.empty()) {
                    error("Multiple output filenames specified: %s and %s", options.outfile.c_str(), param);
                }
                options.outfile = param;
            } else if (arg.compare("-padx") == 0 || arg.compare("--padding-x") == 0) {
                options.padx = atoi(FindParam(argc, argv, arg, i, paramStr));
            } else if (arg.compare("-pady") == 0 || arg.compare("--padding-y") == 0) {
                options.pady = atoi(FindParam(argc, argv, arg, i, paramStr));
            } else if (arg.compare("-fmt") == 0 || arg.compare("--format") == 0) {
                std::string fmt = FindParam(argc, argv, arg, i, paramStr);
                if (fmt.compare("json") == 0) options.format = Options::FORMAT_JSON;
                else if (fmt.compare("plist") == 0) options.format = Options::FORMAT_PLIST;
                else if (fmt.compare("txt") == 0) options.format = Options::FORMAT_TXT;
                else {
                    error("Unrecognized format: %s", fmt.c_str());
                }
            } else if (arg.compare("-flip") == 0 || arg.compare("--allow-flip") == 0) {
                options.allowFlipping = true;
            } else if (arg.compare("-sq") == 0 || arg.compare("--force-square") == 0) {
                options.forceSquare = true;
            } else if (arg.compare("-h") == 0 || arg.compare("--help") == 0) {
                help(stdout);
                exit(0);
            } else {
                error("Invalid argument: %s" , arg.c_str());
            }
        }
    }

    if (options.infiles.empty()) {
        error("No input files specified");
    }
    if (options.outfile.empty()) {
        error("No output file specified");
    }

    ImagePack(options);
	return 0;
}
