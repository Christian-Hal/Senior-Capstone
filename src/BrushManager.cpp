
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

#include "BrushManager.h"
#include "stb_image.h"

// init function that loads in all default brushes
void BrushManager::init()
{
    activeBrush = new BrushTool(1,1);

    if (!loadBrushTipFromPNG("BrushTipTest.png", *activeBrush)) {
		configureAsDefault(*activeBrush);
	}
}

// active brush stuff
const BrushTool& BrushManager::getActiveBrush()
{
    return *activeBrush;
}

void BrushManager::setActiveBrush(int index) {}

// brush loader methods
bool BrushManager::loadBrushTipFromPNG(const std::string& path, BrushTool& outBrush)
{
    int width, height, channels;

    // Force RGBA so we don't care about input format
    unsigned char* data = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if (!data) {
        std::cerr << "Failed to load brush tip: " << path << "\n";
        return false;
    }

    outBrush.tipWidth = width;
    outBrush.tipHeight = height;
    outBrush.tipAlpha.resize(width * height);

    // Convert to normalized alpha
    for (int i = 0; i < width * height; ++i) {
        unsigned char r = data[i * 4 + 0];
        unsigned char g = data[i * 4 + 1];
        unsigned char b = data[i * 4 + 2];
        unsigned char a = data[i * 4 + 3];

        // Krita-style: luminance * alpha
        float luminance =
            (0.2126f * r +
                0.7152f * g +
                0.0722f * b) / 255.0f;

        float alpha = (a / 255.0f) * luminance;

        outBrush.tipAlpha[i] = std::clamp(alpha, 0.0f, 1.0f);
    }

    stbi_image_free(data);
    return true;
}

bool BrushManager::loadBrushFromGBR(const std::string& path, BrushTool& outBrush) {}

void BrushManager::configureAsDefault(BrushTool& brush) {

	float cx = (brush.tipWidth - 1) * 0.5f; 
	float cy = (brush.tipHeight - 1) * 0.5f;
	float radius = std::min(cx, cy); 

	for (int y = 0; y < brush.tipHeight; ++y) {
		
		for (int x = 0; x < brush.tipWidth; ++x) {

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