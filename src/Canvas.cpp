
#include "Canvas.h"
#include <vector>
#include <iostream>

// constructor
Canvas::Canvas() : width(0), height(0), numLayers(0), curLayer(0), pixels(), layerData() {}
Canvas::Canvas(int w, int h) : width(w), height(h), numLayers(1), curLayer(0), pixels(w * h, backgroundColor), layerData{pixels} {}

// canvas size methods
int Canvas::getWidth() const { return width; }
int Canvas::getHeight() const { return height; }
// layer number method
int Canvas::getNumLayers() const { return numLayers; }

const Color* Canvas::getData() const {
    return pixels.data(); 
}

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

void Canvas::createLayer(){
    if(numLayers <= 1){
        numLayers++;
        layerData.push_back(std::vector<Color>(width * height, emptyColor));
        std::cout << numLayers << std::endl;
    }
}

void Canvas::removeLayer(){
    if(numLayers <= 1){
        numLayers--;
        // remove the last element
        layerData.pop_back();
        // since this has the unintended effected of ruining the data in
        // pixel, we need to reiterate through each value for each layer to
        // make sure pixel is correct
        // [][][][] ADD CODE HERE [][][][]
    }
}

void Canvas::selectLayer(int layerNum){
    curLayer = layerNum;
    std::cout << curLayer << " is the current layer" << std::endl;
}