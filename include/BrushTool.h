
#pragma once

#include <vector>
#include <string>



/*
    BrushTool is all the components which make up a brush.

    width, height: resolution of the brush tip.
    alpha[]: grayscale opacity values.

    This describes HOW drawing should happen.

*/
class BrushTool{

public:

    // ----- Brush tip settings ----- 
    // note: from Krita these are the gbr/png files

    int tipWidth;                       // width of brush tip bitmap
    int tipHeight;                      // height of brush tip bitmap 

    std::vector<float> tipAlpha;        // grayscale bitmap alpha values

    // ----- Brush parameter metadata -----
    // note: these can be default or imported from Krita 

    float spacing;                      // distance between stamps
    float hardness;                     // 0->soft 1->hard
    float opacity;                      // overall brush opacity 
    bool rotateWithStroke;              // brush rotation 
    std::string name;                   // tool name 

    // constructor
    BrushTool(int w, int h);
    BrushTool();

    // sample the alpha value at a pixel 
    float sampleAlpha(int x, int y) const;

};