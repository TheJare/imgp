/** @file Rect.h
    @author Jukka Jylänki

    Modified by Javier Arevalo. Original source: https://github.com/juj/RectangleBinPack

    This work is released to Public Domain, do whatever you want with it.
*/
#pragma once

#include <vector>
#include <cassert>
#include <cstdlib>

#ifdef _DEBUG
/// debug_assert is an assert that also requires debug mode to be defined.
#define debug_assert(x) assert(x)
#else
#define debug_assert(x)
#endif

#include "Image.h"

//using namespace std;

namespace rbp {

struct RectSize
{
    int width;
    int height;
    Image *image;
};

struct Rect
{
    int x;
    int y;
    int width;
    int height;
    Image *image;
    bool flipped;
};

/// Returns true if a is contained in b.
bool IsContainedIn(const Rect &a, const Rect &b);

}
