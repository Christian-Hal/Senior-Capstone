
#include "Canvas.h"
#include <vector>

// constructor
Canvas::Canvas(int w, int h) : width(w), height(h), pixels(w * h) {}

// canvas size methods
int Canvas::getWidth() const { return width; }
int Canvas::getHeight() const { return height; }

// pixel manipulation
void Canvas::setPixel(int x, int y, const Color& color)
{
    // making sure (x, y) is within bounds
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    // formula that lets us keep it as a sizable, flat vector 
    // flat vectors are easier to handle memory wise so it
    //      ends up being better than a 2D vector / matrix
    pixels[y * width + x] = color;
}

// the const on this one makes it so that the original can't be changed
// it makes it read only
Color& Canvas::getPixel(int x, int y) const
{
    // making sure (x, y) is within bounds
    if (x < 0 || x >= width || y < 0 || y >= height) {
        // if its not, return the background color
        return const_cast<Color&>(backgroundColor);
    }

    return const_cast<Color&>(pixels[y * width + x]);
}