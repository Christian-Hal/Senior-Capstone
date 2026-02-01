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
        static void updateCanvas(Canvas* oldCanvas, int newCanvasIndex);
        static void createFrame();
        static void removeFrame();
        static void selectFrame(int frame);
        void shutdown();

        //setters and getters
        static void setCurFrame(int newCurFrame);
        static int getCurFrame();

        static void setNumFrames(int numFrames);
        static int getNumFrames();

        static void setCurCanvas(int newCurCanvas);
        static int getCurCanvas();
    private:
        static int numFrames;
        static int curFrame;
        static int curCanvas;
        static int numCanvas;
        
        // frame number, pixel data for that frame
        static vector<vector<Color>> frames;

        // one layer of layerData to be assigned to the current canvas
        static vector<vector<Color>> frLayerData;

        static void writeAllData(Canvas* canvas);
        static void writeMetaData(Canvas* canvas);
        static void writePixelData(Canvas* canvas);
        static void writeLayerData(Canvas* canvas);

};