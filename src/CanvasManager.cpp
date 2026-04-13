
#include <vector>

#include "CanvasManager.h"
#include "FrameRenderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <filesystem>
#include <stb_image.h>
#include <miniz.h>
#include <fstream>



Canvas& CanvasManager::createCanvas(int width, int height, std::string name, bool isAnimation)
{
    Canvas oldCanvasCopy;
    if(this->hasActive()){
        oldCanvasCopy = *activeCanvas;
    }
    std::string fixed_name = checkName(name);
    canvases.emplace_back(Canvas(width, height, fixed_name, isAnimation));
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
    bool isDuplicate = false;
    for (Canvas canvas : canvases) 
    {
            std::string temp = canvas.getName();
            if (temp == name) { isDuplicate = true; }
    }
    if(!isDuplicate){
        return name;
    }
    while(true){
        isDuplicate = true;
        i++;
        for(Canvas canvas : canvases){
            isDuplicate = false;
            if(canvas.getName() == (name + "-" + std::to_string(i))){
                isDuplicate = true;
                break;
            }
        }
        if(!isDuplicate){
            return (name + "-" + std::to_string(i));
        }
    }
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



void savingFlip(int height, int width, std::vector<Color> &pixels)
{
    for (int y = 0; y < height / 2; y++)
    {
        int opposite = height - y - 1;

        for (int x = 0; x < width; x++)
        {
            std::swap(pixels[y * width + x], pixels[opposite * width + x]);
        }
    }
}

// didn't change the main "saving" fucntion of it just implemented it to work with new file system
// works for png and jpg
void CanvasManager::saveToFile(const std::string& path)
{
    int width = activeCanvas->getWidth();
    int height = activeCanvas->getHeight();

    std::vector<Color> pixels(width * height);
    std::memcpy(pixels.data(), activeCanvas->getData(), width * height * sizeof(Color));

    // flip image vertically
    savingFlip(height, width, pixels);

    std::string ext = path.substr(path.find_last_of('.') + 1);

    if (ext == "png")
        stbi_write_png(path.c_str(), width, height, 4, pixels.data(), width * 4);
    
    else if (ext == "jpg")
        stbi_write_jpg(path.c_str(), width, height, 4, pixels.data(), 100);
    
}

// loading function for png/jpg
void CanvasManager::loadFromFile(const std::string& filePath)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 4);

    // getting ride of the path to the file to get the name
    std::filesystem::path pathObj(filePath);
    std::string fileName = pathObj.stem().string();

    // creating new canvas
    Canvas& canvas = createCanvas(width, height, fileName, false);

    // converts data into pixels onto the canvas
    canvas.loadImage(data,1);

    // freeing up memory
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
}

// saving for .ora files 
void CanvasManager::saveORA(const std::string& path)
{
    const int width = activeCanvas->getWidth();
    const int height = activeCanvas->getHeight();
    const int numLayers = activeCanvas->getNumLayers();
    const auto& layers = activeCanvas->getLayerData();

    std::filesystem::create_directory("ora_temp");
    std::filesystem::create_directory("ora_temp/data");
    
    // saving each layer as their own png
    for (int i = 0; i < numLayers; i++)
    {
        std::vector<Color> flipped = layers[i];

        savingFlip(height, width, flipped);

        std::string filename = "ora_temp/data/layer" + std::to_string(i) + ".png";

        stbi_write_png(filename.c_str(), width, height, 4, flipped.data(), width * 4);
    }

    // saving regular png for cover of image
    saveToFile("ora_temp/mergedimage.png");

    // writing the stack.xml 
    std::ofstream xml("ora_temp/stack.xml");

    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<image w=\"" << width << "\" h=\"" << height << "\" "
        << "src=\"mergedimage.png\">\n";
    xml << "  <stack>\n";

    // reverse order for ORA
    for (int i = numLayers - 1; i >= 0; i--)
    {
        xml << "    <layer "
            << "name=\"" << (i == 0 ? "Background" : "Layer " + std::to_string(i)) << "\" "
            << "src=\"data/layer" << i << ".png\" "
            << "x=\"0\" y=\"0\" "
            << "opacity=\"1.0\" "
            << "visibility=\"visible\"/>\n";
    }

    xml << "  </stack>\n</image>";
    xml.close();

    
    std::ofstream mime("ora_temp/mimetype");
    mime << "image/openraster";
    mime.close();

    
    mz_zip_archive zip{};
    mz_zip_writer_init_file(&zip, path.c_str(), 0);

    mz_zip_writer_add_file(&zip, "mimetype", "ora_temp/mimetype", NULL, 0, 0);
    mz_zip_writer_add_file(&zip, "stack.xml", "ora_temp/stack.xml", NULL, 0, 0);
    mz_zip_writer_add_file(&zip, "mergedimage.png", "ora_temp/mergedimage.png", NULL, 0, 0);

    for (int i = 0; i < numLayers; i++)
    {
        std::string src = "ora_temp/data/layer" + std::to_string(i) + ".png";
        std::string dst = "data/layer" + std::to_string(i) + ".png";

        mz_zip_writer_add_file(&zip, dst.c_str(), src.c_str(), NULL, 0, 0);
    }

    mz_zip_writer_finalize_archive(&zip);
    mz_zip_writer_end(&zip);

    std::filesystem::remove_all("ora_temp");
}


void CanvasManager::loadORA(const std::string& path)
{
    mz_zip_archive zip{};

    if (!mz_zip_reader_init_file(&zip, path.c_str(), 0))
    {
        std::cout << "Failed to open ORA file\n";
        return;
    }

    // creating temporary folder for files
    std::filesystem::create_directory("ora_load");

    // extracing the files from the zip
    int fileNum = mz_zip_reader_get_num_files(&zip);

    for (int i = 0; i < fileNum; i++)
    {
        mz_zip_archive_file_stat file_stat;
        mz_zip_reader_file_stat(&zip, i, &file_stat);

        std::string outPath = "ora_load/" + std::string(file_stat.m_filename);

        if (mz_zip_reader_is_file_a_directory(&zip, i))
        {
            std::filesystem::create_directories(outPath);
        }
        else
        {
            std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());
            mz_zip_reader_extract_to_file(&zip, i, outPath.c_str(), 0);
        }
    }

    mz_zip_reader_end(&zip);

    // reading stack.xml file
    std::ifstream xml("ora_load/stack.xml");

    std::string line;
    int width = 0, height = 0;
    std::vector<std::string> layerPaths;

    while (std::getline(xml, line))
    {
        if (line.find("<image") != std::string::npos)
        {
            size_t wPos = line.find("w=\"");
            size_t hPos = line.find("h=\"");

            if (wPos != std::string::npos && hPos != std::string::npos)
            {
                width = std::stoi(line.substr(wPos + 3, line.find("\"", wPos + 3) - (wPos + 3)));
                height = std::stoi(line.substr(hPos + 3, line.find("\"", hPos + 3) - (hPos + 3)));
            }
        }

        if (line.find("<layer") != std::string::npos)
        {
            size_t pos = line.find("src=\"");
            if (pos != std::string::npos)
            {
                pos += 5;
                size_t end = line.find("\"", pos);
                layerPaths.push_back(line.substr(pos, end - pos));
            }
        }
    }

    xml.close();

    if (width == 0 || height == 0 || layerPaths.empty())
    {
        std::cout << "Invalid ORA file\n";
        return;
    }

    std::string name = std::filesystem::path(path).stem().string();
    Canvas& canvas = createCanvas(width, height, name, false);

    // creating correct number of layers
    while (canvas.getNumLayers() < layerPaths.size())
    {
        canvas.createLayer();
    }

    stbi_set_flip_vertically_on_load(true);

    // loading the layer images into the layers
    for (int i = 0; i < layerPaths.size(); i++)
    {
        int targetLayer = layerPaths.size() - 1 - i;
        std::string fullPath = "ora_load/" + layerPaths[i];

        int w, h, ch;
        unsigned char* data = stbi_load(fullPath.c_str(), &w, &h, &ch, 4);

        canvas.loadImage(data, targetLayer);
        stbi_image_free(data);
    }

    stbi_set_flip_vertically_on_load(false);

    std::filesystem::remove_all("ora_load");
}
