
#include "BrushTool.h"



/*
	Creates a brush tip of w x h

	Default initialized alpha values are all 1.0 
	giving a fully opaque brush 
*/
BrushTool::BrushTool(int w, int h)
	: width(w),
	height(h),
	alpha(w* h, 1.0f),
	spacing(0.25f), // 25% of brush size is the distance between stamps
	hardness(1.0f),
	opacity(1.0f),
	rotateWithStroke(false)
{
} 



/*
	Returns the alpha value at a pair of coordinates 

	If the coordinates are outside of the brush, 
	return 0.0, doing nothing. 
*/
float BrushTool::sampleAlpha(int x, int y) const {
	if (x < 0 || y < 0 || x >= width || y >= height)
		return 0.0f;

	return alpha[y * width + x]; 
}