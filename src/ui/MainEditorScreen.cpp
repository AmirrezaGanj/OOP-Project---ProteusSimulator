#include "ui/MainEditorScreen.h"

using namespace std;

// ============================================================
// MainEditorScreen.cpp
// ============================================================

// ---- Layout constants ----
static const int TOOLBAR_H = 44;
static const int STATUS_H  = 26;
static const int LEFT_W    = 240;
static const int LOG_H     = 160;
static const int ACTIVE_H  = 190;


// ============================================================
// Layout helpers
//
// These are free functions rather than member functions so they
// can be used inside the constructor's initialiser list, where
// the UI components must already receive their final rectangles.
// ============================================================

static SDL_Rect makeToolbarRect(int windowW, int windowH)
{
    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = windowW;
    r.h = TOOLBAR_H;
    return r;
}

static SDL_Rect makeLibraryRect(int windowW, int windowH)
{
    int contentH = windowH - TOOLBAR_H - STATUS_H;

    SDL_Rect r;
    r.x = 0;
    r.y = TOOLBAR_H;
    r.w = LEFT_W;
    r.h = contentH - ACTIVE_H;
    return r;
}

static SDL_Rect makeActiveRect(int windowW, int windowH)
{
    int contentH  = windowH - TOOLBAR_H - STATUS_H;
    int libraryH  = contentH - ACTIVE_H;

    SDL_Rect r;
    r.x = 0;
    r.y = TOOLBAR_H + libraryH;
    r.w = LEFT_W;
    r.h = ACTIVE_H;
    return r;
}

static SDL_Rect makeCanvasRect(int windowW, int windowH)
{
    int contentH = windowH - TOOLBAR_H - STATUS_H;

    SDL_Rect r;
    r.x = LEFT_W;
    r.y = TOOLBAR_H;
    r.w = windowW - LEFT_W;
    r.h = contentH - LOG_H;
    return r;
}

static SDL_Rect makeLogRect(int windowW, int windowH)
{
    int contentH = windowH - TOOLBAR_H - STATUS_H;
    int canvasH  = contentH - LOG_H;

    SDL_Rect r;
    r.x = LEFT_W;
    r.y = TOOLBAR_H + canvasH;
    r.w = windowW - LEFT_W;
    r.h = LOG_H;
    return r;
}

static SDL_Rect makeStatusRect(int windowW, int windowH)
{
    SDL_Rect r;
    r.x = 0;
    r.y = windowH - STATUS_H;
    r.w = windowW;
    r.h = STATUS_H;
    return r;
}


// ---- Constructor ----

MainEditorScreen::MainEditorScreen(int windowW,
                                    int windowH,
                                    ProjectManager& projectManager,
                                    TTF_Font* titleFont,
                                    TTF_Font* font)
    : windowW(windowW),
      windowH(windowH),
      circuit(),
      simClock(1.0),
      simEngine(circuit, simClock),
      drcChecker(),
      imageExporter(),
      undoManager(),
      projectManager(projectManager),
      titleFont(titleFont),
      font(font),
      toolbarRect(makeToolbarRect(windowW, windowH)),
      libraryRect(makeLibraryRect(windowW, windowH)),
      activeRect(makeActiveRect(windowW, windowH)),
      canvasRect(makeCanvasRect(windowW, windowH)),
      logRect(makeLogRect(windowW, windowH)),
      statusRect(makeStatusRect(windowW, windowH)),
      toolbar(toolbarRect, font),
      libraryPanel(libraryRect, font),
      activePanel(activeRect, font),
      logPanel(logRect, font),
      canvas(canvasRect, circuit, simClock, font),
      statusBar(statusRect, font),
      propertiesDialog(windowW, windowH, font),
      projectCanvasWidth(1188.0f),
      projectCanvasHeight(840.0f),
      lastLibrarySelection(""),
      lastActiveSelection(""),
      pendingImageExport(false),
      returnToMenuRequested(false)
{
    // Seed the undo history with the current (empty) circuit so the
    // user can undo all the way back to a blank canvas.
    undoManager.initialize(projectManager.serializeCircuit(circuit));

    simEngine.log("Editor ready. Place components to begin.");
}


// ---- Layout (kept for potential window resizing) ----

void MainEditorScreen::computeLayout()
{
    toolbarRect = makeToolbarRect(windowW, windowH);
    libraryRect = makeLibraryRect(windowW, windowH);
    activeRect  = makeActiveRect(windowW, windowH);
    canvasRect  = makeCanvasRect(windowW, windowH);
    logRect     = makeLogRect(windowW, windowH);
    statusRect  = makeStatusRect(windowW, windowH);
}


// ---- Event handling ----

bool MainEditorScreen::handleEvent(const SDL_Event& event)
{
    // 1. The properties dialog is modal — while it is open it
    //    swallows every event so nothing behind it can react.
    if (propertiesDialog.isOpen())
    {
        return propertiesDialog.handleEvent(event);
    }

    // 2. Keyboard shortcuts (Ctrl+S / Ctrl+Z / Ctrl+Y ...)
    if (handleShortcut(event))
    {
        return true;
    }

    // 3. Toolbar
    if (toolbar.handleEvent(event))
    {
        return true;
    }

    // 4. Side panels
    if (libraryPanel.handleEvent(event))
    {
        return true;
    }

    if (activePanel.handleEvent(event))
    {
        return true;
    }

    // 5. Log panel
    if (logPanel.handleEvent(event))
    {
        return true;
    }

    // 6. Canvas gets whatever is left
    return canvas.handleEvent(event);
}


bool MainEditorScreen::handleShortcut(const SDL_Event& event)
{
    if (event.type != SDL_KEYDOWN)
    {
        return false;
    }

    SDL_Keymod mods    = SDL_GetModState();
    bool       ctrlish = (mods & KMOD_CTRL) != 0 || (mods & KMOD_GUI) != 0;

    if (!ctrlish)
    {
        return false;
    }

    SDL_Keycode key = event.key.keysym.sym;

    // Ctrl+S — save
    if (key == SDLK_s)
    {
        handleToolbarAction(Toolbar::ToolbarAction::SAVE);
        return true;
    }

    // Ctrl+Z — undo, Ctrl+Shift+Z — redo
    if (key == SDLK_z)
    {
        if ((mods & KMOD_SHIFT) != 0)
        {
            performRedo();
        }
        else
        {
            performUndo();
        }
        return true;
    }

    // Ctrl+Y — redo
    if (key == SDLK_y)
    {
        performRedo();
        return true;
    }

    // Ctrl+E — export image
    if (key == SDLK_e)
    {
        handleToolbarAction(Toolbar::ToolbarAction::EXPORT_IMAGE);
        return true;
    }

    return false;
}


// ---- Update ----

void MainEditorScreen::update()
{
    // 1. Advance the simulation (does nothing when paused/stopped)
    simEngine.update();

    // 2. Dispatch any toolbar button press
    Toolbar::ToolbarAction action = toolbar.getLastAction();

    if (action != Toolbar::ToolbarAction::NONE)
    {
        handleToolbarAction(action);
        toolbar.clearLastAction();
    }

    // 3. Move panel selections into the canvas placement slot
    syncPanelSelections();

    // 4. The user asked to pin the library selection to the
    //    active-components panel
    if (activePanel.wantsAddFromLibrary())
    {
        string type = libraryPanel.getSelectedComponentType();

        if (!type.empty())
        {
            activePanel.addComponent(type, type);
            simEngine.log("Added " + type + " to active components.");
        }
        else
        {
            simEngine.log("Select a component in the library first.");
        }

        activePanel.clearAddRequest();
    }

    // 5. The canvas changed the circuit — record an undo snapshot
    if (canvas.wasCircuitChanged())
    {
        saveUndoSnapshot();
        canvas.clearCircuitChanged();
    }

    // 6. The canvas requested the properties dialog
    Component* toEdit = canvas.getComponentToEdit();

    if (toEdit != nullptr)
    {
        propertiesDialog.open(toEdit);
        canvas.clearComponentToEdit();
    }

    // 7. The dialog was confirmed — the component changed
    if (propertiesDialog.wasAccepted())
    {
        saveUndoSnapshot();
        propertiesDialog.clearAccepted();
        simEngine.log("Component properties updated.");
    }

    // 8. Keep the UI in sync with the underlying state
    syncToolbarState();
    syncStatusBar();
    syncLogPanel();
}


// ---- Toolbar dispatch ----

void MainEditorScreen::handleToolbarAction(Toolbar::ToolbarAction action)
{
    if (action == Toolbar::ToolbarAction::SELECT_TOOL)
    {
        canvas.setToolMode(Toolbar::ToolMode::SELECT);
        return;
    }

    if (action == Toolbar::ToolbarAction::WIRE_TOOL)
    {
        canvas.setToolMode(Toolbar::ToolMode::WIRE);

        // Wiring and placing at the same time makes no sense
        canvas.setPendingComponentType("");
        libraryPanel.clearSelection();
        activePanel.clearSelection();
        lastLibrarySelection = "";
        lastActiveSelection  = "";
        return;
    }

    if (action == Toolbar::ToolbarAction::RUN)
    {
        tryStartSimulation();
        return;
    }

    if (action == Toolbar::ToolbarAction::PAUSE)
    {
        simEngine.pause();
        return;
    }

    if (action == Toolbar::ToolbarAction::STOP)
    {
        simEngine.stop();
        return;
    }

    if (action == Toolbar::ToolbarAction::STEP)
    {
        simEngine.stepOnce();
        return;
    }

    if (action == Toolbar::ToolbarAction::UNDO)
    {
        performUndo();
        return;
    }

    if (action == Toolbar::ToolbarAction::REDO)
    {
        performRedo();
        return;
    }

    if (action == Toolbar::ToolbarAction::SAVE)
    {
        // ProjectManager prompts on the console the first time
        bool saved = projectManager.save(circuit);

        if (saved)
        {
            simEngine.log("Project saved: " +
                          projectManager.getCurrentProjectName());
        }
        else
        {
            simEngine.log("Save cancelled or failed.");
        }

        return;
    }

    if (action == Toolbar::ToolbarAction::EXPORT_IMAGE)
    {
        // The capture must happen after the frame has been rendered,
        // so we only raise a flag here and act on it inside draw().
        pendingImageExport = true;
        return;
    }
}


// ---- Simulation control ----

void MainEditorScreen::tryStartSimulation()
{
    // Resuming from pause needs no re-check
    if (simClock.isPaused())
    {
        simEngine.run();
        return;
    }

    // Fresh start — validate the design first
    vector<DRCChecker::DRCResult> results = drcChecker.runChecks(circuit);

    for (int i = 0; i < (int)results.size(); i++)
    {
        string prefix =
            (results[i].severity == DRCChecker::Severity::ERROR)
            ? "ERROR: "
            : "WARNING: ";

        simEngine.log(prefix + results[i].message);
    }

    if (drcChecker.hasErrors())
    {
        simEngine.log("Simulation blocked. Fix the errors above and try again.");
        return;
    }

    if (results.empty())
    {
        simEngine.log("Design rule check passed with no issues.");
    }

    simEngine.run();
}


// ---- Undo / redo ----

void MainEditorScreen::saveUndoSnapshot()
{
    undoManager.saveSnapshot(projectManager.serializeCircuit(circuit));
}


void MainEditorScreen::performUndo()
{
    if (!undoManager.canUndo())
    {
        simEngine.log("Nothing to undo.");
        return;
    }

    // Editing history while the simulation runs would desync the
    // engine state, so stop first.
    if (!simClock.isStopped())
    {
        simEngine.stop();
    }

    string snapshot = undoManager.undo();

    canvas.clearSelection();
    projectManager.restoreCircuitFromSnapshot(circuit, snapshot);

    simEngine.log("Undo.");
}


void MainEditorScreen::performRedo()
{
    if (!undoManager.canRedo())
    {
        simEngine.log("Nothing to redo.");
        return;
    }

    if (!simClock.isStopped())
    {
        simEngine.stop();
    }

    string snapshot = undoManager.redo();

    canvas.clearSelection();
    projectManager.restoreCircuitFromSnapshot(circuit, snapshot);

    simEngine.log("Redo.");
}


// ---- Panel synchronisation ----

void MainEditorScreen::syncPanelSelections()
{
    // Only react when a selection actually changes, otherwise the two
    // panels would fight each other for the canvas placement slot
    // on every single frame.

    string librarySel = libraryPanel.getSelectedComponentType();
    string activeSel  = activePanel.getSelectedComponentType();

    if (librarySel != lastLibrarySelection)
    {
        lastLibrarySelection = librarySel;

        if (!librarySel.empty())
        {
            canvas.setPendingComponentType(librarySel);
            canvas.setToolMode(Toolbar::ToolMode::SELECT);
            toolbar.setToolMode(Toolbar::ToolMode::SELECT);

            activePanel.clearSelection();
            lastActiveSelection = "";
        }

        return;
    }

    if (activeSel != lastActiveSelection)
    {
        lastActiveSelection = activeSel;

        if (!activeSel.empty())
        {
            canvas.setPendingComponentType(activeSel);
            canvas.setToolMode(Toolbar::ToolMode::SELECT);
            toolbar.setToolMode(Toolbar::ToolMode::SELECT);

            libraryPanel.clearSelection();
            lastLibrarySelection = "";
        }
    }
}


void MainEditorScreen::syncToolbarState()
{
    toolbar.setCanUndo(undoManager.canUndo());
    toolbar.setCanRedo(undoManager.canRedo());

    toolbar.setSimRunning(!simClock.isStopped());
    toolbar.setSimPaused(simClock.isPaused());
}


void MainEditorScreen::syncStatusBar()
{
    statusBar.setMouseWorldPosition(canvas.getMouseWorldPosition());
    statusBar.setZoom(canvas.getZoom());
    statusBar.setSimulationState(simClock.getCurrentState());
    statusBar.setHintMessage(canvas.getHintMessage());
}


void MainEditorScreen::syncLogPanel()
{
    logPanel.syncMessages(simEngine.getLog());
}


// ---- Draw ----

void MainEditorScreen::draw(SDL_Renderer* renderer)
{
    // Neutral background behind every panel
    SDL_SetRenderDrawColor(renderer, 32, 32, 36, 255);
    SDL_Rect full = { 0, 0, windowW, windowH };
    SDL_RenderFillRect(renderer, &full);

    canvas.draw(renderer);
    libraryPanel.draw(renderer);
    activePanel.draw(renderer);
    logPanel.draw(renderer);
    toolbar.draw(renderer);
    statusBar.draw(renderer);

    // The image export must capture the finished frame, so it runs
    // after everything else has been drawn but before the dialog
    // overlay is added (we do not want the dialog in the picture).
    if (pendingImageExport)
    {
        pendingImageExport = false;

        string path = "saved_projects/" +
                      projectManager.getDefaultExportFilename();

        bool ok = imageExporter.exportRegionAsPNG(renderer, canvasRect, path);

        if (ok)
        {
            simEngine.log("Circuit exported to " + path);
        }
        else
        {
            simEngine.log("ERROR: image export failed.");
        }
    }

    // The modal dialog is drawn last so it sits above everything
    propertiesDialog.draw(renderer);
}


// ---- Project lifecycle ----

void MainEditorScreen::startNewProject(float canvasW, float canvasH)
{
    simEngine.stop();

    projectManager.newProject(circuit);

    projectCanvasWidth  = canvasW;
    projectCanvasHeight = canvasH;

    canvas.clearSelection();
    canvas.setPendingComponentType("");
    canvas.resetView();

    activePanel.clear();
    libraryPanel.clearSelection();
    lastLibrarySelection = "";
    lastActiveSelection  = "";

    undoManager.initialize(projectManager.serializeCircuit(circuit));

    simEngine.clearLog();
    simEngine.log("New project created (" +
                  to_string((int)canvasW) + " x " +
                  to_string((int)canvasH) + ").");
}


bool MainEditorScreen::loadProject(const string& filePath)
{
    simEngine.stop();

    canvas.clearSelection();
    canvas.setPendingComponentType("");

    bool ok = projectManager.openProject(circuit, filePath);

    if (!ok)
    {
        simEngine.log("ERROR: could not load " + filePath);
        return false;
    }

    canvas.resetView();

    activePanel.clear();
    libraryPanel.clearSelection();
    lastLibrarySelection = "";
    lastActiveSelection  = "";

    undoManager.initialize(projectManager.serializeCircuit(circuit));

    simEngine.clearLog();
    simEngine.log("Loaded project: " +
                  projectManager.getCurrentProjectName());

    return true;
}


// ---- Exit request ----

bool MainEditorScreen::wantsReturnToMenu() const
{
    return returnToMenuRequested;
}

void MainEditorScreen::clearReturnToMenu()
{
    returnToMenuRequested = false;
}