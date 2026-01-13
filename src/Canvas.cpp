
#include "Canvas.h"
#include <vector>
#include <iostream>

// constructor
Canvas::Canvas() : width(0), height(0), numLayers(0), curLayer(0), pixels(), layerData() {}
Canvas::Canvas(int w, int h) : width(w), height(h), numLayers(2), curLayer(1), pixels(w * h, backgroundColor) 
{
    layerData.push_back(pixels);
    layerData.push_back(std::vector<Color>(w * h, emptyColor));
}

// canvas size methods
int Canvas::getWidth() const { return width; }
int Canvas::getHeight() const { return height; }

// layer number method
int Canvas::getNumLayers() const { return numLayers; }
int Canvas::getCurLayer() const { return curLayer; }

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
    
    // checks to see if there are any values in layers above whos
    // values are visible
    bool isVisible = true;
    for(int i = curLayer; i < numLayers; i++){
        if(layerData[i][y * width + x].a != 0){
            if(curLayer != i){
                isVisible = false;
            }
        }
    }

    layerData[curLayer][y * width + x] = color;
    
    // formula that lets us keep it as a sizable, flat vector 
    // flat vectors are easier to handle memory wise so it
    //      ends up being better than a 2D vector / matrix
    if(isVisible){
        pixels[y * width + x] = color;
    }
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



bool operator==(const Color& c2, const Color& c1)
{
    return (c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b)  && (c1.a == c2.a);
}

bool operator!=(const Color& c2, const Color& c1)
{
    return (c1.r != c2.r) || (c1.g != c2.g) || (c1.b != c2.b)  || (c1.a != c2.a);
}


// adds a layer to layerData
void Canvas::createLayer(){
    numLayers++;
    layerData.push_back(std::vector<Color>(width * height, emptyColor));
    curLayer = numLayers - 1;
}



// removes a layer from layerData and removes the pixel values on that layer
// [][][] There are some issues with the eraser and background when removing layers [][][]
void Canvas::removeLayer(){
    // do not remove layers if ther is only one layer
    if(numLayers > 2){
        
        // we need to reiterate through each value for each layer to
        // make sure pixel is correct        
        for(int i = 0; i < pixels.size(); i++){
            if(layerData[curLayer][i] == pixels[i] && pixels[i].a != 0){
                for(int j = 0; j < numLayers; j++){  
                    if(j != curLayer && layerData[j][i].a != 0){
                        pixels[i] = layerData[j][i];  
                    }
                }
            }
        }
        layerData.erase(layerData.begin()+curLayer);
        // update the number of layers and the current layer if needed
        numLayers--;
        if(layerData.size() == curLayer){
            curLayer--;
        }

        // since this has the unintended effected of ruining the data in
        // pixel, we need to reiterate through each value for each layer to
        // make sure pixel is correct
    }
}



void Canvas::selectLayer(int layerNum){
    curLayer = layerNum;
}

