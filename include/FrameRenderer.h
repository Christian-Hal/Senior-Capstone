#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "Canvas.h"

using namespace std;

class FrameRenderer{
    public:
        FrameRenderer();
        static void newCanvas(Canvas* canvas);
        static void updateCanvas();
        static void createFrame();
        static void removeFrame();
        static void selectFrame(int frame);
        void shutdown();

        //setters and getters
        static void setCurFrame(int newCurFrame);
        static int getCurFrame();

        static void setnumFrames(int numFrames);
        static int getnumFrames();

        static void setCurCanvas(int newCurCanvas);
        static int getCurCanvas();
    private:
        static int numFrames;
        static int curFrame;
        static int curCanvas;
        static int numCanvas;

        // frame number, pixel data for that frame
        static vector<vector<Color>> frames;
        
};