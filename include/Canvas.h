
#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <glm/glm.hpp>

struct Color {
    unsigned char r, g, b, a;
};

struct Pixel {
    int index;
    Color before;
    Color after;
};

struct StrokePath {
    std::vector<Pixel> pixels;
    int layerNum; 
};

class Canvas {

    public:
        // constructor
        Canvas();
        Canvas(int w, int h, std::string name);

        // getter methods
        int getWidth() const;
        int getHeight() const;
        int getNumLayers() const;
        int getCurLayer() const;
        const Color* getData() const;
        const std::vector<std::vector<Color>>& getLayerData() const;

        const std::string getName() const;
        void setName(std::string name);
        
        // pixel manipulation
        void setPixel(int x, int y, const Color& color);
        void blendPixel(int x, int y, const Color& srcColor, float brushAlpha);
        const Color& getPixel(int x, int y) const;
        void setPixels(std::vector<Color> newPixels);
        void setLayerData(std::vector<std::vector<Color>> newLayerData);

        // layer manipulation
        void createLayer();
        void removeLayer();
        void selectLayer(int layerNum);

        // rotation, zoom, and offset data for each canvas
        glm::vec2 offset = { 0.0f, 0.0f };
        float zoom = 1.0f;
        float rotation = 0.0f;

        /////// FUNCTIONS FOR THE UNDO AND REDO STUFF
        void beginStrokeRecord();   // sets up a new StrokePath
        void recordPixelChange(int index, const Color& before); // records the pixel into the active stroke
        void endStrokeRecord();     // pushes the activeStroke into the undo stack

        void undo();    // undoes the most recent strokepath and sends it to the redo stack
        void redo();    // redoes the most recent strokepath and sends it to the undo stack
        void resetPixel(int index, const Color color);  // resets the pixel to the given color but doesn't record it into the stroke (only for undo/redo)

        bool canUndo() const;
        bool canRedo() const;

    private:
        // canvas settings
        std::string canvasName;
        int width, height;
        int numLayers;
        int curLayer;
        Color backgroundColor = {255, 255, 255, 255};
        Color emptyColor = {0, 0, 0, 0};

        // RGBA pixel data
        std::vector<Color> pixels;
        std::vector<std::vector<Color>> layerData;

        /////// VARIABLES FOR THE UNDO AND REDO STUFF
        // Stacks for undo and redo strokes
        std::vector<StrokePath> undoStack = {};
        std::vector<StrokePath> redoStack = {};

        // keeps track of the active stroke to record pixels into
        StrokePath activeStroke;

        // seen pixels is a flat vector the size of the canvas that records if an index was seen during this stroke or not
        // currentStrokeIndex is just the int value that seenPixels sets the index too if seen
        // it gets checked for in recordPixelChange
        std::vector<int> seenPixels;
        int currentStrokeIndex;

};