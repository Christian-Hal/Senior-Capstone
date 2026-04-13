
#include <vector>

#include "CanvasManager.h"
#include "FrameRenderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <filesystem>
#include <stb_image.h>
#include <miniz.h>
#include <fstream>



Canvas& CanvasManager::createCanvas(int width, int height, std::string name)
{
    Canvas oldCanvasCopy;
    if (hasActive()) {
        oldCanvasCopy = getActive();
    }

    std::string fixed_name = checkName(name);
    canvases.emplace_back(Canvas(width, height, fixed_name));

    activeCanvasIndex = canvases.size() - 1;

    FrameRenderer::newCanvas(&oldCanvasCopy, &canvases[activeCanvasIndex]);

    canvasChange = true;

    return canvases[activeCanvasIndex];
}


void CanvasManager::closeCanvas(int index)
{
    if (index < 0 || index >= (int)canvases.size())
        return;

    // some of the broken parts of update canvas, explaination further down
    if (index == activeCanvasIndex && hasActive())
    {
        Canvas& active = getActive();
        FrameRenderer::removeOnionSkin(active);
        FrameRenderer::frames[FrameRenderer::curFrame - 1] =
            std::vector<Color>(active.getData(), active.getData() + active.getWidth() * active.getHeight());
    }

    canvases.erase(canvases.begin() + index);

    // Reindex folders   
    reindexFrameFolders(index);

    if (canvases.empty())
    {
        activeCanvasIndex = -1;
        FrameRenderer::reset(); // nuke it bc no active canvas
        canvasChange = true;
        return;
    }

    // fix activeCanvasIndex becuase there are still canvases left
    // if the canvas being closed is the active one chose the next one available
    if (activeCanvasIndex == index)
        activeCanvasIndex = std::min(index, (int)canvases.size() - 1);

    // 
    else if (index < activeCanvasIndex)
        activeCanvasIndex--;

    // 5. Sync FrameRenderer to the new active canvas
    //    Manually set curCanvas to match the (now reindexed) folder
    FrameRenderer::curCanvas = activeCanvasIndex + 1;
    FrameRenderer::curFrame = 1;

    // clear any frames from the previous canvas
    FrameRenderer::frames.clear();

    // populate the now active canvas with any frames it might have had
    // don't full understand everything going on here from frameRenderer but the canvas needed to update but
    // would crash if the update function was used, so i broke it apart until it worked
    Canvas& newActive = getActive();
    int* meta = FrameRenderer::readMetaData(); 
    FrameRenderer::numFrames = meta[3];
    FrameRenderer::frames = FrameRenderer::readPixelData(meta);
    newActive.setPixels(FrameRenderer::frames[0]);
    newActive.setLayerData(FrameRenderer::readLayerData(meta));
    FrameRenderer::updateOnionSkin(newActive);

    // -1 of numCanvas because there is one less now
    FrameRenderer::numCanvas--;

    canvasChange = true;
}



void CanvasManager::reindexFrameFolders(int deletedIndex)
{
    namespace fs = std::filesystem;

    int oldSize = (int)canvases.size() + 1; // erase already happened, so +1
    int folderIndex = 0;

    for (int i = 1; i <= oldSize; i++) // folders are labeled 1 for canvas 1, canvas 2...
    {
        std::string path = "./frameDatas/canvas" + std::to_string(i);

        if (!fs::exists(path))
            continue;

        if (i == deletedIndex + 1) // +1 because folders are 1-indexed
        {
            fs::remove_all(path);
            continue;
        }

        std::string tempPath = "./frameDatas/temp" + std::to_string(folderIndex++);
        fs::rename(path, tempPath);
    }

    for (int i = 0; i < folderIndex; i++)
    {
        fs::rename(
            "./frameDatas/temp" + std::to_string(i),
            "./frameDatas/canvas" + std::to_string(i + 1) // back to 1-indexed
        );
    }
}




int CanvasManager::getActiveCanvasIndex() const
{
    return activeCanvasIndex;
}

void CanvasManager::undo()
{
    if (!hasActive()) {
        return;
    }
    getActive().undo();
}

void CanvasManager::redo()
{
    if (!hasActive()) {
        return;
    }
    getActive().redo();
}

Canvas& CanvasManager::getActive()
{
    return canvases[activeCanvasIndex];
}

bool CanvasManager::hasActive()
{
    return activeCanvasIndex >= 0 && activeCanvasIndex < canvases.size();
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
    if (index < 0 || index >= canvases.size())
        return;

    Canvas oldCanvasCopy;
    if (hasActive()) {
        oldCanvasCopy = getActive();
    }

    activeCanvasIndex = index;

    canvasChange = true;

    FrameRenderer::updateCanvas(&oldCanvasCopy, &getActive(), index);
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
    int width = getActive().getWidth();
    int height = getActive().getHeight();

    std::vector<Color> pixels(width * height);
    std::memcpy(pixels.data(), getActive().getData(), width * height * sizeof(Color));

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
    Canvas& canvas = createCanvas(width, height, fileName);

    // converts data into pixels onto the canvas
    canvas.loadImage(data,1);

    // freeing up memory
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
}

// saving for .ora files 
void CanvasManager::saveORA(const std::string& path)
{
    const int width = getActive().getWidth();
    const int height = getActive().getHeight();
    const int numLayers = getActive().getNumLayers();
    const auto& layers = getActive().getLayerData();

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
    Canvas& canvas = createCanvas(width, height, name);

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