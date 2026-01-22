#pragma once

#include <vector>

class BrushTool{

    public:
    // brush tip settings
    std::vector<int> mask; // defines the 'shape' of the brush
    int width, height;

    // constructor
    BrushTool(int w, int h);

};