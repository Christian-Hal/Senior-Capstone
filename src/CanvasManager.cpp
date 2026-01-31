
#include <vector>

#include "CanvasManager.h"
#include "FrameRenderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Canvas& CanvasManager::createCanvas(int width, int height, std::string name)
{
    Canvas oldCanvasCopy;
    if(this->hasActive()){
        oldCanvasCopy = *activeCanvas;
    }
    std::string fixed_name = checkName(name);
    canvases.emplace_back(Canvas(width, height, fixed_name));
    FrameRenderer::newCanvas(&oldCanvasCopy, &canvases.back());
    activeCanvas = &canvases.back();

    canvasChange = true;

    return *activeCanvas;
}

Canvas& CanvasManager::getActive()
{
    return *activeCanvas;
}

bool CanvasManager::hasActive()
{
    return activeCanvas != nullptr;
}

int CanvasManager::getNumCanvases()
{
    return canvases.size();
}

std::string CanvasManager::checkName(std::string name)
{
    int i = 0;

    for (Canvas canvas : canvases) 
    {
        std::string temp = canvas.getName();
        if (temp == name) { i++; }
    }

    if (i > 0) {return (name + "-" + std::to_string(i)); }
    return name;
}

const std::vector<Canvas>& CanvasManager::getOpenCanvases() const
{
    return canvases;
}

void CanvasManager::setActiveCanvas(int index)
{
    if (index > 0 && index < canvases.size()){}
    
    activeCanvas = &canvases[index];
    canvasChange = true;
    FrameRenderer::updateCanvas();
}

// bandaid placement of saving features 
// will look pretty later

void CanvasManager::getFrameData(CanvasManager& canvasManager)
{
    // get the width and height of the canvas
    int saveWidth = canvasManager.getActive().getWidth();
    int saveHeight = canvasManager.getActive().getHeight();

    // checks if the buffer size is not 0
    // when the app first runs these two are initialized to zero as a sort of "file is not open"
    // so this is an easy fix until we get the state system fully set up and can know when a file is or isnt open
    if (saveWidth == 0 || saveHeight == 0)
    {
        return;
    }
    const Color* pixelValues = canvasManager.getActive().getData();

    std::vector<Color> pixels(saveWidth * saveHeight);
    std::memcpy(pixels.data(), pixelValues, saveWidth * saveHeight * sizeof(Color));

    for (int y = 0; y < saveHeight / 2; y++) {
        int opposite = saveHeight - y - 1;
        for (int x = 0; x < saveWidth; x++) {
            std::swap(pixels[y * saveWidth + x], pixels[opposite * saveWidth + x]);
        }
    }
    // read the pixels in the canvas and write them to a png
    std::string imageName = canvasManager.getActive().getName();
    stbi_write_png((imageName + ".png").c_str(), saveWidth, saveHeight, 4, pixels.data(), saveWidth * 4);
}



