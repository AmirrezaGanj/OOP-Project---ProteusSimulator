#pragma once

#include <string>
#include <SDL2/SDL.h>

using namespace std;

// ============================================================
// ImageExporter — saves the current canvas as a PNG or JPG.
//
// Uses SDL_RenderReadPixels() to capture the renderer's pixel
// data, then SDL2_image's IMG_SavePNG() / IMG_SaveJPG() to
// write it to disk. After saving, opens the file automatically
// using the OS file viewer (macOS: `open`, Windows: `start`).
//
// The export area is the full renderer output — the entire
// application window. If you want only the canvas portion,
// pass a clipping SDL_Rect to the export methods.
//
// Dependencies:
//   SDL2_image (libSDL2_image) — must be linked in CMakeLists.
// ============================================================

class ImageExporter
{
public:

    ImageExporter();

    // ---- Export methods ----

    // Captures the full renderer output and saves as PNG.
    // filePath: absolute or relative path including filename
    // Returns true on success.
    bool exportAsPNG(SDL_Renderer* renderer,
                     const string& filePath);

    // Captures the full renderer output and saves as JPG.
    // quality: 0-100 (higher = better quality, larger file)
    bool exportAsJPG(SDL_Renderer* renderer,
                     const string& filePath,
                     int quality = 90);

    // Captures only a rectangular region of the renderer.
    // Useful for exporting just the canvas area without UI panels.
    bool exportRegionAsPNG(SDL_Renderer* renderer,
                           const SDL_Rect& region,
                           const string& filePath);


    void openFileWithViewer(const string& filePath);

    // ---- Last export info ----

    // Returns the file path of the most recently exported file.
    // Empty string if nothing has been exported yet.
    string getLastExportPath() const;

private:

    string lastExportPath;

    // Reads all pixels from the renderer into a new SDL_Surface.
    // Caller is responsible for calling SDL_FreeSurface() on result.
    // Returns nullptr on failure.
    SDL_Surface* captureRenderer(SDL_Renderer* renderer);

    // Reads a specific rect region from the renderer into a surface.
    SDL_Surface* captureRegion(SDL_Renderer* renderer,
                                const SDL_Rect& region);
};