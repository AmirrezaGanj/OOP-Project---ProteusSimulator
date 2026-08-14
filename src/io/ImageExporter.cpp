#include "io/ImageExporter.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cstdlib>

using namespace std;

// ============================================================
// ImageExporter.cpp
// ============================================================


// ---- Constructor ----

ImageExporter::ImageExporter()
    : lastExportPath("")
{
}


// ---- Export methods ----

bool ImageExporter::exportAsPNG(SDL_Renderer* renderer,
                                 const string& filePath)
{
    SDL_Surface* surface = captureRenderer(renderer);

    if (surface == nullptr)
    {
        cout << "ImageExporter: failed to capture renderer pixels." << endl;
        return false;
    }

    int result = IMG_SavePNG(surface, filePath.c_str());
    SDL_FreeSurface(surface);

    if (result != 0)
    {
        cout << "ImageExporter: PNG save failed — " << IMG_GetError() << endl;
        return false;
    }

    lastExportPath = filePath;
    cout << "Circuit exported as PNG: " << filePath << endl;

    openFileWithViewer(filePath);
    return true;
}


bool ImageExporter::exportAsJPG(SDL_Renderer* renderer,
                                 const string& filePath,
                                 int quality)
{
    SDL_Surface* surface = captureRenderer(renderer);

    if (surface == nullptr)
    {
        cout << "ImageExporter: failed to capture renderer pixels." << endl;
        return false;
    }

    // Clamp quality to valid range
    if (quality < 0)   quality = 0;
    if (quality > 100) quality = 100;

    int result = IMG_SaveJPG(surface, filePath.c_str(), quality);
    SDL_FreeSurface(surface);

    if (result != 0)
    {
        cout << "ImageExporter: JPG save failed — " << IMG_GetError() << endl;
        return false;
    }

    lastExportPath = filePath;
    cout << "Circuit exported as JPG: " << filePath << endl;

    openFileWithViewer(filePath);
    return true;
}


bool ImageExporter::exportRegionAsPNG(SDL_Renderer* renderer,
                                       const SDL_Rect& region,
                                       const string& filePath)
{
    SDL_Surface* surface = captureRegion(renderer, region);

    if (surface == nullptr)
    {
        cout << "ImageExporter: failed to capture region." << endl;
        return false;
    }

    int result = IMG_SavePNG(surface, filePath.c_str());
    SDL_FreeSurface(surface);

    if (result != 0)
    {
        cout << "ImageExporter: PNG region save failed — "
             << IMG_GetError() << endl;
        return false;
    }

    lastExportPath = filePath;
    cout << "Circuit region exported as PNG: " << filePath << endl;

    openFileWithViewer(filePath);
    return true;
}


// ---- Post-export viewer ----

void ImageExporter::openFileWithViewer(const string& filePath)
{
    string command = "";

#ifdef __APPLE__
    // macOS
    command = "open \"" + filePath + "\"";

#elif defined(_WIN32) || defined(_WIN64)
    // Windows
    command = "start \"\" \"" + filePath + "\"";

#else
    // Linux
    command = "xdg-open \"" + filePath + "\"";
#endif

    if (!command.empty())
    {
        int ret = system(command.c_str());
        if (ret != 0)
        {
            cout << "ImageExporter: could not open file viewer "
                 << "(file was still saved successfully)." << endl;
        }
    }
}


// ---- Last export info ----

string ImageExporter::getLastExportPath() const
{
    return lastExportPath;
}


// ---- Private helpers ----

SDL_Surface* ImageExporter::captureRenderer(SDL_Renderer* renderer)
{
    // Query the renderer output size
    int width  = 0;
    int height = 0;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    if (width <= 0 || height <= 0)
    {
        cout << "ImageExporter: invalid renderer output size." << endl;
        return nullptr;
    }

    // Create a surface to hold the pixel data
    // SDL_PIXELFORMAT_RGB24 = 3 bytes per pixel, no alpha
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
        0, width, height, 24, SDL_PIXELFORMAT_RGB24
    );

    if (surface == nullptr)
    {
        cout << "ImageExporter: SDL_CreateRGBSurface failed — "
             << SDL_GetError() << endl;
        return nullptr;
    }

    // Read pixels from the renderer into the surface
    int result = SDL_RenderReadPixels(
        renderer,
        nullptr,                    // nullptr = read the full output
        SDL_PIXELFORMAT_RGB24,
        surface->pixels,
        surface->pitch
    );

    if (result != 0)
    {
        cout << "ImageExporter: SDL_RenderReadPixels failed — "
             << SDL_GetError() << endl;
        SDL_FreeSurface(surface);
        return nullptr;
    }

    return surface;
}


SDL_Surface* ImageExporter::captureRegion(SDL_Renderer* renderer,
                                           const SDL_Rect& region)
{
    if (region.w <= 0 || region.h <= 0)
    {
        cout << "ImageExporter: invalid region dimensions." << endl;
        return nullptr;
    }

    // Create a surface matching the region size
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
        0, region.w, region.h, 24, SDL_PIXELFORMAT_RGB24
    );

    if (surface == nullptr)
    {
        cout << "ImageExporter: SDL_CreateRGBSurface failed — "
             << SDL_GetError() << endl;
        return nullptr;
    }

    // Read only the specified rectangle region from the renderer
    int result = SDL_RenderReadPixels(
        renderer,
        &region,
        SDL_PIXELFORMAT_RGB24,
        surface->pixels,
        surface->pitch
    );

    if (result != 0)
    {
        cout << "ImageExporter: SDL_RenderReadPixels (region) failed — "
             << SDL_GetError() << endl;
        SDL_FreeSurface(surface);
        return nullptr;
    }

    return surface;
}