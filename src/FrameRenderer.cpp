
// included libraries for functionality
#include "FrameRenderer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>


// NOTES - there are 3 types of files in a canvas folder: the metaData file containing the width, hieght, number of layers and number of frames this is stored in meta.dat
//          each value is on a new line and stored in the order previously given
//          The second type of file is the frames[num][data] data, this contains the pixels[] information for each frame in a canvas tihs is stored in framedata.dat
//          The thrid is layerDatas.dat and contains the layerdata for each frame in a canvas. this is stored in layerDatas.dat


// use fs for filesystem and auto-add std
namespace fs = std::filesystem;
using namespace std;

// initalize the FrameRenderer variables
int FrameRenderer::numCanvas = -1;
int FrameRenderer::numFrames = -1;
int FrameRenderer::curCanvas = -1;
int FrameRenderer::curFrame = -1;
// this will be stored in memory so we can access it quickly everything else gets written to a file
// we need the frame data so we can play high fps animations
vector<vector<Color>> FrameRenderer::frames;

// ONLY CALL THIS FUNCTION ONCE!!!
// THIS IS MEANT TO BE MOSTLY STATIC SO YOU CAN REFERENCE THE CLASS ANYWHERE
// CREATING A SECOND FRAME RENDERER WILL BREAK EVERYTHING
// if this is "bad practice" fuck you i dont care
FrameRenderer::FrameRenderer()
{
    fs::create_directories("./frameDatas");
    numCanvas = 0;
    numFrames = 0;
    curCanvas = -1;
    curFrame = -1;
}

// called whenever a new canvas is made
// this saves whatever was on the current canvas (if there was a current canvas)
// this changes the "frames" variable to the proper [empty] pixels values since we changed canvases 
// this also creates a new folder that holds all of the information that is being stored in the new canvas
// creates metaData.dat
// it also creates the file fd1.dat and for every frame that is added the number goes up by 1
void FrameRenderer::newCanvas(Canvas* oldCanvas, Canvas* newCanvas){
    // Save the data if there already was a canvas
    if(numCanvas != 0){
        cout << to_string(oldCanvas->getHeight()) << endl;
        writeAllData(oldCanvas);
    }
    numCanvas++;
    curCanvas = numCanvas;
    numFrames = 1;
    curFrame = 1;

    fs::create_directories("./frameDatas/canvas" + to_string(curCanvas));
    writeAllData(newCanvas);
}

// this function is called whenever you move from one canvas to another
// saves the pixelData and frame data 
void FrameRenderer::updateCanvas(){
    cout << "Canvas Changed" << endl;
}

// create a new frame and save changes to the old frame
// inserts the frame at 1 + curFrame and update numFrames
// updates all frame filenames accordingly
// loads new frame with blank canvas
void FrameRenderer::createFrame(){
    cout << "Creating a new frame" << endl;
}

// remove current frame and update file names accordingly
void FrameRenderer::removeFrame(){
    cout << "removing a frame" << endl;
}

// save the current data to the drive
// load the pixelDatas into the corrrect file
void FrameRenderer::selectFrame(int frameNum){
    if(0 < frameNum && frameNum <= numFrames){
        cout << "Frame #" << frameNum << " Selected" << endl;
    }
    else{
        cout << "Frame outside of bounds" << endl;
    }
}

// removes all of the files after shutdown so you dont eat up storage.
void FrameRenderer::shutdown(){
    fs::remove_all("./frameDatas");
}

//  ----------- setters and getters ---------------
void FrameRenderer::setCurFrame(int newCurFrame){
    if(0 < newCurFrame && newCurFrame <= numFrames){
        curFrame = newCurFrame;
    }
    else{
        cout << "FrameRenderer.cpp line 82: you tried to set \"curFrames\" to something it should not be" << endl;
    }
}

int FrameRenderer::getCurFrame(){
    return curFrame;
}

void FrameRenderer::setnumFrames(int newNumFrames){
    if(0 < newNumFrames){
        numFrames = newNumFrames;
    }
    else{
        cout << "FrameRenderer.cpp line 95: you tried to set \"numFrames\" to something it should not be" << endl;
    }

}

int FrameRenderer::getnumFrames(){
    return numFrames;
}

void FrameRenderer::setCurCanvas(int newCurCanvas){
    if(0 < newCurCanvas && newCurCanvas < numCanvas){
        curCanvas = newCurCanvas;
    }
    else {
        cout << "FrameRenderer.cpp line 105: You tried to set \"curCanvas\" to something it should not be" << endl;
    }
}

int FrameRenderer::getCurCanvas(){
    return curCanvas;
}


// Private functions 
void FrameRenderer::writeAllData(Canvas* canvas){
    writeMetaData(canvas);
    writePixelData(canvas);
    writeLayerData(canvas);
}

void FrameRenderer::writeMetaData(Canvas* canvas){
    //saves the width, hight, number of layers, and number of frames
    ofstream File("./frameDatas/canvas" + to_string(curCanvas) + "/meta.dat");
    int width = canvas->getWidth();
    int height = canvas->getHeight();
    int numLayers = canvas->getNumLayers();

    File.write(reinterpret_cast<const char*>(&width), sizeof(int));
    File.write(reinterpret_cast<const char*>(&height), sizeof(int));
    File.write(reinterpret_cast<const char*>(&numLayers), sizeof(int));  
    File.write(reinterpret_cast<char*>(&numFrames), sizeof(numFrames));
    File.close();
}

void FrameRenderer::writePixelData(Canvas* canvas){
    ofstream File("./frameDatas/canvas" + to_string(curCanvas) + "/frameData.dat");
    for(int i = 0; i < numFrames; i++){
        File.write(
            reinterpret_cast<const char*>(canvas->getData()),
            canvas->getWidth() * canvas->getHeight() * 4); // * 4 since color is unisigned char* 4 times over
    }
    File.close();
}

void FrameRenderer::writeLayerData(Canvas* canvas){
    cout << "not yet implimented" << endl;
}
