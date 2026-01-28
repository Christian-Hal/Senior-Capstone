
#pragma once

#include <string>

#include "BrushTool.h"

class BrushManager 
{
    public:
    // init function that loads in all default brushes
    void init();

    // active brush stuff
    const BrushTool& getActiveBrush();
    void setActiveBrush(int index);
    bool brushChange = false;

    // brush loader methods
    bool loadBrushTipFromPNG(const std::string& path, BrushTool& outBrush);
    bool loadBrushFromGBR(const std::string& path, BrushTool& outBrush);

    private:
    // list of all loaded brushes
    std::vector<BrushTool> canvases;

    // pointer to the active brush
    BrushTool* activeBrush = nullptr;

    // default functions
    void configureAsDefault(BrushTool& brush);

    // helper functions
    uint32_t read_be32(std::ifstream& f);
};