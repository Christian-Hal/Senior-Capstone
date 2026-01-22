#include "BrushTool.h"

BrushTool::BrushTool(int w, int h) : width(w), height(h) 
{
    width = 5;
    height = 5;
    
    mask = {
        1, 0, 0, 0, 1,
        0, 1, 0, 1, 0,
        0, 0, 1, 0, 0,
        0, 1, 0, 1, 0,
        1, 0, 0, 0, 1
    };
} 