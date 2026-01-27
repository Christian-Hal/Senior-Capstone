
#include "BrushTool.h"
#include <cmath> 
#include <algorithm>

/*
	Generating a basic soft round brush tip 

	This is the default for when there is no 
	specified brush tip from a png import 

	It produces alpha data
*/
void generateDefaultRoundBrush(BrushTool& brush) {

	float cx = (brush.tipWidth - 1) * 0.5f; 
	float cy = (brush.tipHeight - 1) * 0.5f;

	for (int y = 0; y < brush.tipHeight; ++y) {
		
		for (int x = 0; x < brush.tipWidth) {

			float dx = x - cx; 
			float dy = y - cy; 
			float dist = std::sqrt(dx * dx + dy * dy); 

			// normalize distance 
			float t = dist / radius; 

			// soft falloff 
			float alpha = 1.0f - t; 

			brush.tipAlpha[y * brush.tipWidth + x] = std::clamp(alpha, 0.0f, 1.0f); 
		}
	}

}

