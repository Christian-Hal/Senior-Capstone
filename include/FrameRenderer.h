#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "Canvas.h"

using namespace std;

class FrameRenderer{
    public:
        FrameRenderer();
        static void newCanvas(Canvas* oldCanvas, Canvas* newCanvas);
        static void updateCanvas(Canvas* oldCanvas, Canvas* newCanvas, int newCanvasIndex);
        static void createFrame(Canvas canvas);
        static void removeFrame(Canvas canvas);
        static void selectFrame(Canvas canvas, int frameDelta);
        void shutdown();

    private:
        static int numFrames;
        static int curFrame;
        static int curCanvas;
        static int numCanvas;
        
        // frame number, pixel data for that frame
        static vector<vector<Color>> frames;

        // one layer of layerData to be assigned to the current canvas
        static vector<vector<Color>> frLayerData;

        // functions that write to files
        static void writeAllData(Canvas* canvas);
        static void writeMetaData(Canvas* canvas);
        static void writePixelData(Canvas* canvas);
        static void writeLayerData(Canvas* canvas);

        // functions that read from files
        static int* readMetaData();
        static vector<vector<Color>> readPixelData(int* arr);
        static vector<vector<Color>> readLayerData(int* arr);

        static void rename(bool isAdding);
};