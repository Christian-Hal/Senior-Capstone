
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

void CanvasManager::undo()
{
    if (!activeCanvas) {
        return;
    }
    activeCanvas->undo();
}

void CanvasManager::redo()
{
    if (!activeCanvas) {
        return;
    }
    activeCanvas->redo();
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
    if (index < 0 || static_cast<size_t>(index) >= canvases.size()) {
        return;
    }
    
    Canvas oldCanvasCopy;
    if(this->hasActive()){
        oldCanvasCopy = *activeCanvas;
    }
    activeCanvas = &canvases[index];
    canvasChange = true;
    FrameRenderer::updateCanvas(&oldCanvasCopy, activeCanvas, index);
}

// didn't change the main "saving" fucntion of it just implemented it to work with new file system
void CanvasManager::saveToFile(const std::string& path)
{
    int saveWidth = activeCanvas->getWidth();
    int saveHeight = activeCanvas->getHeight();

    const Color* pixelValues = activeCanvas->getData();

    std::vector<Color> pixels(saveWidth * saveHeight);
    std::memcpy(pixels.data(), pixelValues, saveWidth * saveHeight * sizeof(Color));

    // flip image vertically
    for (int y = 0; y < saveHeight / 2; y++)
    {
        int opposite = saveHeight - y - 1;

        for (int x = 0; x < saveWidth; x++)
        {
            std::swap(pixels[y * saveWidth + x], pixels[opposite * saveWidth + x]
            );
        }
    }

    std::string ext = path.substr(path.find_last_of('.') + 1);

    if (ext == "png")
        stbi_write_png(path.c_str(), saveWidth, saveHeight, 4, pixels.data(), saveWidth * 4);
    
    else if (ext == "jpg")
        stbi_write_jpg(path.c_str(), saveWidth, saveHeight, 4, pixels.data(), 100);
    
}



