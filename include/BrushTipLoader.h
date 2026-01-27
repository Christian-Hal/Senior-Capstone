
#pragma once 
#include <string>

#include "BrushTool.h"

class BrushTipLoader{
public:
	static bool loadBrushTipFromPNG(const std::string& path, BrushTool& outBrush);
};
