/** @file GuillotineBinPack.cpp
    @author Jukka Jyl�nki

    Modified by Javier Arevalo. Original source: https://github.com/juj/RectangleBinPack

    @brief Implements different bin packer algorithms that use the GUILLOTINE data structure.

    This work is released to Public Domain, do whatever you want with it.
*/
#include <utility>
#include <iostream>
#include <limits>
#include <algorithm>

#include <cassert>
#include <cstring>
#include <cmath>

#include "GuillotineBinPack.h"

namespace rbp {

using namespace std;

GuillotineBinPack::GuillotineBinPack()
:binWidth(0),
binHeight(0)
{
}

GuillotineBinPack::GuillotineBinPack(int width, int height)
{
    Init(width, height);
}

void GuillotineBinPack::Init(int width, int height)
{
    binWidth = width;
    binHeight = height;
    occupiedWidth = 0;
    occupiedHeight = 0;

    // Clear any memory of previously packed rectangles.
    usedRectangles.clear();

    // We start with a single big free rectangle that spans the whole bin.
    Rect n;
    n.x = 0;
    n.y = 0;
    n.width = width;
    n.height = height;

    freeRectangles.clear();
    freeRectangles.push_back(n);
}

bool GuillotineBinPack::Insert(std::vector<RectSize> rects, bool merge, bool flip, 
    FreeRectChoiceHeuristic rectChoice, GuillotineSplitHeuristic splitMethod)
{
    // Remember variables about the best packing choice we have made so far during the iteration process.
    int bestFreeRect = 0;
    int bestRect = 0;
    bool bestFlipped = false;

    // Pack rectangles one at a time until we have cleared the rects array of all rectangles.
    // rects will get destroyed in the process.
    while(rects.size() > 0)
    {
        // Stores the penalty score of the best rectangle placement - bigger=worse, smaller=better.
        int bestScore = std::numeric_limits<int>::max();

        for(size_t i = 0; i < freeRectangles.size(); ++i)
        {
            for(size_t j = 0; j < rects.size(); ++j)
            {
                // If this rectangle is a perfect match, we pick it instantly.
                if (rects[j].width == freeRectangles[i].width && rects[j].height == freeRectangles[i].height)
                {
                    bestFreeRect = i;
                    bestRect = j;
                    bestFlipped = false;
                    bestScore = std::numeric_limits<int>::min();
                    i = freeRectangles.size(); // Force a jump out of the outer loop as well - we got an instant fit.
                    break;
                }
                // If flipping this rectangle is a perfect match, pick that then.
                else if (flip && rects[j].height == freeRectangles[i].width && rects[j].width == freeRectangles[i].height)
                {
                    bestFreeRect = i;
                    bestRect = j;
                    bestFlipped = true;
                    bestScore = std::numeric_limits<int>::min();
                    i = freeRectangles.size(); // Force a jump out of the outer loop as well - we got an instant fit.
                    break;
                }
                // Try if we can fit the rectangle upright.
                else if (rects[j].width <= freeRectangles[i].width && rects[j].height <= freeRectangles[i].height)
                {
                    int score = ScoreByHeuristic(rects[j].width, rects[j].height, freeRectangles[i], rectChoice);
                    if (score < bestScore)
                    {
                        bestFreeRect = i;
                        bestRect = j;
                        bestFlipped = false;
                        bestScore = score;
                    }
                }
                // If not, then perhaps flipping sideways will make it fit?
                else if (flip && rects[j].height <= freeRectangles[i].width && rects[j].width <= freeRectangles[i].height)
                {
                    int score = ScoreByHeuristic(rects[j].height, rects[j].width, freeRectangles[i], rectChoice);
                    if (score < bestScore)
                    {
                        bestFreeRect = i;
                        bestRect = j;
                        bestFlipped = true;
                        bestScore = score;
                    }
                }
            }
        }

        // If we didn't manage to find any rectangle to pack, abort.
        if (bestScore == std::numeric_limits<int>::max())
            return false;

        // Otherwise, we're good to go and do the actual packing.
        Rect newNode;
        newNode.x = freeRectangles[bestFreeRect].x;
        newNode.y = freeRectangles[bestFreeRect].y;
        newNode.width = rects[bestRect].width;
        newNode.height = rects[bestRect].height;
        newNode.flipped = bestFlipped;
        newNode.image = rects[bestRect].image;

        if (bestFlipped)
            std::swap(newNode.width, newNode.height);

        occupiedWidth = std::max(occupiedWidth, newNode.x + newNode.width);
        occupiedHeight = std::max(occupiedHeight, newNode.y + newNode.height);

        // Remove the free space we lost in the bin.
        SplitFreeRectByHeuristic(freeRectangles[bestFreeRect], newNode, splitMethod);
        freeRectangles.erase(freeRectangles.begin() + bestFreeRect);

        // Remove the rectangle we just packed from the input list.
        rects.erase(rects.begin() + bestRect);

        // Perform a Rectangle Merge step if desired.
        if (merge)
            MergeFreeList();

        // Remember the new used rectangle.
        usedRectangles.push_back(newNode);
    }
    return true;
}

/// @return True if r fits inside freeRect (possibly rotated).
bool Fits(const RectSize &r, const Rect &freeRect)
{
    return (r.width <= freeRect.width && r.height <= freeRect.height) ||
        (r.height <= freeRect.width && r.width <= freeRect.height);
}

/// @return True if r fits perfectly inside freeRect, i.e. the leftover area is 0.
bool FitsPerfectly(const RectSize &r, const Rect &freeRect)
{
    return (r.width == freeRect.width && r.height == freeRect.height) ||
        (r.height == freeRect.width && r.width == freeRect.height);
}

/// Computes the ratio of used surface area to the total bin area.
float GuillotineBinPack::Occupancy() const
{
    ///\todo The occupancy rate could be cached/tracked incrementally instead
    ///      of looping through the list of packed rectangles here.
    unsigned long usedSurfaceArea = 0;
    for(size_t i = 0; i < usedRectangles.size(); ++i)
        usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

    return (float)usedSurfaceArea / (binWidth * binHeight);
}

/// Returns the heuristic score value for placing a rectangle of size width*height into freeRect. Does not try to rotate.
int GuillotineBinPack::ScoreByHeuristic(int width, int height, const Rect &freeRect, FreeRectChoiceHeuristic rectChoice)
{
    switch(rectChoice)
    {
    case RectBestAreaFit: return ScoreBestAreaFit(width, height, freeRect);
    case RectBestShortSideFit: return ScoreBestShortSideFit(width, height, freeRect);
    case RectBestLongSideFit: return ScoreBestLongSideFit(width, height, freeRect);
    case RectWorstAreaFit: return ScoreWorstAreaFit(width, height, freeRect);
    case RectWorstShortSideFit: return ScoreWorstShortSideFit(width, height, freeRect);
    case RectWorstLongSideFit: return ScoreWorstLongSideFit(width, height, freeRect);
    default: assert(false); return std::numeric_limits<int>::max();
    }
}

int GuillotineBinPack::ScoreBestAreaFit(int width, int height, const Rect &freeRect)
{
    return freeRect.width * freeRect.height - width * height;
}

int GuillotineBinPack::ScoreBestShortSideFit(int width, int height, const Rect &freeRect)
{
    int leftoverHoriz = abs(freeRect.width - width);
    int leftoverVert = abs(freeRect.height - height);
    int leftover = min(leftoverHoriz, leftoverVert);
    return leftover;
}

int GuillotineBinPack::ScoreBestLongSideFit(int width, int height, const Rect &freeRect)
{
    int leftoverHoriz = abs(freeRect.width - width);
    int leftoverVert = abs(freeRect.height - height);
    int leftover = max(leftoverHoriz, leftoverVert);
    return leftover;
}

int GuillotineBinPack::ScoreWorstAreaFit(int width, int height, const Rect &freeRect)
{
    return -ScoreBestAreaFit(width, height, freeRect);
}

int GuillotineBinPack::ScoreWorstShortSideFit(int width, int height, const Rect &freeRect)
{
    return -ScoreBestShortSideFit(width, height, freeRect);
}

int GuillotineBinPack::ScoreWorstLongSideFit(int width, int height, const Rect &freeRect)
{
    return -ScoreBestLongSideFit(width, height, freeRect);
}

void GuillotineBinPack::SplitFreeRectByHeuristic(const Rect &freeRect, const Rect &placedRect, GuillotineSplitHeuristic method)
{
    // Compute the lengths of the leftover area.
    const int w = freeRect.width - placedRect.width;
    const int h = freeRect.height - placedRect.height;

    // Placing placedRect into freeRect results in an L-shaped free area, which must be split into
    // two disjoint rectangles. This can be achieved with by splitting the L-shape using a single line.
    // We have two choices: horizontal or vertical. 

    // Use the given heuristic to decide which choice to make.

    bool splitHorizontal;
    switch(method)
    {
    case SplitShorterLeftoverAxis:
        // Split along the shorter leftover axis.
        splitHorizontal = (w <= h);
        break;
    case SplitLongerLeftoverAxis:
        // Split along the longer leftover axis.
        splitHorizontal = (w > h);
        break;
    case SplitMinimizeArea:
        // Maximize the larger area == minimize the smaller area.
        // Tries to make the single bigger rectangle.
        splitHorizontal = (placedRect.width * h > w * placedRect.height);
        break;
    case SplitMaximizeArea:
        // Maximize the smaller area == minimize the larger area.
        // Tries to make the rectangles more even-sized.
        splitHorizontal = (placedRect.width * h <= w * placedRect.height);
        break;
    case SplitShorterAxis:
        // Split along the shorter total axis.
        splitHorizontal = (freeRect.width <= freeRect.height);
        break;
    case SplitLongerAxis:
        // Split along the longer total axis.
        splitHorizontal = (freeRect.width > freeRect.height);
        break;
    default:
        splitHorizontal = true;
        assert(false);
    }

    // Perform the actual split.
    SplitFreeRectAlongAxis(freeRect, placedRect, splitHorizontal);
}

/// This function will add the two generated rectangles into the freeRectangles array. The caller is expected to
/// remove the original rectangle from the freeRectangles array after that.
void GuillotineBinPack::SplitFreeRectAlongAxis(const Rect &freeRect, const Rect &placedRect, bool splitHorizontal)
{
    // Form the two new rectangles.
    Rect bottom;
    bottom.x = freeRect.x;
    bottom.y = freeRect.y + placedRect.height;
    bottom.height = freeRect.height - placedRect.height;

    Rect right;
    right.x = freeRect.x + placedRect.width;
    right.y = freeRect.y;
    right.width = freeRect.width - placedRect.width;

    if (splitHorizontal)
    {
        bottom.width = freeRect.width;
        right.height = placedRect.height;
    }
    else // Split vertically
    {
        bottom.width = placedRect.width;
        right.height = freeRect.height;
    }

    // Add the new rectangles into the free rectangle pool if they weren't degenerate.
    if (bottom.width > 0 && bottom.height > 0)
        freeRectangles.push_back(bottom);
    if (right.width > 0 && right.height > 0)
        freeRectangles.push_back(right);
}

void GuillotineBinPack::MergeFreeList()
{
    // Do a Theta(n^2) loop to see if any pair of free rectangles could me merged into one.
    // Note that we miss any opportunities to merge three rectangles into one. (should call this function again to detect that)
    for(size_t i = 0; i < freeRectangles.size(); ++i)
        for(size_t j = i+1; j < freeRectangles.size(); ++j)
        {
            if (freeRectangles[i].width == freeRectangles[j].width && freeRectangles[i].x == freeRectangles[j].x)
            {
                if (freeRectangles[i].y == freeRectangles[j].y + freeRectangles[j].height)
                {
                    freeRectangles[i].y -= freeRectangles[j].height;
                    freeRectangles[i].height += freeRectangles[j].height;
                    freeRectangles.erase(freeRectangles.begin() + j);
                    --j;
                }
                else if (freeRectangles[i].y + freeRectangles[i].height == freeRectangles[j].y)
                {
                    freeRectangles[i].height += freeRectangles[j].height;
                    freeRectangles.erase(freeRectangles.begin() + j);
                    --j;
                }
            }
            else if (freeRectangles[i].height == freeRectangles[j].height && freeRectangles[i].y == freeRectangles[j].y)
            {
                if (freeRectangles[i].x == freeRectangles[j].x + freeRectangles[j].width)
                {
                    freeRectangles[i].x -= freeRectangles[j].width;
                    freeRectangles[i].width += freeRectangles[j].width;
                    freeRectangles.erase(freeRectangles.begin() + j);
                    --j;
                }
                else if (freeRectangles[i].x + freeRectangles[i].width == freeRectangles[j].x)
                {
                    freeRectangles[i].width += freeRectangles[j].width;
                    freeRectangles.erase(freeRectangles.begin() + j);
                    --j;
                }
            }
        }
}

}
