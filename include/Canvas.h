
#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <glm/glm.hpp>

struct Color {
    unsigned char r, g, b, a;
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

        const std::string getName() const;
        void setName(std::string name);
        
        // pixel manipulation
        void setPixel(int x, int y, const Color& color);
        void blendPixel(int x, int y, const Color& srcColor, float brushAlpha);
        const Color& getPixel(int x, int y) const;

        // layer manipulation
        void createLayer();
        void removeLayer();
        void selectLayer(int layerNum);

        glm::vec2 offset = { 0.0f, 0.0f };
        float zoom = 1.0f;
        float rotation = 0.0f;

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
};