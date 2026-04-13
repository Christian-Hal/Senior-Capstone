
// included libraries for functionality
#include "FrameRenderer.h"
#include "stb_image_write.h"
#include "imgui.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <array>
#include <thread>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;

// initalize the FrameRenderer variables
int FrameRenderer::numCanvas = -1;
int FrameRenderer::numFrames = -1;
int FrameRenderer::curCanvas = -1;
int FrameRenderer::curFrame = -1;
bool FrameRenderer::isPlaying = false;
int FrameRenderer::numBefore = 1;
int FrameRenderer::numAfter = 1;
bool FrameRenderer::onionSkinEnabled = false;
bool FrameRenderer::inputBlocked = false;

// this will be stored in memory so we can access it quickly everything else gets written to a file
// we need the frame data so we can play high fps animations
vector<vector<Color>> FrameRenderer::frames;

// this is the Layer data for each frame the first vector is the canvas, the second is the frame and the 3rd is the layer 4th is pixel location/color
vector<vector<vector<vector<Color>>>> FrameRenderer::frLayerData;
// this is the frames data first vector is canvas, second is the frame, third is the pixel location/color
vector<vector<vector<Color>>> FrameRenderer::frameData;
//this is how we store metadata each vector is a new canvas
vector<array<int, 4>> FrameRenderer::metaData;

// ONLY CALL THIS FUNCTION ONCE!!!
// THIS IS MEANT TO BE MOSTLY STATIC SO YOU CAN REFERENCE THE CLASS ANYWHERE
// CREATING A SECOND FRAME RENDERER WILL BREAK EVERYTHING
// if this is "bad practice" fuck you i dont care
FrameRenderer::FrameRenderer()
{
    numCanvas = 0;
    numFrames = 0;
    curCanvas = -1;
    curFrame = -1;
}

// called whenever a new canvas is made
// this saves whatever was on the current canvas (if there was a current canvas)
// this changes the "frames" variable to the proper [empty] pixels values since we changed canvases 
void FrameRenderer::newCanvas(Canvas* oldCanvas, Canvas* newCanvas){
    // Save the data if there already was a canvas
    if(numCanvas != 0){
        removeOnionSkin(*oldCanvas);
        frames[curFrame - 1] = vector<Color>(oldCanvas->getData(), oldCanvas->getData() + (oldCanvas->getWidth() * oldCanvas->getHeight()));
        writeAllData(oldCanvas);
    }
    numCanvas++;
    curCanvas = numCanvas;
    numFrames = 1;
    curFrame = 1;

    // save the first "frame" to frames
    // if its a new canvas we only have 1 frame, the starting frame
    frames.clear();
    frames.push_back(vector<Color>(newCanvas->getData(), newCanvas->getData() + (newCanvas->getWidth() * newCanvas->getHeight())));
    
    // save a blank "new canvas" in frLayerData
    frLayerData.push_back(
        vector<vector<vector<Color>>>(
            numFrames,
            vector<vector<Color>>(
                1,
                vector<Color>(newCanvas->getWidth() * newCanvas->getHeight(), newCanvas->getBackgroundColor())
            )
        )
    );
    // save a blank "new canvas" in frameData
    frameData.push_back(
        vector<vector<Color>>(
            numFrames,
            vector<Color>(newCanvas->getWidth() * newCanvas->getHeight(), newCanvas->getBackgroundColor())
        )
    );
    // save a blank "new canvas" in metaData
    metaData.push_back({0,0,0,0});

    writeAllData(newCanvas);
    updateOnionSkin(*newCanvas);
}

// this function is called whenever you move from one canvas to another
// saves the pixelData and frame data 
// updates the frames vector with the proper array.
// pulls the correct layerData for the first frame.
// [NOTE] : no matter what frame you were on last visit, you will always go back to frame 1 after changing canvases
void FrameRenderer::updateCanvas(Canvas* oldCanvas, Canvas* newCanvas, int newCanvasIndex){
    
    // Save data
    if(numCanvas != 0){
        removeOnionSkin(*oldCanvas);
        frames[curFrame - 1] =  vector<Color>(oldCanvas->getData(), oldCanvas->getData() + (oldCanvas->getWidth() * oldCanvas->getHeight()));
        writeAllData(oldCanvas);
        
    }

    // update internal values
    curCanvas = newCanvasIndex + 1;
    curFrame = 1;

    // after saving we can clear the frame
    frames.clear();

    // and load it with the new canvas
    int* meta = readMetaData();
    numFrames = meta[3]; // meta contains [canvasWidth, canvasHeight, numLayers, numFrames]

    // getFrame data and assign it to the active canvas
    frames = readPixelData(meta);
    newCanvas->setPixels(frames[curFrame - 1]);
    newCanvas->setLayerData(readLayerData(meta));
    updateOnionSkin(*newCanvas);
}


// save changes to the old frame
// inserts the frame at 1 + curFrame and update numFrames
// updates all frame filenames accordingly
// loads new frame with blank canvas
void FrameRenderer::createFrame(Canvas& canvas){
    // Save the old frame
    removeOnionSkin(canvas);
    frames[curFrame - 1] =  vector<Color>(canvas.getData(), canvas.getData() + (canvas.getWidth() * canvas.getHeight()));
    writeAllData(&canvas);
    numFrames++;
    curFrame++;
    // insert new frame
    int* meta = readMetaData(); // meta[0] is width, meta[1] is height

    // this ought to insert inbetween the oldCurrent frame
    frames.insert(frames.begin() + (curFrame - 1), vector<Color>(meta[0] * meta[1], canvas.getBackgroundColor()));
    
    frLayerData[curCanvas-1].insert(frLayerData[curCanvas-1].begin() + (curFrame-1), vector<vector<Color>>(
            canvas.getNumLayers(), vector<Color>(
                meta[0] * meta[1], canvas.getBackgroundColor()
            )
        )
    );
    frameData[curCanvas-1].insert(frameData[curCanvas-1].begin() + (curFrame - 1), vector<Color>(meta[0] * meta[1], canvas.getBackgroundColor()));

    canvas.setPixels(frames[curFrame - 1]);
    //band-aid solution. this does not fix removing layers fully
    vector<vector<Color>> layDat(meta[2], vector<Color>(meta[0] * meta[1], {0,0,0,0})); //meta[2] is the number of layers
    layDat[0] = vector<Color>(meta[0] * meta[1], {255,255,255,255});
    canvas.setLayerData(layDat);
    writeAllData(&canvas);
    updateOnionSkin(canvas);
}

// remove current frame and update file names accordingly
void FrameRenderer::removeFrame(Canvas& canvas){
    if(numFrames > 1){
        // erases the frameData
        frames.erase(frames.begin() + curFrame - 1);
        // erases the layerData
        //[][][][]
        
        frLayerData[curCanvas - 1].erase(frLayerData[curCanvas-1].begin() + curFrame - 1);
        frameData[curCanvas - 1].erase(frameData[curCanvas-1].begin() + curFrame - 1);
        
        // fixes the names of layerdata
        if(curFrame == numFrames){
            curFrame--;
        }
        numFrames--;

        // saves the information (changed frameData, changed metadata)
        int* meta = readMetaData();
        canvas.setPixels(frames[curFrame-1]);
        canvas.setLayerData(readLayerData(meta));
        updateOnionSkin(canvas);
    }
}


// save the current data to the drive
// load the pixelDatas from the corrrect file to here
// update the canvas with info from the drive
void FrameRenderer::selectFrame(Canvas& canvas, int frameDelta){
    if(0 < curFrame + frameDelta && curFrame + frameDelta <= numFrames){
        // save data to drive
        removeOnionSkin(canvas);
        frames[curFrame - 1] =  vector<Color>(canvas.getData(), canvas.getData() + (canvas.getWidth() * canvas.getHeight()));
        writeAllData(&canvas);
        curFrame = curFrame + frameDelta;
        int* meta = readMetaData();
        canvas.setPixels(frames[curFrame-1]);
        canvas.setLayerData(readLayerData(meta));
        updateOnionSkin(canvas);
    }
}

/* 
This function plays the animation

it uses one thread on the CPU so it can run in parallel with the program and will cycle through
the "Frames" vector until it gets to the end.

after this the thread gets detached so it can be used by other things
*/
void FrameRenderer::play(Canvas& canvas){
    if(!isPlaying){
        removeOnionSkin(canvas);
        inputBlocked = true;
        frames[curFrame - 1] = vector<Color>(canvas.getData(), canvas.getData() + (canvas.getWidth() * canvas.getHeight()));
        int start = 1;
        std::thread t([&canvas, start]{
            isPlaying = true;
            int i = start;
            while (i <= numFrames) {
                canvas.setPixels(frames[i-1]);
                i++;
                std::this_thread::sleep_for(std::chrono::milliseconds(84)); // 42 milliseconds is ~ 24 fps
            }
            canvas.setPixels(frames[curFrame - 1]);
            isPlaying = false;
            updateOnionSkin(canvas);
            inputBlocked = false;
        });
        t.detach();
    }

}

void FrameRenderer::updateOnionSkin(Canvas& canvas){
    if(onionSkinEnabled){
        Color green = {0,255,0,128};
        Color blendedColor = canvas.colorTimes(canvas.getBackgroundColor(), green);
        int oldLayer = canvas.getCurLayer();
        canvas.selectLayer(0);
        vector<vector<Color>> layDat = canvas.getLayerData();
        for(int i = 0; i < numBefore; i++){
            if(curFrame > 1 + i){
                for(int j = 0; j < layDat[0].size(); j++){
                    if(!canvas.colorEquals(frames[curFrame - 2 - i][j], canvas.getBackgroundColor())){
                        int x = j % canvas.getWidth();
                        int y = j / canvas.getWidth();
                        canvas.blendPixel(x, y, blendedColor, blendedColor.a / 255.0f);
                    }
                }
            }
        }
        green = {255, 0, 0, 128};
        blendedColor = canvas.colorTimes(canvas.getBackgroundColor(), green);
        for(int i = 0; i < numAfter; i++){
            if(curFrame < numFrames - i){ // This line needs fixing
                for(int j = 0; j < layDat[0].size(); j++){
                    if(!canvas.colorEquals(frames[curFrame + i][j], canvas.getBackgroundColor())){
                        int x = j % canvas.getWidth();
                        int y = j / canvas.getWidth();
                        canvas.blendPixel(x, y, blendedColor, blendedColor.a / 255.0f);
                    }
                }
            }
        }
        canvas.selectLayer(oldLayer);
    }
}

void FrameRenderer::removeOnionSkin(Canvas& canvas){
    int oldLayer = canvas.getCurLayer();
    canvas.selectLayer(0);
    int wid = canvas.getWidth();
    int hei = canvas.getHeight();
    for(int i = 0; i < hei * wid; i++){
        canvas.setPixel(i % wid, i / wid, canvas.getBackgroundColor());
    }
    canvas.selectLayer(oldLayer);
}

void FrameRenderer::toggleOnionSkin(){
    onionSkinEnabled = !onionSkinEnabled;
}

void FrameRenderer::saveAnimation(const string& path, Canvas& canvas){
    writeAllData(&canvas);
    int width = canvas.getWidth();
    int height = canvas.getHeight();
    string prefix = path.substr(0, path.find_last_of('.'));
    string ext = path.substr(path.find_last_of('.') + 1);
    size_t slash = path.find_last_of('/');
    bool isMac = false;
    // if your on mac
    if (slash == std::string::npos) {
        slash = path.find_last_of("\\");
        isMac = true;
    }
    string title = path.substr(slash + 1, path.find_last_of('.') - (slash + 1));
    fs::create_directory(prefix);
    for(int i = 0; i < frames.size(); i++){
        string finalPath;
        if(!isMac){
            finalPath = prefix + "/" + title + "-" + to_string(i) + "." + ext;
        }
        else{
            finalPath = prefix + "\\" + title + "-" + to_string(i) + "." + ext;
        }
        vector<Color> pixels(width * height);
        memcpy(pixels.data(), frames[i].data(), width * height * sizeof(Color));
        for (int y = 0; y < height / 2; y++)
        {
            int opposite = height - y - 1;
            for (int x = 0; x < width; x++)
            {
                swap(pixels[y * width + x], pixels[opposite * width + x]);
            }
        }

        if (ext == "png")
            stbi_write_png(finalPath.c_str(), width, height, 4, pixels.data(), width * 4);
    
        else if (ext == "jpg")
            stbi_write_jpg(finalPath.c_str(), width, height, 4, pixels.data(), 100);
    }
}

// removes all of the files after shutdown so you dont eat up storage.
void FrameRenderer::shutdown(){

}

// gets the current frame (which is a number from 1-NumFrames)
int FrameRenderer::getCurFrame(){
    return curFrame;
}

// gets the total number of frames in this canvas
int FrameRenderer::getNumFrames(){
    return numFrames;
}
// returns the number of onion skin frames shown before
int FrameRenderer::getNumAfter(){
    return numAfter;
}
void FrameRenderer::setNumAfter(int newNumAfter){
    if(newNumAfter <= numFrames && newNumAfter >= 0){
        numAfter = newNumAfter;
    }
}

int FrameRenderer::getNumBefore(){
    return numBefore;
}

void FrameRenderer::setNumBefore(int newNumBefore){
    if(newNumBefore <= numFrames && newNumBefore >= 0){
        numBefore = newNumBefore;
    }
}
// --------------------- Private functions ------------------------------ 
void FrameRenderer::writeAllData(Canvas* canvas){
    writeMetaData(canvas);
    writePixelData(canvas);
    writeLayerData(canvas);
}

//saves the width, hight, number of layers, and number of frames
// one file for each canvas
void FrameRenderer::writeMetaData(Canvas* canvas){
    // 0 is width 1 is height 2 is numLayers 3 is numFrames
    int canvasWidth = canvas->getWidth();
    int canvasHeight = canvas->getHeight();
    int numLayers = canvas->getNumLayers();
    metaData[curCanvas-1] = {canvasWidth, canvasHeight, numLayers, numFrames};
}

// saves the vector frames so we can access it later in the event we use a different canvas.
// one file for each canvas.
void FrameRenderer::writePixelData(Canvas* canvas){
    const Color* pixelData = canvas->getData();
    vector<Color> curPixels(pixelData, pixelData + (canvas->getWidth() * canvas->getHeight()));
    frameData[curCanvas-1][curFrame-1] = curPixels;
}

// saves the layerData for each frame in a canvas. will be written to layerData in canvas every time the frame is updated.
// one file for each frame in a canvas
void FrameRenderer::writeLayerData(Canvas* canvas){
    vector<vector<Color>> curLayerDat = canvas->getLayerData();
    frLayerData[curCanvas-1][curFrame - 1] = curLayerDat;
}

// returns a pointer where you can access [width, height, NumLayers, NumFrames]
int* FrameRenderer::readMetaData() {
    return metaData[numCanvas-1].data();
}

vector<vector<Color>> FrameRenderer::readPixelData(int* arr) {
    return frameData[curCanvas - 1];
}
    
vector<vector<Color>> FrameRenderer::readLayerData(int* arr){
    return frLayerData[curCanvas-1][curFrame-1];
}