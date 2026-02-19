
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

        // various getter methods
        const std::vector<BrushTool>& getLoadedBrushes();

        // function for generating and grabbing the brush dab
        const std::vector<float> generateBrushDab();

    private:
        // list of all loaded brushes
        std::vector<BrushTool> loaded_Brushes;

        // list of all default brushe paths to load on init
        const std::vector<std::string> defaultBrushPaths = {"circle.gbr", "cross.gbr", "confetti.gbr"};

        // pointer to the active brush
        BrushTool* activeBrush = nullptr;

        // default functions
        void configureAsDefault(BrushTool& brush);

        // helper functions
        uint32_t read_be32(std::ifstream& f);
};