#pragma once

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "io/ProjectManager.h"
#include "ui/StartMenuScreen.h"
#include "ui/MainEditorScreen.h"

using namespace std;

// ============================================================
// Application — owns the SDL lifecycle and the main loop.
//
// Responsibilities:
//   1. Initialise SDL, SDL_ttf and SDL_image
//   2. Create the window and renderer
//   3. Load the fonts shared by every UI component
//   4. Create and own the two screens
//   5. Run the main loop: poll events -> update -> draw -> present
//   6. Switch between the start menu and the editor
//   7. Clean everything up on exit
//
// Usage from main.cpp:
//
//     Application app;
//     return app.run();
//
// run() performs initialise -> loop -> shutdown and returns an
// exit code (0 on success, 1 if initialisation failed).
// ============================================================

class Application
{
public:

    Application();
    ~Application();

    // Runs the whole program. Returns the process exit code.
    int run();

private:

    // ---- Window configuration ----
    static const int  WINDOW_WIDTH;
    static const int  WINDOW_HEIGHT;
    static const int  TARGET_FPS;
    static const int  FONT_SIZE;
    static const int  TITLE_FONT_SIZE;

    // ---- Which screen is currently active ----
    enum class ScreenId
    {
        START_MENU,
        EDITOR
    };

    // ---- SDL objects ----
    SDL_Window*   window;
    SDL_Renderer* renderer;

    TTF_Font* font;         
    TTF_Font* titleFont;    

    // ---- Program state ----
    bool     isRunning;
    ScreenId currentScreen;

    // ---- Owned systems ----
    // ProjectManager is shared by both screens, so Application owns it.
    ProjectManager* projectManager;

    StartMenuScreen*  startMenu;
    MainEditorScreen* editor;

    // ---- Lifecycle ----

    // Initialises SDL, creates the window/renderer, loads fonts and
    // builds both screens. Returns false if anything failed.
    bool initialise();

    // The main loop. Runs until isRunning becomes false.
    void mainLoop();

    // Destroys everything in reverse order of creation.
    void shutdown();

    // ---- Frame steps ----

    void processEvents();
    void update();
    void render();

    // ---- Screen transitions ----

    // Reads the start menu's pending action and reacts to it.
    void handleStartMenuAction();

    // Returns to the start menu from the editor.
    void goToStartMenu();

    // ---- Helpers ----

    // Tries the bundled font first, then a few well-known system
    // fonts, so the program still runs if assets/fonts is empty.
    TTF_Font* loadFontWithFallback(int pointSize);

    // Creates the saved_projects directory if it does not exist.
    void ensureSaveDirectoryExists();
};