
#pragma once
#include <vector>

struct Color {
    unsigned char r, g, b, a;
};

class Canvas {

    public:
        // constructor
        Canvas(int w, int h);

        // getter methods
        int getWidth() const;
        int getHeight() const;

        // pixel manipulation
        void setPixel(int x, int y, const Color& color);
        Color& getPixel(int x, int y) const;

    private:
        // canvas settings
        int width, height;
        Color backgroundColor = {255, 255, 255, 255};

        // RGBA pixel data
        std::vector<Color> pixels;
};