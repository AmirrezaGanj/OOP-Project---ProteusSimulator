#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "core/Circuit.h"
#include "core/Component.h"
#include "simulation/SimulationClock.h"
#include "ui/Toolbar.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Canvas — the main circuit design board.
//
// This is the most complex UI class in the project. It owns
// the view transform (pan + zoom) and handles every mouse and
// keyboard interaction that happens on the drawing area.
//
// Responsibilities:
//   1. View transform: zoom (mouse wheel) and pan (middle-drag)
//   2. Grid rendering that reacts to zoom and pan
//   3. Snap-to-grid for placement and dragging
//   4. Component placement from the library selection
//   5. Selection: single click, and multi-select drag rectangle
//   6. Dragging selected components (with snapping)
//   7. Rotation (R), mirroring (M), deletion (Delete)
//   8. Double-click to request the properties dialog
//   9. Wire mode: click pin -> click pin -> create wire
//  10. Pin highlighting when the mouse is near a pin
//  11. Junction creation by clicking a wire in junction mode
//  12. Live interaction during simulation (switches, buttons)
//
// Canvas does NOT own Circuit — it holds a reference.
// Canvas does NOT open dialogs itself — it raises a request
// flag that MainEditorScreen reads and acts upon.
// ============================================================

class Canvas
{
public:

    // viewportRect: the screen area the canvas occupies
    // circuit:      reference to the circuit being edited
    // clock:        reference to the simulation clock (read-only use)
    // font:         pre-loaded font for coordinate/label text
    Canvas(const SDL_Rect& viewportRect,
           Circuit& circuit,
           SimulationClock& clock,
           TTF_Font* font);

    // ---- Event handling ----

    // Returns true if the event was consumed by the canvas.
    bool handleEvent(const SDL_Event& event);

    // ---- Draw ----

    void draw(SDL_Renderer* renderer) const;

    // ---- View transform access ----

    Vector2D getPanOffset() const;
    float    getZoom()      const;

    // Mouse position in world coordinates (for the status bar)
    Vector2D getMouseWorldPosition() const;

    // Resets the view to 100% zoom centred on the origin
    void resetView();

    // ---- Placement ----

    // Sets which component type will be placed on the next click.
    // Pass "" to cancel placement mode.
    void setPendingComponentType(const string& type);
    string getPendingComponentType() const;

    // ---- Tool mode ----

    void setToolMode(Toolbar::ToolMode mode);
    Toolbar::ToolMode getToolMode() const;

    // ---- Change notification (for undo snapshots) ----

    // True when the circuit was modified since the last check.
    // MainEditorScreen polls this, saves an undo snapshot, then clears it.
    bool wasCircuitChanged() const;
    void clearCircuitChanged();

    // ---- Properties dialog request ----

    // Non-null when the user double-clicked a component.
    // MainEditorScreen reads this, opens the dialog, then clears it.
    Component* getComponentToEdit() const;
    void       clearComponentToEdit();

    // ---- Selection helpers ----

    void clearSelection();
    int  getSelectedCount() const;

    // Deletes everything currently selected (components and wires)
    void deleteSelection();

    // ---- Status hint (shown in the status bar) ----

    string getHintMessage() const;

private:

    // ---- Grid constants ----
    static const float GRID_SIZE;      // world units between grid lines
    static const float MIN_ZOOM;
    static const float MAX_ZOOM;
    static const float ZOOM_STEP;
    static const float PIN_SNAP_RADIUS;   // world units for pin detection
    static const float WIRE_CLICK_TOL;    // world units for wire hit testing

    SDL_Rect         viewportRect;
    Circuit&         circuit;
    SimulationClock& clock;
    TTF_Font*        font;             

    // ---- View transform ----
    Vector2D panOffset;     // screen-space offset of the world origin
    float    zoom;

    // ---- Mouse state ----
    Vector2D mouseScreenPos;
    Vector2D mouseWorldPos;

    // ---- Tool state ----
    Toolbar::ToolMode toolMode;
    string            pendingComponentType;   // "" = not placing

    // ---- Panning state ----
    bool     isPanning;
    Vector2D panStartMouseScreen;
    Vector2D panStartOffset;

    // ---- Dragging state ----
    bool     isDraggingComponents;
    Vector2D dragStartWorld;
    Vector2D dragLastWorld;

    // ---- Selection rectangle state ----
    bool     isSelectingRect;
    Vector2D selectRectStartWorld;

    // ---- Wire drawing state ----
    bool   isDrawingWire;
    string wireStartComponentId;
    string wireStartPinName;
    Vector2D wireStartWorldPos;

    // ---- Selected wires (indices into circuit.getWires()) ----
    vector<int> selectedWireIndices;

    // ---- Interaction with a pressed push button during simulation ----
    string pressedButtonComponentId;

    // ---- Output flags ----
    bool       circuitChanged;
    Component* componentToEdit;
    string     hintMessage;

    // ---- Colors ----
    SDL_Color bgColor;
    SDL_Color gridMinorColor;
    SDL_Color gridMajorColor;
    SDL_Color originColor;
    SDL_Color selectRectColor;
    SDL_Color wirePreviewColor;
    SDL_Color pinHighlightColor;

    // ============================================================
    // Private helpers
    // ============================================================

    // ---- Coordinate helpers ----

    Vector2D screenToWorld(const Vector2D& screenPos) const;
    Vector2D worldToScreen(const Vector2D& worldPos)  const;
    bool     isInsideViewport(int screenX, int screenY) const;

    // ---- Event sub-handlers ----

    bool handleMouseMotion(const SDL_Event& event);
    bool handleMouseDown(const SDL_Event& event);
    bool handleMouseUp(const SDL_Event& event);
    bool handleMouseWheel(const SDL_Event& event);
    bool handleKeyDown(const SDL_Event& event);

    // ---- Interaction sub-handlers ----

    // Left click in SELECT mode
    void handleSelectModeClick(const Vector2D& worldPos,
                               bool shiftHeld,
                               bool isDoubleClick);

    // Left click in WIRE mode
    void handleWireModeClick(const Vector2D& worldPos);

    // Left click while the simulation is running (interactive components)
    bool handleSimulationClick(const Vector2D& worldPos);

    // ---- Placement ----

    // Creates a new component of pendingComponentType at worldPos
    void placeComponent(const Vector2D& worldPos);

    // Factory: constructs a component instance from a type string.
    // Returns nullptr for an unknown type.
    Component* createComponent(const string& type,
                               const string& id,
                               const Vector2D& position);

    // ---- Pin detection ----

    // Updates the isHighlighted flag on every pin based on mouse position.
    void updatePinHighlights();

    // Finds the pin nearest to worldPos within PIN_SNAP_RADIUS.
    // Returns true and fills the out-params when one is found.
    bool findPinNear(const Vector2D& worldPos,
                     string& outComponentId,
                     string& outPinName,
                     Vector2D& outPinWorldPos) const;

    // ---- Wire maintenance ----

    // Re-routes every wire attached to the given component after it moved.
    void updateWiresForComponent(const string& componentId);

    // Re-routes every wire in the circuit (used after multi-drag).
    void updateAllWires();

    // ---- Selection helpers ----

    void selectSingleComponent(Component* component, bool addToSelection);
    void applyRectSelection(const Vector2D& cornerA, const Vector2D& cornerB);
    vector<Component*> getSelectedComponents() const;
    bool isWireSelected(int wireIndex) const;

    // ---- Drawing sub-routines ----

    void drawGrid(SDL_Renderer* renderer)            const;
    void drawOriginMarker(SDL_Renderer* renderer)    const;
    void drawWires(SDL_Renderer* renderer)           const;
    void drawJunctions(SDL_Renderer* renderer)       const;
    void drawComponents(SDL_Renderer* renderer)      const;
    void drawSelectionRect(SDL_Renderer* renderer)   const;
    void drawWirePreview(SDL_Renderer* renderer)     const;
    void drawPlacementGhost(SDL_Renderer* renderer)  const;

    void renderText(SDL_Renderer* renderer,
                    const string& text,
                    int x, int y,
                    SDL_Color color) const;
};