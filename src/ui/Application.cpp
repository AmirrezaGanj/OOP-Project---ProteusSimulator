#include "ui/Application.h"

#include <SDL2/SDL_image.h>
#include <iostream>
#include <sys/stat.h>

using namespace std;

// ============================================================
// Application.cpp
// ============================================================

const int Application::WINDOW_WIDTH    = 1400;
const int Application::WINDOW_HEIGHT   = 900;
const int Application::TARGET_FPS      = 60;
const int Application::FONT_SIZE       = 14;
const int Application::TITLE_FONT_SIZE = 30;


// ---- Constructor / Destructor ----

Application::Application()
    : window(nullptr),
      renderer(nullptr),
      font(nullptr),
      titleFont(nullptr),
      isRunning(false),
      currentScreen(ScreenId::START_MENU),
      projectManager(nullptr),
      startMenu(nullptr),
      editor(nullptr)
{
}


Application::~Application()
{
    shutdown();
}


// ---- Public entry point ----

int Application::run()
{
    if (!initialise())
    {
        shutdown();
        return 1;
    }

    mainLoop();
    shutdown();

    return 0;
}


// ---- Initialisation ----

bool Application::initialise()
{
    // ---- SDL core ----
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    {
        cout << "SDL_Init failed: " << SDL_GetError() << endl;
        return false;
    }

    // ---- SDL_ttf ----
    if (TTF_Init() != 0)
    {
        cout << "TTF_Init failed: " << TTF_GetError() << endl;
        return false;
    }

    // ---- SDL_image (needed by ImageExporter for PNG/JPG output) ----
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;

    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags)
    {
        // Not fatal — the program still runs, only export would fail
        cout << "Warning: IMG_Init incomplete: " << IMG_GetError() << endl;
    }

    // ---- Window ----
    window = SDL_CreateWindow(
        "Proteus Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (window == nullptr)
    {
        cout << "SDL_CreateWindow failed: " << SDL_GetError() << endl;
        return false;
    }

    // ---- Renderer ----
    // Accelerated with vsync — vsync alone would cap the frame rate,
    // but we also cap manually so the loop behaves the same if the
    // driver ignores vsync.
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (renderer == nullptr)
    {
        cout << "SDL_CreateRenderer failed: " << SDL_GetError() << endl;
        return false;
    }

    // ---- Fonts ----
    font      = loadFontWithFallback(FONT_SIZE);
    titleFont = loadFontWithFallback(TITLE_FONT_SIZE);

    if (font == nullptr)
    {
        cout << "Warning: no font could be loaded. "
             << "The interface will render without text." << endl;
    }

    // If the large font failed but the small one worked, share it
    if (titleFont == nullptr)
    {
        titleFont = font;
    }

    // ---- Storage directory ----
    ensureSaveDirectoryExists();

    // ---- Owned systems ----
    projectManager = new ProjectManager();

    startMenu = new StartMenuScreen(WINDOW_WIDTH,
                                    WINDOW_HEIGHT,
                                    *projectManager,
                                    titleFont,
                                    font);

    editor = new MainEditorScreen(WINDOW_WIDTH,
                                  WINDOW_HEIGHT,
                                  *projectManager,
                                  titleFont,
                                  font);

    currentScreen = ScreenId::START_MENU;
    isRunning     = true;

    cout << "Proteus Simulator started." << endl;
    return true;
}


// ---- Main loop ----

void Application::mainLoop()
{
    const Uint32 frameDelay = 1000 / TARGET_FPS;

    while (isRunning)
    {
        Uint32 frameStart = SDL_GetTicks();

        processEvents();
        update();
        render();

        // Cap the frame rate so the program does not spin the CPU
        Uint32 frameTime = SDL_GetTicks() - frameStart;

        if (frameTime < frameDelay)
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }
}


// ---- Frame steps ----

void Application::processEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        // ---- Global quit handling ----
        if (event.type == SDL_QUIT)
        {
            isRunning = false;
            return;
        }

        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE)
        {
            isRunning = false;
            return;
        }

        // ---- Route the event to the active screen ----
        if (currentScreen == ScreenId::START_MENU)
        {
            if (startMenu != nullptr)
            {
                startMenu->handleEvent(event);
            }
        }
        else
        {
            if (editor != nullptr)
            {
                editor->handleEvent(event);
            }
        }
    }
}


void Application::update()
{
    if (currentScreen == ScreenId::START_MENU)
    {
        handleStartMenuAction();
        return;
    }

    if (editor != nullptr)
    {
        editor->update();

        if (editor->wantsReturnToMenu())
        {
            editor->clearReturnToMenu();
            goToStartMenu();
        }
    }
}


void Application::render()
{
    // Clear to a neutral dark colour
    SDL_SetRenderDrawColor(renderer, 30, 32, 38, 255);
    SDL_RenderClear(renderer);

    if (currentScreen == ScreenId::START_MENU)
    {
        if (startMenu != nullptr)
        {
            startMenu->draw(renderer);
        }
    }
    else
    {
        if (editor != nullptr)
        {
            editor->draw(renderer);
        }
    }

    SDL_RenderPresent(renderer);
}


// ---- Screen transitions ----

void Application::handleStartMenuAction()
{
    if (startMenu == nullptr)
    {
        return;
    }

    StartMenuScreen::MenuAction action = startMenu->getAction();

    if (action == StartMenuScreen::MenuAction::NONE)
    {
        return;
    }

    if (action == StartMenuScreen::MenuAction::EXIT)
    {
        startMenu->clearAction();
        isRunning = false;
        return;
    }

    if (action == StartMenuScreen::MenuAction::NEW_PROJECT)
    {
        float w = startMenu->getCanvasWidth();
        float h = startMenu->getCanvasHeight();

        startMenu->clearAction();

        if (editor != nullptr)
        {
            editor->startNewProject(w, h);
            currentScreen = ScreenId::EDITOR;
        }

        return;
    }

    if (action == StartMenuScreen::MenuAction::OPEN_PROJECT)
    {
        string path = startMenu->getSelectedProjectPath();

        startMenu->clearAction();

        if (editor != nullptr && !path.empty())
        {
            if (editor->loadProject(path))
            {
                currentScreen = ScreenId::EDITOR;
            }
            else
            {
                // Loading failed — stay on the menu and refresh the
                // list in case the file was moved or deleted
                startMenu->refresh();
            }
        }
    }
}


void Application::goToStartMenu()
{
    currentScreen = ScreenId::START_MENU;

    if (startMenu != nullptr)
    {
        startMenu->refresh();
    }
}


// ---- Helpers ----

TTF_Font* Application::loadFontWithFallback(int pointSize)
{
    // Ordered list of candidates. The bundled font comes first so a
    // properly set-up project always uses the intended typeface.
    const int candidateCount = 6;

    string candidates[candidateCount] =
    {
        "assets/fonts/Roboto-Regular.ttf",
        "assets/fonts/DejaVuSans.ttf",
        "assets/fonts/arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc"
    };

    for (int i = 0; i < candidateCount; i++)
    {
        TTF_Font* loaded = TTF_OpenFont(candidates[i].c_str(), pointSize);

        if (loaded != nullptr)
        {
            cout << "Loaded font: " << candidates[i]
                 << " (size " << pointSize << ")" << endl;
            return loaded;
        }
    }

    cout << "Could not load any font at size " << pointSize << "." << endl;
    return nullptr;
}


void Application::ensureSaveDirectoryExists()
{
    struct stat info;

    if (stat("saved_projects", &info) == 0)
    {
        return;     // already exists
    }

    // 0755 — owner can read/write/execute, others can read/execute
    int result = mkdir("saved_projects");

    if (result == 0)
    {
        cout << "Created saved_projects directory." << endl;
    }
    else
    {
        cout << "Warning: could not create saved_projects directory. "
             << "Saving may fail." << endl;
    }
}


// ---- Shutdown ----

void Application::shutdown()
{
    // Destroy in reverse order of creation

    if (editor != nullptr)
    {
        delete editor;
        editor = nullptr;
    }

    if (startMenu != nullptr)
    {
        delete startMenu;
        startMenu = nullptr;
    }

    if (projectManager != nullptr)
    {
        delete projectManager;
        projectManager = nullptr;
    }

    // titleFont may be the same pointer as font — only close it once
    if (titleFont != nullptr && titleFont != font)
    {
        TTF_CloseFont(titleFont);
    }
    titleFont = nullptr;

    if (font != nullptr)
    {
        TTF_CloseFont(font);
        font = nullptr;
    }

    if (renderer != nullptr)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window != nullptr)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    IMG_Quit();

    if (TTF_WasInit())
    {
        TTF_Quit();
    }

    SDL_Quit();
}