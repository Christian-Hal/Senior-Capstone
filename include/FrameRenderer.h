#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "Canvas.h"

using namespace std;

class FrameRenderer{
    public:
        FrameRenderer();
        // static void changeOnionSkinColor(Color colorPrev, Color& colorNext);
        static void changeOnionSkinsSeen(int numPrev, int numNext);
        static void changeFPS(int fps);
        static void newCanvas(Canvas* oldCanvas, Canvas* newCanvas);
        static void updateCanvas(Canvas* oldCanvas, Canvas* newCanvas, int newCanvasIndex);
        static void createFrame(Canvas& canvas);
        static void removeFrame(Canvas& canvas);
        static void selectFrame(Canvas& canvas, int frameDelta);
        static void reorderFrame(Canvas& canvas, int frameOne, int frameTwo);
        static void play(Canvas& canvas);
        static void updateOnionSkin(Canvas& canvas);
        static void removeOnionSkin(Canvas& canvas);
        static void toggleOnionSkin();
        static void saveAnimation(const string& path, Canvas& canvas);
        static void loadAnimation(Canvas& canvas, std::vector<std::filesystem::path> images);
        static bool inputBlocked;
        static int getCurFrame();
        static int getNumFrames();
        static int getNumAfter();
        static void setNumAfter(int newNumAfter);
        static int getNumBefore();
        static void setNumBefore(int newNumBefore);
        static long long timeFunction(const string& name, const function<void()>& fn);

        // frame number, pixel data for that frame
        static vector<vector<Color>> frames;
        static int numFrames;
        static int curFrame;
        static int curCanvas;
        static int numCanvas;
        static void reset();
        
        static bool isPlaying;
        static int numBefore;
        static int numAfter;
        static int fps;
        static bool onionSkinEnabled;

        // functions that read from files
        static int* readMetaData();
        static vector<vector<Color>> readPixelData(int* arr);
        static vector<vector<Color>> readLayerData(int* arr);

        static void removeCanvas(int index, Canvas* newActiveCanvas = nullptr);
        static void copyFrame(Canvas& canvas);
    private:

        // create the data structures we will use to store all this information.
        static vector<vector<vector<vector<Color>>>> frLayerData;
        static vector<vector<vector<Color>>> frameData;
        static vector<array<int, 4>> metaData;

        // functions that write to files
        static void writeAllData(Canvas* canvas);
        static void writeMetaData(Canvas* canvas);
        static void writePixelData(Canvas* canvas);
        static void writeLayerData(Canvas* canvas);

        

        static void rename(bool isAdding);
};