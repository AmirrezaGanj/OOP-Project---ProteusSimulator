#pragma once

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "core/Circuit.h"
#include "simulation/SimulationClock.h"
#include "simulation/SimulationEngine.h"
#include "simulation/DRCChecker.h"
#include "io/ProjectManager.h"
#include "io/ImageExporter.h"
#include "utils/UndoRedoManager.h"

#include "ui/Canvas.h"
#include "ui/Toolbar.h"
#include "ui/StatusBar.h"
#include "ui/ComponentLibraryPanel.h"
#include "ui/ActiveComponentsPanel.h"
#include "ui/SimulationLogPanel.h"
#include "ui/PropertiesDialog.h"

using namespace std;

// ============================================================
// MainEditorScreen — the integration hub of the whole program.
//
// It owns the circuit and every subsystem that operates on it,
// lays out all the UI panels, and routes events between them.
//
// Per-frame flow driven by Application:
//   1. handleEvent() for every SDL event
//   2. update()      once
//   3. draw()        once
//
// Event priority (highest first):
//   PropertiesDialog (modal) -> Toolbar -> Panels -> Canvas
// ============================================================

class MainEditorScreen
{
public:

    // windowW / windowH: full window size
    // projectManager:    shared with StartMenuScreen, owned by Application
    // titleFont / font:  pre-loaded fonts owned by Application
    MainEditorScreen(int windowW,
                     int windowH,
                     ProjectManager& projectManager,
                     TTF_Font* titleFont,
                     TTF_Font* font);

    // ---- Frame lifecycle ----

    bool handleEvent(const SDL_Event& event);
    void update();
    void draw(SDL_Renderer* renderer);

    // ---- Project lifecycle ----

    // Wipes the circuit and starts a blank project with the given
    // canvas size. Called by Application after StartMenuScreen
    // raises NEW_PROJECT.
    void startNewProject(float canvasW, float canvasH);

    // Loads a project file into the circuit. Returns true on success.
    // Called by Application after StartMenuScreen raises OPEN_PROJECT.
    bool loadProject(const string& filePath);

    // ---- Exit request ----

    // True when the user asked to return to the start menu.
    bool wantsReturnToMenu() const;
    void clearReturnToMenu();

private:

    int windowW;
    int windowH;

    // ---- Core systems (declaration order = construction order) ----

    Circuit          circuit;
    SimulationClock  simClock;
    SimulationEngine simEngine;
    DRCChecker       drcChecker;
    ImageExporter    imageExporter;
    UndoRedoManager  undoManager;

    ProjectManager&  projectManager;   

    TTF_Font* titleFont;               
    TTF_Font* font;                    

    // ---- Layout rectangles ----

    SDL_Rect toolbarRect;
    SDL_Rect libraryRect;
    SDL_Rect activeRect;
    SDL_Rect canvasRect;
    SDL_Rect logRect;
    SDL_Rect statusRect;

    // ---- UI components ----

    Toolbar               toolbar;
    ComponentLibraryPanel libraryPanel;
    ActiveComponentsPanel activePanel;
    SimulationLogPanel    logPanel;
    Canvas                canvas;
    StatusBar             statusBar;
    PropertiesDialog      propertiesDialog;

    // ---- State ----

    // The canvas dimensions chosen at project creation.
    // Stored so they can be reported and saved later.
    float projectCanvasWidth;
    float projectCanvasHeight;

    // Remembers the last component type each panel reported, so we
    // only react when the selection actually changes.
    string lastLibrarySelection;
    string lastActiveSelection;

    // Set when the user presses the EXPORT button. The actual pixel
    // capture must happen during draw(), after the frame is rendered.
    bool pendingImageExport;

    bool returnToMenuRequested;

    // ---- Layout ----

    void computeLayout();

    // ---- Toolbar action dispatch ----

    void handleToolbarAction(Toolbar::ToolbarAction action);

    // ---- Simulation control ----

    // Runs the design rule check, logs every result, and starts the
    // simulation only when no ERROR-level issues were found.
    void tryStartSimulation();

    // ---- Undo / redo ----

    void saveUndoSnapshot();
    void performUndo();
    void performRedo();

    // ---- Panel synchronisation ----

    void syncPanelSelections();
    void syncToolbarState();
    void syncStatusBar();
    void syncLogPanel();

    // ---- Keyboard shortcuts ----

    // Returns true when the shortcut was recognised and consumed.
    bool handleShortcut(const SDL_Event& event);
};