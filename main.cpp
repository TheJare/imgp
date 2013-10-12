// ImagePacker.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdarg.h>
#include <vector>

#include "ImagePacker.h"

void help(FILE *out) {
    fputs(
        "Usage: ImagePacker in_files [-options]\n"
        "  Options:\n"
        "    -o, --output-filename filename    Output filename\n"
        "    -minw, --min-width    number      Minimum output texture width\n"
        "    -maxw, --max-width    number      Maximum output texture width\n"
        "    -minh, --min-height   number      Minimum output texture height\n"
        "    -maxh, --max-height   number      Maximum output texture height\n"
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
        if (argv[i][0] != '-') {
            options.AddInfile(argv[i]);
        } else {
            const char *sep = strchr(argv[i], '=');
            const char *paramStr = NULL;
            std::string arg(argv[i], (sep == NULL)? strlen(argv[i]) : sep-argv[i]);
            if (sep != NULL) {
                paramStr = sep+1;
            }

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
            } else {
                error("Invalid argument: %s" , arg.c_str());
            }
        }
    }

    if (options.infiles.empty()) {
        error("No input files specified");
    }
	return 0;
}
