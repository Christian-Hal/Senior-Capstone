
#include <vector>
#include <iostream>
#include <string>

#include "Canvas.h"
#include "FrameRenderer.h"



// constructor
Canvas::Canvas() : width(0), height(0), numLayers(0), curLayer(0), pixels(), layerData(), canvasName("") {}
Canvas::Canvas(int w, int h, std::string name) : width(w), height(h), numLayers(2), curLayer(1), pixels(w * h, backgroundColor), canvasName(name) 
{
    layerData.push_back(pixels);
    layerData.push_back(std::vector<Color>(w * h, emptyColor));
}



/*
    Canvas getter. 

    @return name of the canvas. 
*/
const std::string Canvas::getName() const
{
    return canvasName;
}



/*
    Canvas setter. 
*/
void Canvas::setName(std::string name)
{
    canvasName = name;
}




/*
    Canvas width and height getters. 
*/
int Canvas::getWidth() const { return width; }
int Canvas::getHeight() const { return height; }

/*
    Canvas number of layers and current layer getters. 
*/
int Canvas::getNumLayers() const { return numLayers; }
int Canvas::getCurLayer() const { return curLayer; }



/*
    Getter for pixel data. 

    @return pixel data as a Color type pointer 
*/
const Color* Canvas::getData() const {
    return pixels.data(); 
}



/*
    Equality operator overload for Color datatype. 

    Is true if rgba values are equal for both Colors. 
*/
const std::vector<std::vector<Color>>& Canvas::getLayerData() const {
    return layerData;
}



// Overloading operations
bool operator==(const Color& c2, const Color& c1)
{
    return (c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b)  && (c1.a == c2.a);
}



/*
    Inequality operator overload for Color datatype.

    Is true if any of the rgba values are not equal for both Colors. 

    EX: if color1 red is not equal to color2 red this is true.
*/
bool operator!=(const Color& c2, const Color& c1)
{
    return (c1.r != c2.r) || (c1.g != c2.g) || (c1.b != c2.b)  || (c1.a != c2.a);
}



/*
    Multiplication operator overload for Color datatype. 

    Used to blend two colors together. 

    Uses the standard alpha blending formula. 

    Note: Currently does not apply to drawings on a singular layer due to 
          set pixel replacing pixels rather than drawing them. 
*/
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



/*
    If the pixel at a given coordinate is within bounds, 
    the color is drawn onto that layer. 

    @param x: The x coordinate of the pixel to be set.
    @param y: The y coordinate of the pixel to be set. 
    @param color: The color to set the pixel to. 
*/
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



void Canvas::setPixels(std::vector<Color> newPixels){
    pixels = newPixels;
}



void Canvas::setLayerData(std::vector<std::vector<Color>> newLayerData){
    layerData = newLayerData;
}



/*
    Note: Not yet implemented 
void Canvas::setLayerData(std::vector<std::vector<Color>> newLayerData){
    layerData = newLayerData;
}

    Modified version of setPixel which does not replace 
    pixels entirely but rather accumulates them to 
    allow for single layer blending. 

    If the alpha value of the source pixel is full, it 
    calls setPixel instead. 

    @param x: The x coordinate of the pixel to be blended. 
    @param y: The y coordinate of the pixel to be blended. 
    @param src: The color value of the source pixel, which is the pixel to be set. 
    @param brushAlpha: The alpha of the current brush, used in calculating the blending. 
*/
void Canvas::blendPixel(int x, int y, const Color& src, float brushAlpha) {

    // making sure (x, y) is within bounds 
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    if (src.a == 255) {
        this->setPixel(x, y, src);
        return;
    }

    int index = y * width + x; 

    // read existing color 
    Color& layerColor = layerData[curLayer][index];

    // convert normalized alpha into 0-255 rep 
    Color srcColor = src;
    srcColor.a = static_cast<unsigned char>(srcColor.a * brushAlpha);

    // blend source over destination 
    Color out = srcColor * layerColor;

    layerData[curLayer][index] = out; 


    // initialize the background color for later use in the for loop
    Color col = layerData[0][index];

    for (int i = 1; i < numLayers; i++) {

        col = col * layerData[i][index];
    }

    // formula that lets us keep it as a sizable, flat vector 
    // flat vectors are easier to handle memory wise so it
    //      ends up being better than a 2D vector / matrix
    pixels[index] = col;

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

