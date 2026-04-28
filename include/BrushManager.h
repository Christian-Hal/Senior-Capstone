
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "BrushTool.h"

#include "tinyxml2.h"

class BrushManager 
{
    public:
        // init function that loads in all default brushes
        void init();

        // active brush stuff
        const BrushTool& getActiveBrush();
        void setActiveBrush(int index);
        bool brushChange = false;

        // brush loader method
        void loadBrush(const std::string& path);

        // various getter methods
        const std::vector<BrushTool>& getLoadedBrushes();

        // function for generating and grabbing the brush dab
        // want brush size to be a diameter measurement as opposed to a multiplicativie relationship 
        const std::vector<float> generateBrushDab(int brushSize);

    private:
        // list of all loaded brushes
        std::vector<BrushTool> loaded_Brushes;

        // list of all default brush paths to load on init
        const std::vector<std::string> defaultBrushPaths = {"assets/circle.gbr", "assets/confetti.gbr"}; //"cross.gbr"

        // index of the active brush in loaded_Brushes
        int activeBrushIndex = 0;

        // brush loader methods
        bool loadBrushTipFromPNG(const std::string& path, BrushTool& outBrush); // loads from a file path
        bool loadBrushFromGBR(const std::string& path, BrushTool& outBrush);
        bool loadBrushFromKPP(const std::string& path, BrushTool& outBrush);
        bool loadBrushFromJBR(const std::string& path, BrushTool& outBrush);

        // default functions
        void configureAsDefault(BrushTool& brush);

        // helper functions
        uint32_t read_be32(std::ifstream& f);
        bool extractFile(const std::string& zipPath, const std::string& filename, std::vector<unsigned char>& out);
        bool loadTipFromPNG(const std::vector<unsigned char>& pngData, BrushTool& brush);
        float getParam(tinyxml2::XMLDocument& doc, const char* name, float defaultVal);
};