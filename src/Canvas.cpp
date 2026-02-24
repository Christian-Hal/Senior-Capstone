
#include <vector>
#include <iostream>
#include <string>

#include "Canvas.h"
#include "FrameRenderer.h"



// constructor
Canvas::Canvas() : width(0), height(0), numLayers(0), curLayer(0), pixels(), layerData(), canvasName("") {}
Canvas::Canvas(int w, int h, std::string name) : width(w), height(h), 
                    numLayers(2), curLayer(1), pixels(w * h, backgroundColor), 
                    canvasName(name), currentStrokeIndex(-1), seenPixels(w * h, -1)
{
    layerData.push_back(pixels);
    layerData.push_back(std::vector<Color>(w * h, emptyColor));
}

// Undo and Redo stuff
void Canvas::beginStrokeRecord() 
{
    // create a new StrokePath on the current layer
    activeStroke = StrokePath{};
    activeStroke.layerNum = curLayer;

    // sets a new stroke index for the seenPixel list
    currentStrokeIndex++;
}

// Records the change of a single pixel during a stroke, storing the index of the pixel and its previous color value
void Canvas::recordPixelChange(int index, const Color& before)
{
    // check if we've seen the pixel before during this stroke, keeps from reassigning the same pixel multiple times
    if (seenPixels[index] == currentStrokeIndex) {
        return;
    }

    // if we haven't seen the pixel before, then we mark it as seen and save it to the active stroke
    seenPixels[index] = currentStrokeIndex;
    Pixel pixel = {index, before, before};
    activeStroke.pixels.push_back(pixel);
}

void Canvas::endStrokeRecord()
{
    // set the after color for each pixel to said pixels current color
    for (Pixel& p : activeStroke.pixels)
    {
        p.after = layerData[activeStroke.layerNum][p.index];
    }

    // if the active stroke isn't empty then push it to the Undo stack
    if (!activeStroke.pixels.empty())
    {
        undoStack.push_back(activeStroke);
        redoStack.clear();
        if (undoStack.size() > 5) {
            undoStack.erase(undoStack.begin());
        }
    }

    // resets the activeStroke
    activeStroke = StrokePath{};
}

// returns true if the action can be done, false other wise
bool Canvas::canUndo() const { return !undoStack.empty(); }
bool Canvas::canRedo() const { return !redoStack.empty(); }

void Canvas::undo() 
{
    if (undoStack.empty()) {
        return;
    }

    // Pop the last stroke from the undo stack
    StrokePath stroke = undoStack.back();
    undoStack.pop_back();

    // save the real current layer num in a temp variable and move to the strokes layer
    int realLayer = curLayer;
    curLayer = stroke.layerNum;

    // for each pixel in the stroke, set it back to its before color
    for (Pixel p : stroke.pixels)
    {
        resetPixel(p.index, p.before);
    }

    // move back to the real current layer
    curLayer = realLayer;
    
    // move it to the redo stack
    redoStack.push_back(stroke);
}

void Canvas::redo() 
{
    if (redoStack.empty()) {
        return;
    }

    // Pop the last stroke from the redo stack
    StrokePath stroke = redoStack.back();
    redoStack.pop_back();

    // save the real current layer num in a temp variable and move to the strokes layer
    int realLayer = curLayer;
    curLayer = stroke.layerNum;

    // for each pixel in the stroke, set it back to its after color
    for (Pixel p : stroke.pixels)
    {
        resetPixel(p.index, p.after);
    }

    // move back to the real current layer
    curLayer = realLayer;
    
    // move it to the undo stack
    undoStack.push_back(stroke);
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

// Overloading operations

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

    int index = y * width + x;

    // record the pixel change before changing the color
    recordPixelChange(index, layerData[curLayer][index]);

    // fit the color into the layerData vector so it can be accessed later
    layerData[curLayer][index] = color;

    // initialize the background color for later use in the for loop
    Color col = layerData[0][index];
    for(int i = 1; i < numLayers; i++){ 
        col = col * layerData[i][index];
    }
    
    // formula that lets us keep it as a sizable, flat vector 
    // flat vectors are easier to handle memory wise so it
    //      ends up being better than a 2D vector / matrix
    pixels[index] = col;
    
}

void Canvas::resetPixel(int index, const Color color)
{
    // making sure (x, y) is within bounds
    if (index < 0 || static_cast<size_t>(index) >= pixels.size()) {
        return;
    }

    // fit the color into the layerData vector so it can be accessed later
    layerData[curLayer][index] = color;

    // initialize the background color for later use in the for loop
    Color col = layerData[0][index];
    for(int i = 1; i < numLayers; i++){ 
        col = col * layerData[i][index];
    }
    
    // formula that lets us keep it as a sizable, flat vector 
    // flat vectors are easier to handle memory wise so it
    //      ends up being better than a 2D vector / matrix
    pixels[index] = col;
    
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

    // idea: 	result = color_s * alpha_s + color_d * (1 - alpha_s)

    // making sure (x, y) is within bounds 
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    // if the incoming pixel is full opacity, just set the pixel. 
    if (src.a == 255) {
        this->setPixel(x, y, src);
        return;
    }

    int index = y * width + x; 

    // read existing color 
    Color& layerColor = layerData[curLayer][index];

    // scaling color with alpha values 
    Color srcColor; 
    srcColor.r = static_cast<unsigned char>(static_cast<float>(src.r) * brushAlpha); 
	srcColor.g = static_cast<unsigned char>(static_cast<float>(src.g) * brushAlpha);
	srcColor.b = static_cast<unsigned char>(static_cast<float>(src.b) * brushAlpha);
	srcColor.a = static_cast<unsigned char>(static_cast<float>(src.a) * brushAlpha);

    // blend source over destination 
    Color out = srcColor * layerColor;

    layerData[curLayer][index] = out; 

    // initialize the background color for later use in the for loop
    Color col = layerData[0][index];

    for (int i = 1; i < numLayers; i++) {

        col = col * layerData[i][index];
    }

    // formula that lets us keep it as a sizable, flat vector 
    pixels[index] = col;

}



// the const on this one makes it so that the original can't be changed
// it makes it read only
const Color& Canvas::getPixel(int x, int y) const
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

