

#include <string>

#include "BrushTool.h"



/*
	Creates a brush tip of w x h

	Default initialized alpha values are all 1.0 
	giving a fully opaque brush. 

	@param w: The width of the brushtip.
	@param h: The height of the brushtip.
	@param name: The name of the brush. 
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



/*
	Alternative default BrushTool constructor. 

	Consists of default values and takes no parameters.
*/
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
