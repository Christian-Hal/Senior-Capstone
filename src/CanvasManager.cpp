
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
    if (hasActive()) {
        oldCanvasCopy = getActive();
    }

    std::string fixed_name = checkName(name);
    canvases.emplace_back(Canvas(width, height, fixed_name, isAnimation));

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

    if (canvases.empty())
    {
        activeCanvasIndex = -1;
        FrameRenderer::removeCanvas(index); 
        canvasChange = true;
        return;
    }

    // fix activeCanvasIndex becuase there are still canvases left
    // if the canvas being closed is the active one chose the next one available
    if (activeCanvasIndex == index)
        activeCanvasIndex = std::min(index, (int)canvases.size() - 1);

    // for the instance that he canvas being closed was one earlier down the vector with a lower index
    // we dont need an if the index was higher since the activeCanvasIndex would not change
    else if (index < activeCanvasIndex)
        activeCanvasIndex--;

  
    FrameRenderer::removeCanvas(index, &canvases[activeCanvasIndex]);
    canvasChange = true;
}


int CanvasManager::getActiveCanvasIndex() const
{
    return activeCanvasIndex;
}

ImVec4 CanvasManager::getPaperColor()
{
    if (!hasActive()) {
        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // default white
    }
    Color bgColor = getActive().getBackgroundColor();
    return ImVec4(bgColor.r / 255.0f, bgColor.g / 255.0f, bgColor.b / 255.0f, bgColor.a / 255.0f);
}

void CanvasManager::setPaperColor(const ImVec4& color)
{
    if (!hasActive()) {
        return;
    }

    Color bgColor = {
        static_cast<unsigned char>(color.x * 255.0f),
        static_cast<unsigned char>(color.y * 255.0f),
        static_cast<unsigned char>(color.z * 255.0f),
        static_cast<unsigned char>(color.w * 255.0f)
    };

    getActive().setBackgroundColor(bgColor);
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
    if (!mz_zip_writer_init_file(&zip, path.c_str(), 0))
    {
        std::cout << "Failed to create ORA file\n";
        std::filesystem::remove_all("ora_temp");
        return;
    }

    mz_zip_writer_add_file(&zip, "mimetype", "ora_temp/mimetype", NULL, 0, MZ_NO_COMPRESSION);
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

    std::filesystem::create_directory("ora_load");

    int fileNum = mz_zip_reader_get_num_files(&zip);
    for (int i = 0; i < fileNum; i++)
    {
        mz_zip_archive_file_stat file_stat;
        mz_zip_reader_file_stat(&zip, i, &file_stat);

        std::string outPath = "ora_load/" + std::string(file_stat.m_filename);

        if (mz_zip_reader_is_file_a_directory(&zip, i))
            std::filesystem::create_directories(outPath);
        else
        {
            std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());
            mz_zip_reader_extract_to_file(&zip, i, outPath.c_str(), 0);
        }
    }

    mz_zip_reader_end(&zip);

    std::ifstream xml("ora_load/stack.xml");
    std::string line;
    int width = 0, height = 0;

    // store src path + x/y offset per layer
    struct LayerEntry {
        std::string path;
        std::string name;
        int x = 0;
        int y = 0;
    };
    std::vector<LayerEntry> layers;

    auto readAttr = [](const std::string& line, const std::string& attr) -> std::string {
        std::string search = " " + attr + "=\"";
        size_t pos = line.find(search);
        if (pos == std::string::npos) return "";
        pos += search.size();
        size_t end = line.find("\"", pos);
        if (end == std::string::npos) return "";
        return line.substr(pos, end - pos);
        };

    while (std::getline(xml, line))
    {
        if (line.find("<image") != std::string::npos)
        {
            std::string w = readAttr(line, "w");
            std::string h = readAttr(line, "h");
            if (!w.empty()) width = std::stoi(w);
            if (!h.empty()) height = std::stoi(h);
        }

        if (line.find("<layer") != std::string::npos)
        {
            std::string src = readAttr(line, "src");
            std::string x = readAttr(line, "x");
            std::string y = readAttr(line, "y");
            std::string name = readAttr(line, "name");

            if (!src.empty())
            {
                LayerEntry entry;
                entry.path = "ora_load/" + src;
                entry.name = name;
                entry.x = x.empty() ? 0 : std::stoi(x);
                entry.y = y.empty() ? 0 : std::stoi(y);
                layers.push_back(entry);
            }
        }
    }

    xml.close();

    if (width == 0 || height == 0 || layers.empty())
    {
        std::cout << "Invalid ORA file\n";
        std::filesystem::remove_all("ora_load");
        return;
    }

    std::string name = std::filesystem::path(path).stem().string();
    Canvas& canvas = createCanvas(width, height, name, false);

    std::cout << "Layer count after createCanvas: " << canvas.getNumLayers() << "\n";
    std::cout << "Layers in ORA file: " << layers.size() << "\n";

    while (canvas.getNumLayers() < (int)layers.size())
        canvas.createLayer();

    for (int i = 0; i < (int)layers.size(); i++)
    {
        const LayerEntry& entry = layers[i];

        int targetLayer;
        if (entry.name == "Background")
            targetLayer = 0;
        else
            targetLayer = (int)layers.size() - 1 - i;
    }

    for (int i = 0; i < (int)layers.size(); i++)
    {
        int targetLayer = (int)layers.size() - 1 - i;
        const LayerEntry& entry = layers[i];

        int w, h, ch;
        unsigned char* data = stbi_load(entry.path.c_str(), &w, &h, &ch, 4);
        if (!data)
        {
            std::cout << "Warning: failed to load layer: " << entry.path << "\n";
            continue;
        }

        // load into a full-canvas-sized buffer, placing the image at x/y offset
        // ORA y is from top, canvas y is from bottom so we flip it
        std::vector<unsigned char> fullCanvas(width * height * 4, 0);

        for (int row = 0; row < h; row++)
        {
            for (int col = 0; col < w; col++)
            {
                int srcX = col;
                int srcY = row;

                int dstX = entry.x + col;
                // flip y: ORA counts from top, our canvas from bottom
                int dstY = height - 1 - (entry.y + row);

                if (dstX < 0 || dstX >= width || dstY < 0 || dstY >= height)
                    continue;

                int srcIdx = (srcY * w + srcX) * 4;
                int dstIdx = (dstY * width + dstX) * 4;

                fullCanvas[dstIdx + 0] = data[srcIdx + 0];
                fullCanvas[dstIdx + 1] = data[srcIdx + 1];
                fullCanvas[dstIdx + 2] = data[srcIdx + 2];
                fullCanvas[dstIdx + 3] = data[srcIdx + 3];
            }
        }

        stbi_image_free(data);
        canvas.loadImage(fullCanvas.data(), targetLayer);
    }

    stbi_set_flip_vertically_on_load(false);
    std::filesystem::remove_all("ora_load");
}