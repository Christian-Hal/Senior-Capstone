
#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <glm/glm.hpp>

struct Color {
    unsigned char r, g, b, a;

    /*
    Equality operator overload for Color datatype. 

    Is true if rgba values are equal for both Colors. 
    */
    bool operator==(const Color& other) const {
        return (r == other.r) && (g == other.g) && (b == other.b)  && (a == other.a);
    }
};

struct Pixel {
    int index;
    Color before;
    Color after;
    bool wasEditedBefore;
    bool wasEditedAfter;
};

struct StrokePath {
	std::vector<Pixel> pixels;
	int layerNum = -1;
};

class Canvas {

public:
	// constructor
	Canvas();
	Canvas(int w, int h, std::string name, bool isAnimation, bool useAnimTemplate);

	// getter methods
	int getWidth() const;
	int getHeight() const;
    
	int getNumLayers() const;
	int getCurLayer() const;
	Color getBackgroundColor() const;
	const Color* getData() const;
	const std::vector<std::vector<Color>>& getLayerData() const;

    friend bool operator!=(const Color& c2, const Color& c1);
    friend Color operator*(const Color& c2, const Color& c1);

	bool colorEquals(const Color& c2, const Color& c1);
	const Color colorTimes(const Color& c2, const Color& c1);

	const std::string getName() const;
	void setName(std::string name);

	// pixel manipulation
	void setPixel(int x, int y, const Color& color);
	void blendPixel(int x, int y, const Color& srcColor, float brushAlpha);
	const Color& getPixel(int x, int y) const;
	void setPixels(std::vector<Color> newPixels);
	void setLayerData(std::vector<std::vector<Color>> newLayerData);
	void recompositePixelsFromLayers();  // Recompute pixel composite from all layers

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
	void loadImage(unsigned char* data, int layerIndex);
    
    void setBackgroundColor(const Color& color); // sets the background color of the canvas

    bool isAnimation() const { return isAnim; }
    bool isUsingAnimTemplate() const { return animationTemplate; }

    bool isDirty = false; // bool to determine if the canvas has been modifiyed without saving

private:
    // canvas settings
    std::string canvasName;
    int width, height;
    int numLayers;
    int curLayer;
    Color backgroundColor = {255, 255, 255, 255};
    Color emptyColor = {0, 0, 0, 0};
    
	// pixel vectors
	std::vector<Color> pixels;
    std::vector<std::vector<Color>> layerData;

    std::vector<bool> editedPixels;

	// seen pixels is a flat vector the size of the canvas that records if an index was seen during this stroke or not
	// currentStrokeIndex is just the int value that seenPixels sets the index too if seen
	// it gets checked for in recordPixelChange
	bool seenPixelsInitialized = false;
	std::vector<int> seenPixels;
	int currentStrokeIndex;
    

	/////// VARIABLES FOR THE UNDO AND REDO STUFF
	// Stacks for undo and redo strokes
	std::vector<StrokePath> undoStack = {};
	std::vector<StrokePath> redoStack = {};

	// keeps track of the active stroke to record pixels into
	StrokePath activeStroke;

    // variable to state if the canvas should be using the animation template or not
    bool isAnim = false;
    bool animationTemplate = false;
};