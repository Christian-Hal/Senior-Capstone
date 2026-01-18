

#include <vector>
#include <iostream>
#include <string>

#include "Canvas.h"


// constructor
Canvas::Canvas() : width(0), height(0), numLayers(0), curLayer(0), pixels(), layerData(), canvasName("") {}
Canvas::Canvas(int w, int h, std::string name) : width(w), height(h), numLayers(2), curLayer(1), pixels(w * h, backgroundColor), canvasName(name) 
{
    layerData.push_back(pixels);
    layerData.push_back(std::vector<Color>(w * h, emptyColor));
}

// canvas name methods
const std::string Canvas::getName() const
{
    return canvasName;
}
void Canvas::setName(std::string name)
{
    canvasName = name;
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

// Overloading operations
bool operator==(const Color& c2, const Color& c1)
{
    return (c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b)  && (c1.a == c2.a);
}

bool operator!=(const Color& c2, const Color& c1)
{
    return (c1.r != c2.r) || (c1.g != c2.g) || (c1.b != c2.b)  || (c1.a != c2.a);
}

// new 

Color operator*(const Color& c2, const Color& c1){
    
    // Normalize
    float a1 = c1.a / 255.0f;
    float a2 = c2.a / 255.0f;

    // Output alpha
    float outA = a1 + a2 * (1.0f - a1);

    if (outA == 0.0f)
        return {0, 0, 0, 0};

    // blending function
    // C = (C1 * a1 + C2 * a2 * (1 - a1)) / outA
    float r = (c1.r * a1 + c2.r * a2 * (1.0f - a1)) / outA;
    float g = (c1.g * a1 + c2.g * a2 * (1.0f - a1)) / outA;
    float b = (c1.b * a1 + c2.b * a2 * (1.0f - a1)) / outA;

    return {static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(outA * 255.0f)};
}



// pixel manipulation
void Canvas::setPixel(int x, int y, const Color& color)
{
    // making sure (x, y) is within bounds
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    // fit the color into the layerData vector so it can be accessed later
    layerData[curLayer][y * width + x] = color;

    // initialize the background color for later use in the for loop
    Color col = layerData[0][y * width + x];
    for(int i = 1; i < numLayers; i++){ 
        col = col * layerData[i][y * width + x];
    }
    
    // formula that lets us keep it as a sizable, flat vector 
    // flat vectors are easier to handle memory wise so it
    //      ends up being better than a 2D vector / matrix
    pixels[y * width + x] = col;
    
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


// adds a layer to layerData
void Canvas::createLayer(){
    numLayers++;
    layerData.insert(layerData.begin() + curLayer + 1, std::vector<Color>(width * height, emptyColor));
    curLayer = curLayer + 1;
}



// removes a layer from layerData and removes the pixel values on that layer
void Canvas::removeLayer(){
    // do not remove layers if there is only one layer
    if(numLayers > 2){
        // when removing a layer we need to iterate through through every pixel 
        for(int i = 0; i < pixels.size(); i++){
            // if the pixel value is not transparent
            if(pixels[i].a != 0 || layerData[curLayer][i].a ){
                // place a transparent pixel at i
                layerData[curLayer][i] = {0,0,0,0};
                // calculate what pixel will be with the new empty value
                Color col = layerData[0][i];
                for(int j = 0; j < numLayers; j++){
                    col = col * layerData[j][i];
                }
                pixels[i] = col;
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

