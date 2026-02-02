
// included libraries for functionality
#include "FrameRenderer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <array>


// NOTES - there are 3 types of files in a canvas folder: the metaData file containing the width, hieght, number of layers and number of frames this is stored in meta.dat
//          each value is on a new line and stored in the order previously given
//          The second type of file is the frames[num][data] data, this contains the pixels[] information for each frame in a canvas tihs is stored in framedata.dat
//          The thrid is layerDatas.dat and contains the layerdata for each frame in a canvas. this is stored in layerDatas[i].dat


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
// creates meta.dat
// creates frameData.dat
// creates the layerData[1].dat and for every frame that is added the number goes up by 1
void FrameRenderer::newCanvas(Canvas* oldCanvas, Canvas* newCanvas){
    // Save the data if there already was a canvas
    if(numCanvas != 0){
        frames[curFrame - 1] = vector<Color>(oldCanvas->getData(), oldCanvas->getData() + (oldCanvas->getWidth() * oldCanvas->getHeight()));
        writeAllData(oldCanvas);
    }
    numCanvas++;
    curCanvas = numCanvas;
    numFrames = 1;
    curFrame = 1;

    fs::create_directories("./frameDatas/canvas" + to_string(curCanvas));
    // save the first "frame" to frames
    // if its a new canvas we only have 1 frame, the starting frame
    frames.clear();
    frames.push_back(vector<Color>(newCanvas->getData(), newCanvas->getData() + (newCanvas->getWidth() * newCanvas->getHeight())));
    writeAllData(newCanvas);
}

// this function is called whenever you move from one canvas to another
// saves the pixelData and frame data 
// updates the frames vector with the proper array.
// pulls the correct layerData for the first frame.
// [NOTE] : no matter what frame you were on last visit, you will always go back to frame 1 after changing canvases
void FrameRenderer::updateCanvas(Canvas* oldCanvas, Canvas* newCanvas, int newCanvasIndex){
    
    // Save data
    if(numCanvas != 0){
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
    cout << "wrote to layerData" << endl;


    cout << "Canvas Changed to canvas " << to_string(curCanvas) << endl;
}


// save changes to the old frame
// inserts the frame at 1 + curFrame and update numFrames
// updates all frame filenames accordingly
// loads new frame with blank canvas
void FrameRenderer::createFrame(Canvas& canvas){
    // Save the old frame
    frames[curFrame - 1] =  vector<Color>(canvas.getData(), canvas.getData() + (canvas.getWidth() * canvas.getHeight()));
    writeAllData(&canvas);
    numFrames++;
    curFrame++;
    // insert new frame
    int* meta = readMetaData(); // meta[0] is width, meta[1] is height
    if(numFrames == curFrame){
        frames.insert(frames.end(), vector<Color>(meta[0] * meta[1], {255,255,255,255}));
    }
    else{
        // this ought to insert inbetween the oldCurrent frame
        frames.insert(frames.begin() + curFrame - 1, vector<Color>(meta[0] * meta[1], {255,255,255,255}));
    }
    canvas.setPixels(frames[curFrame - 1]);
    canvas.setLayerData(readLayerData(meta));
    // create function that renames any other frames that come after
    rename(true);
    writeAllData(&canvas);
}

// remove current frame and update file names accordingly
void FrameRenderer::removeFrame(Canvas& canvas){
    cout << "current frame: " << curFrame << endl;
    if(numFrames > 1){
        // erases the frameData
        frames.erase(frames.begin() + curFrame - 1);
        // erases the layerData
        if(!fs::remove("./frameDatas/canvas" + to_string(curCanvas) + "/layerData" + to_string(curFrame) + ".dat")){
            cerr << "File was not removed" << endl;
        }
        // fixes the names of layerdata
        rename(false);
        if(curFrame == numFrames){
            curFrame--;
        }
        numFrames--;
        // saves the information (changed frameData, changed metadata)
        writeAllData(&canvas);
        int* meta = readMetaData();
        canvas.setPixels(frames[curFrame-1]);
        canvas.setLayerData(readLayerData(meta));
    
    }
}

// save the current data to the drive
// load the pixelDatas from the corrrect file to here
// update the canvas with info from the drive
void FrameRenderer::selectFrame(Canvas& canvas, int frameDelta){
    if(0 < curFrame + frameDelta && curFrame + frameDelta <= numFrames){
        // save data to drive
        cout << "Frame #" << to_string(frameDelta + curFrame) << " Selected" << endl;
        frames[curFrame - 1] =  vector<Color>(canvas.getData(), canvas.getData() + (canvas.getWidth() * canvas.getHeight()));
        writeAllData(&canvas);
        curFrame = curFrame + frameDelta;
        int* meta = readMetaData();
        canvas.setPixels(frames[curFrame-1]);
        canvas.setLayerData(readLayerData(meta));
    }
    else{
        cout << "Frame outside of bounds" << endl;
    }
}

// removes all of the files after shutdown so you dont eat up storage.
void FrameRenderer::shutdown(){
    fs::remove_all("./frameDatas");
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

// saves the vector frames so we can access it later in the event we use a different canvas.
// one file for each canvas.
void FrameRenderer::writePixelData(Canvas* canvas){
    ofstream File("./frameDatas/canvas" + to_string(curCanvas) + "/frameData.dat");
    for (auto& frame : frames) {
        size_t frameSize = frame.size();
        File.write(reinterpret_cast<const char*>(frame.data()), frameSize * 4);
    }
    File.close();
}

// saves the layerData for each frame in a canvas. will be written to layerData in canvas every time the frame is updated.
// one file for each frame in a canvas
void FrameRenderer::writeLayerData(Canvas* canvas){
    if(curFrame <= numFrames){
        vector<vector<Color>> frLayerData = canvas->getLayerData();

        ofstream File("./frameDatas/canvas" + to_string(curCanvas) + "/layerData" + to_string(curFrame) + ".dat");
        for (int i = 0; i < frLayerData.size(); i++){
            File.write(reinterpret_cast<const char*>(frLayerData[i].data()), frLayerData[i].size() * 4);
        }
        File.close();
    }
}

// returns a pointer where you can access [width, height, NumLayers, NumFrames]
int* FrameRenderer::readMetaData() {
    static int meta[4];
    // init all values
    for(int i = 0; i < 4; i++){
        meta[i] = 0;
    }

    ifstream File("./frameDatas/canvas" + to_string(curCanvas) + "/meta.dat", ios::binary);
    if (!File) {
        cout << "returning nothing";
        return meta;   // return default {0,0,0,0} if file missing
    }

    File.read(reinterpret_cast<char*>(meta), 4 * sizeof(int));
    return meta;
}

vector<vector<Color>> FrameRenderer::readPixelData(int* arr) {
    int width = arr[0];
    int height = arr[1];
    int numFra = arr[3];

    vector<vector<Color>> readFrames(numFra, vector<Color>(width * height));

    ifstream File("./frameDatas/canvas" + to_string(curCanvas) + "/frameData.dat", ios::binary);
    if (!File){
        return readFrames;
    }

    for (int i = 0; i < numFra; i++) {
        File.read(reinterpret_cast<char*>(readFrames[i].data()), width * height * sizeof(Color));
    }
    return readFrames;
}
    
vector<vector<Color>> FrameRenderer::readLayerData(int* arr){
    int width = arr[0];
    int height = arr[1];
    int numLay = arr[2];

    vector<vector<Color>> returnData(numLay, vector<Color>(width*height));

    string path = "./frameDatas/canvas" + to_string(curCanvas) + "/layerData" + to_string(curFrame) +  ".dat";
    ifstream File(path, ios::binary);
    if (!File){
        return returnData;
    }

    for(int i = 0; i < numLay; i++){
        File.read(reinterpret_cast<char*>(returnData[i].data()), width * height * sizeof(Color));
    }
    return returnData;
}

void FrameRenderer::rename(bool isAdding){
    if(isAdding){
        for(int i = curFrame + 2; i <= numFrames; i++){
            cout << "renaming file" << endl;
            fs::rename(
                "./frameDatas/canvas" + to_string(curCanvas) + "/layerData" + to_string(i) + ".dat",
                "./frameDatas/canvas" + to_string(curCanvas) + "/layerData" + to_string(i+1) + ".dat");
        }
    }
    else{
        for(int i = curFrame; i < numFrames; i++){
            cout << i << endl;
            fs::rename(
                "./frameDatas/canvas" + to_string(curCanvas) + "/layerData" + to_string(i + 1) + ".dat",
                "./frameDatas/canvas" + to_string(curCanvas) + "/layerData" + to_string(i) + ".dat");
            }
        
    }
}
