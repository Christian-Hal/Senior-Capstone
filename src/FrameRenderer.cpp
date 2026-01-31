
// included libraries for functionality
#include "FrameRenderer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

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
void FrameRenderer::newCanvas(Canvas* canvas){
    cout << "New Canvas created" << endl;
    fs::create_directories("./frameDatas/canvas" + to_string(numCanvas));
    ofstream File("./frameDatas/canvas0/metaData.dat");
    File.write(
        reinterpret_cast<const char*>(canvas->getData()),
        canvas->getWidth() * canvas->getHeight() * 4); // * 32 since color is unisigned char* 4 times over which is 8 bits each
    File.close();
    numCanvas++;
    curCanvas = numCanvas;
    // now save the data
    

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

