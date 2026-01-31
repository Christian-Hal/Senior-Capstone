

#include <string>

#include "BrushTool.h"



/*
	Creates a brush tip of w x h

	Default initialized alpha values are all 1.0 
	giving a fully opaque brush 
*/
BrushTool::BrushTool(int w, int h, std::string name) :
	tipWidth(w),
	tipHeight(h),
	brushName(name),
	tipAlpha(w* h, 1.0f),
	spacing(0.25f), // 25% of brush size is the distance between stamps
	hardness(1.0f),
	opacity(1.0f),
	rotateWithStroke(false)
{}

BrushTool::BrushTool() : 
	tipWidth(0),
	tipHeight(0),
	brushName("Untitled"),
	tipAlpha(0, 1.0f),
	spacing(0.25f), // 25% of brush size is the distance between stamps
	hardness(1.0f),
	opacity(1.0f),
	rotateWithStroke(false)
{}



/*
	Returns the alpha value at a pair of coordinates 

	If the coordinates are outside of the brush, 
	return 0.0, doing nothing. 
*/
float BrushTool::sampleAlpha(int x, int y) const {
	if (x < 0 || y < 0 || x >= tipWidth || y >= tipHeight)
		return 0.0f;

	return tipAlpha[y * tipWidth + x]; 
}