
#include <vector>
#include <iostream>
#include <algorithm>

#include "BrushTipLoader.h"
#include "stb_image.h"

bool BrushTipLoader::loadBrushTipFromPNG(const std::string& path, BrushTool& outBrush)
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
