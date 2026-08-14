#include "ui/Canvas.h"
#include "utils/MathUtils.h"

// Component headers — needed by the placement factory and by
// the live-simulation interaction handler.
#include "components/sources/GND.h"
#include "components/sources/DCVoltageSource.h"
#include "components/sources/Battery.h"
#include "components/sources/ClockGenerator.h"
#include "components/passive/Resistor.h"
#include "components/passive/Capacitor.h"
#include "components/passive/Inductor.h"
#include "components/interactive/Switch.h"
#include "components/interactive/PushButton.h"
#include "components/interactive/LED.h"
#include "components/interactive/SevenSegment.h"
#include "components/gates/ANDGate.h"
#include "components/gates/ORGate.h"
#include "components/gates/NOTGate.h"
#include "components/gates/NANDGate.h"
#include "components/gates/XORGate.h"
#include "components/gates/DFlipFlop.h"

#include <cmath>
#include <algorithm>

using namespace std;

// ============================================================
// Canvas.cpp
// ============================================================

const float Canvas::GRID_SIZE       = 20.0f;
const float Canvas::MIN_ZOOM        = 0.25f;
const float Canvas::MAX_ZOOM        = 4.0f;
const float Canvas::ZOOM_STEP       = 1.12f;
const float Canvas::PIN_SNAP_RADIUS = 10.0f;
const float Canvas::WIRE_CLICK_TOL  = 6.0f;


// ---- Constructor ----

Canvas::Canvas(const SDL_Rect& viewportRect,
               Circuit& circuit,
               SimulationClock& clock,
               TTF_Font* font)
    : viewportRect(viewportRect),
      circuit(circuit),
      clock(clock),
      font(font),
      panOffset(0.0f, 0.0f),
      zoom(1.0f),
      mouseScreenPos(0.0f, 0.0f),
      mouseWorldPos(0.0f, 0.0f),
      toolMode(Toolbar::ToolMode::SELECT),
      pendingComponentType(""),
      isPanning(false),
      panStartMouseScreen(0.0f, 0.0f),
      panStartOffset(0.0f, 0.0f),
      isDraggingComponents(false),
      dragStartWorld(0.0f, 0.0f),
      dragLastWorld(0.0f, 0.0f),
      isSelectingRect(false),
      selectRectStartWorld(0.0f, 0.0f),
      isDrawingWire(false),
      wireStartComponentId(""),
      wireStartPinName(""),
      wireStartWorldPos(0.0f, 0.0f),
      pressedButtonComponentId(""),
      circuitChanged(false),
      componentToEdit(nullptr),
      hintMessage("Ready")
{
    bgColor           = { 245, 245, 240, 255 };
    gridMinorColor    = { 220, 220, 214, 255 };
    gridMajorColor    = { 198, 198, 190, 255 };
    originColor       = { 180, 120, 120, 255 };
    selectRectColor   = { 0,   120, 215, 255 };
    wirePreviewColor  = { 0,   150, 0,   255 };
    pinHighlightColor = { 255, 200, 0,   255 };

    // Start with the world origin near the top-left of the viewport,
    // leaving a small margin so the origin marker is visible.
    panOffset = Vector2D((float)viewportRect.x + 60.0f,
                         (float)viewportRect.y + 60.0f);
}


// ---- Coordinate helpers ----

Vector2D Canvas::screenToWorld(const Vector2D& screenPos) const
{
    return MathUtils::screenToWorld(screenPos, panOffset, zoom);
}

Vector2D Canvas::worldToScreen(const Vector2D& worldPos) const
{
    return MathUtils::worldToScreen(worldPos, panOffset, zoom);
}

bool Canvas::isInsideViewport(int screenX, int screenY) const
{
    SDL_Point pt   = { screenX, screenY };
    SDL_Rect  rect = viewportRect;
    return (SDL_PointInRect(&pt, &rect) == SDL_TRUE);
}


// ---- Event handling ----

bool Canvas::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        return handleMouseMotion(event);
    }

    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        return handleMouseDown(event);
    }

    if (event.type == SDL_MOUSEBUTTONUP)
    {
        return handleMouseUp(event);
    }

    if (event.type == SDL_MOUSEWHEEL)
    {
        return handleMouseWheel(event);
    }

    if (event.type == SDL_KEYDOWN)
    {
        return handleKeyDown(event);
    }

    return false;
}


bool Canvas::handleMouseMotion(const SDL_Event& event)
{
    mouseScreenPos = Vector2D((float)event.motion.x, (float)event.motion.y);
    mouseWorldPos  = screenToWorld(mouseScreenPos);

    // ---- Panning ----
    if (isPanning)
    {
        Vector2D delta = mouseScreenPos - panStartMouseScreen;
        panOffset      = panStartOffset + delta;
        hintMessage    = "Panning view";
        return true;
    }

    // ---- Dragging selected components ----
    if (isDraggingComponents)
    {
        Vector2D snappedNow = MathUtils::snapToGrid(mouseWorldPos, GRID_SIZE);
        Vector2D delta      = snappedNow - dragLastWorld;

        if (delta.x != 0.0f || delta.y != 0.0f)
        {
            vector<Component*> selected = getSelectedComponents();

            for (int i = 0; i < (int)selected.size(); i++)
            {
                Vector2D newPos = selected[i]->getPosition() + delta;
                selected[i]->moveTo(MathUtils::snapToGrid(newPos, GRID_SIZE));
            }

            // Wires attached to moved components must follow
            updateAllWires();

            dragLastWorld = snappedNow;
        }

        hintMessage = "Moving components";
        return true;
    }

    // ---- Selection rectangle ----
    if (isSelectingRect)
    {
        hintMessage = "Selecting";
        return true;
    }

    // ---- Pin highlighting (wire mode, or always for feedback) ----
    if (isInsideViewport(event.motion.x, event.motion.y))
    {
        updatePinHighlights();
    }

    return isInsideViewport(event.motion.x, event.motion.y);
}


bool Canvas::handleMouseDown(const SDL_Event& event)
{
    if (!isInsideViewport(event.button.x, event.button.y))
    {
        return false;
    }

    mouseScreenPos = Vector2D((float)event.button.x, (float)event.button.y);
    mouseWorldPos  = screenToWorld(mouseScreenPos);

    // ---- Middle mouse: start panning ----
    if (event.button.button == SDL_BUTTON_MIDDLE)
    {
        isPanning           = true;
        panStartMouseScreen = mouseScreenPos;
        panStartOffset      = panOffset;
        return true;
    }

    // ---- Right mouse: cancel current action / delete under cursor ----
    if (event.button.button == SDL_BUTTON_RIGHT)
    {
        if (isDrawingWire)
        {
            isDrawingWire = false;
            hintMessage   = "Wire cancelled";
            return true;
        }

        if (!pendingComponentType.empty())
        {
            pendingComponentType = "";
            hintMessage          = "Placement cancelled";
            return true;
        }

        // Right-click on a component deletes it
        Component* comp = circuit.getComponentAtPosition(mouseWorldPos);
        if (comp != nullptr)
        {
            circuit.removeComponent(comp->getId());
            selectedWireIndices.clear();
            circuitChanged = true;
            hintMessage    = "Component deleted";
            return true;
        }

        return true;
    }

    if (event.button.button != SDL_BUTTON_LEFT)
    {
        return false;
    }

    // ---- Live simulation interaction takes priority ----
    if (!clock.isStopped())
    {
        if (handleSimulationClick(mouseWorldPos))
        {
            return true;
        }
    }

    bool shiftHeld     = (SDL_GetModState() & KMOD_SHIFT) != 0;
    bool isDoubleClick = (event.button.clicks == 2);

    if (toolMode == Toolbar::ToolMode::WIRE)
    {
        handleWireModeClick(mouseWorldPos);
        return true;
    }

    handleSelectModeClick(mouseWorldPos, shiftHeld, isDoubleClick);
    return true;
}


bool Canvas::handleMouseUp(const SDL_Event& event)
{
    // Release a held push button regardless of where the mouse is
    if (event.button.button == SDL_BUTTON_LEFT &&
        !pressedButtonComponentId.empty())
    {
        Component* comp = circuit.findComponent(pressedButtonComponentId);
        if (comp != nullptr)
        {
            PushButton* btn = dynamic_cast<PushButton*>(comp);
            if (btn != nullptr)
            {
                btn->handleMouseUp();
            }
        }
        pressedButtonComponentId = "";
    }

    if (event.button.button == SDL_BUTTON_MIDDLE && isPanning)
    {
        isPanning   = false;
        hintMessage = "Ready";
        return true;
    }

    if (event.button.button != SDL_BUTTON_LEFT)
    {
        return false;
    }

    // ---- Finish dragging ----
    if (isDraggingComponents)
    {
        isDraggingComponents = false;
        circuitChanged       = true;
        hintMessage          = "Ready";
        return true;
    }

    // ---- Finish rectangle selection ----
    if (isSelectingRect)
    {
        isSelectingRect = false;

        Vector2D endWorld = screenToWorld(
            Vector2D((float)event.button.x, (float)event.button.y));

        applyRectSelection(selectRectStartWorld, endWorld);

        hintMessage = "Selected " + to_string(getSelectedCount()) + " item(s)";
        return true;
    }

    return false;
}


bool Canvas::handleMouseWheel(const SDL_Event& event)
{
    int mx = 0;
    int my = 0;
    SDL_GetMouseState(&mx, &my);

    if (!isInsideViewport(mx, my))
    {
        return false;
    }

    Vector2D cursorScreen((float)mx, (float)my);

    // World point currently under the cursor — we want it to stay put
    Vector2D worldUnderCursor = screenToWorld(cursorScreen);

    float newZoom = zoom;

    if (event.wheel.y > 0)
    {
        newZoom = zoom * ZOOM_STEP;
    }
    else if (event.wheel.y < 0)
    {
        newZoom = zoom / ZOOM_STEP;
    }

    newZoom = MathUtils::clamp(newZoom, MIN_ZOOM, MAX_ZOOM);

    if (newZoom == zoom)
    {
        return true;
    }

    zoom = newZoom;

    // Recompute the pan so worldUnderCursor maps back to cursorScreen
    panOffset = cursorScreen - (worldUnderCursor * zoom);

    mouseWorldPos = screenToWorld(cursorScreen);

    hintMessage = "Zoom " + to_string((int)(zoom * 100.0f)) + "%";
    return true;
}


bool Canvas::handleKeyDown(const SDL_Event& event)
{
    SDL_Keycode key = event.key.keysym.sym;

    // ---- Delete selection ----
    if (key == SDLK_DELETE || key == SDLK_BACKSPACE)
    {
        if (getSelectedCount() > 0)
        {
            deleteSelection();
            return true;
        }
        return false;
    }

    // ---- Rotate selected components ----
    if (key == SDLK_r)
    {
        vector<Component*> selected = getSelectedComponents();
        if (selected.empty())
        {
            return false;
        }

        for (int i = 0; i < (int)selected.size(); i++)
        {
            selected[i]->rotate90();
        }

        updateAllWires();
        circuitChanged = true;
        hintMessage    = "Rotated 90 degrees";
        return true;
    }

    // ---- Mirror selected components ----
    if (key == SDLK_m)
    {
        vector<Component*> selected = getSelectedComponents();
        if (selected.empty())
        {
            return false;
        }

        for (int i = 0; i < (int)selected.size(); i++)
        {
            selected[i]->mirrorHorizontal();
        }

        updateAllWires();
        circuitChanged = true;
        hintMessage    = "Mirrored";
        return true;
    }

    // ---- Escape: cancel everything in progress ----
    if (key == SDLK_ESCAPE)
    {
        isDrawingWire        = false;
        isSelectingRect      = false;
        isDraggingComponents = false;
        pendingComponentType = "";
        clearSelection();
        hintMessage = "Ready";
        return true;
    }

    // ---- Reset view ----
    if (key == SDLK_HOME)
    {
        resetView();
        return true;
    }

    return false;
}


// ---- SELECT mode click ----

void Canvas::handleSelectModeClick(const Vector2D& worldPos,
                                    bool shiftHeld,
                                    bool isDoubleClick)
{
    // ---- Placement has priority when a component type is pending ----
    if (!pendingComponentType.empty())
    {
        placeComponent(worldPos);
        return;
    }

    Component* comp = circuit.getComponentAtPosition(worldPos);

    // ---- Double click opens the properties dialog ----
    if (isDoubleClick && comp != nullptr)
    {
        componentToEdit = comp;
        hintMessage     = "Editing " + comp->getId();
        return;
    }

    if (comp != nullptr)
    {
        // Clicking an already-selected component starts a drag of the
        // whole selection. Clicking an unselected one selects it first.
        if (!comp->isSelected())
        {
            selectSingleComponent(comp, shiftHeld);
        }

        isDraggingComponents = true;
        dragStartWorld       = MathUtils::snapToGrid(worldPos, GRID_SIZE);
        dragLastWorld        = dragStartWorld;
        hintMessage          = "Moving " + comp->getId();
        return;
    }

    // ---- Clicked a wire? ----
    int wireIdx = circuit.getWireIndexAtPosition(worldPos, WIRE_CLICK_TOL);
    if (wireIdx >= 0)
    {
        if (!shiftHeld)
        {
            clearSelection();
        }

        if (!isWireSelected(wireIdx))
        {
            selectedWireIndices.push_back(wireIdx);
        }

        hintMessage = "Wire selected — press Delete to remove";
        return;
    }

    // ---- Clicked empty space: begin a rubber-band selection ----
    if (!shiftHeld)
    {
        clearSelection();
    }

    isSelectingRect      = true;
    selectRectStartWorld = worldPos;
}


// ---- WIRE mode click ----

void Canvas::handleWireModeClick(const Vector2D& worldPos)
{
    string   compId;
    string   pinName;
    Vector2D pinPos;

    bool foundPin = findPinNear(worldPos, compId, pinName, pinPos);

    if (!isDrawingWire)
    {
        if (foundPin)
        {
            isDrawingWire        = true;
            wireStartComponentId = compId;
            wireStartPinName     = pinName;
            wireStartWorldPos    = pinPos;
            hintMessage          = "Wiring from " + compId + "." + pinName;
        }
        else
        {
            // Not on a pin — clicking a wire adds a junction dot there
            int wireIdx = circuit.getWireIndexAtPosition(worldPos, WIRE_CLICK_TOL);

            if (wireIdx >= 0)
            {
                Vector2D snapped = MathUtils::snapToGrid(worldPos, GRID_SIZE);

                if (circuit.getJunctionAtPosition(snapped, 8.0f) == nullptr)
                {
                    circuit.addJunction(snapped);
                    circuitChanged = true;
                    hintMessage    = "Junction added";
                }
                else
                {
                    circuit.removeJunction(snapped);
                    circuitChanged = true;
                    hintMessage    = "Junction removed";
                }
            }
            else
            {
                hintMessage = "Click a pin to start a wire";
            }
        }

        return;
    }

    // ---- Second click: finish the wire ----
    if (!foundPin)
    {
        hintMessage = "Wire must end on a pin";
        return;
    }

    // Reject a wire from a pin back to itself
    if (compId == wireStartComponentId && pinName == wireStartPinName)
    {
        isDrawingWire = false;
        hintMessage   = "Wire cancelled";
        return;
    }

    bool created = circuit.addWire(wireStartComponentId, wireStartPinName,
                                   compId, pinName);

    if (created)
    {
        circuitChanged = true;
        hintMessage    = "Wire created";
    }
    else
    {
        hintMessage = "Could not create wire";
    }

    isDrawingWire = false;
}


// ---- Live simulation interaction ----

bool Canvas::handleSimulationClick(const Vector2D& worldPos)
{
    Component* comp = circuit.getComponentAtPosition(worldPos);

    if (comp == nullptr)
    {
        return false;
    }

    // Switch toggles on click
    Switch* sw = dynamic_cast<Switch*>(comp);
    if (sw != nullptr)
    {
        sw->handleClick();
        hintMessage = sw->isClosed() ? "Switch closed" : "Switch open";
        return true;
    }

    // Push button is held down until mouse-up
    PushButton* btn = dynamic_cast<PushButton*>(comp);
    if (btn != nullptr)
    {
        btn->handleMouseDown();
        pressedButtonComponentId = comp->getId();
        hintMessage              = "Button pressed";
        return true;
    }

    return false;
}


// ---- Placement ----

void Canvas::placeComponent(const Vector2D& worldPos)
{
    if (pendingComponentType.empty())
    {
        return;
    }

    Vector2D snapped = MathUtils::snapToGrid(worldPos, GRID_SIZE);

    string newId = circuit.generateUniqueId(pendingComponentType);

    Component* comp = createComponent(pendingComponentType, newId, snapped);

    if (comp == nullptr)
    {
        hintMessage = "Unknown component type";
        return;
    }

    circuit.addComponent(comp);
    circuitChanged = true;
    hintMessage    = "Placed " + newId;
}


Component* Canvas::createComponent(const string& type,
                                    const string& id,
                                    const Vector2D& position)
{
    // Sources
    if (type == "GND")       return new GND(id, id, position);
    if (type == "DCVOLTAGE") return new DCVoltageSource(id, id, position);
    if (type == "BATTERY")   return new Battery(id, id, position);
    if (type == "CLOCK")     return new ClockGenerator(id, id, position);

    // Passive
    if (type == "RESISTOR")  return new Resistor(id, id, position);
    if (type == "CAPACITOR") return new Capacitor(id, id, position);
    if (type == "INDUCTOR")  return new Inductor(id, id, position);

    // Interactive
    if (type == "SWITCH")    return new Switch(id, id, position);
    if (type == "BUTTON")    return new PushButton(id, id, position);
    if (type == "LED")       return new LED(id, id, position);
    if (type == "7SEG")      return new SevenSegment(id, id, position);

    // Gates
    if (type == "AND")       return new ANDGate(id, id, position);
    if (type == "OR")        return new ORGate(id, id, position);
    if (type == "NOT")       return new NOTGate(id, id, position);
    if (type == "NAND")      return new NANDGate(id, id, position);
    if (type == "XOR")       return new XORGate(id, id, position);
    if (type == "DFF")       return new DFlipFlop(id, id, position);

    return nullptr;
}


// ---- Pin detection ----

void Canvas::updatePinHighlights()
{
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        vector<Pin>& pins = components[i]->getPins();

        for (int j = 0; j < (int)pins.size(); j++)
        {
            // Pin::checkMouseOver sets isHighlighted internally
            pins[j].sensitivityRadius = PIN_SNAP_RADIUS;
            pins[j].checkMouseOver(mouseWorldPos);
        }
    }
}


bool Canvas::findPinNear(const Vector2D& worldPos,
                          string& outComponentId,
                          string& outPinName,
                          Vector2D& outPinWorldPos) const
{
    vector<Component*>& components = circuit.getComponents();

    float      bestDistance = PIN_SNAP_RADIUS;
    bool       found        = false;

    for (int i = 0; i < (int)components.size(); i++)
    {
        vector<Pin>& pins = components[i]->getPins();

        for (int j = 0; j < (int)pins.size(); j++)
        {
            float d = MathUtils::distance(pins[j].worldPosition, worldPos);

            if (d <= bestDistance)
            {
                bestDistance   = d;
                outComponentId = components[i]->getId();
                outPinName     = pins[j].name;
                outPinWorldPos = pins[j].worldPosition;
                found          = true;
            }
        }
    }

    return found;
}


// ---- Wire maintenance ----

void Canvas::updateWiresForComponent(const string& componentId)
{
    vector<Wire*>& wires = circuit.getWires();

    for (int i = 0; i < (int)wires.size(); i++)
    {
        if (!wires[i]->isConnectedToComponent(componentId))
        {
            continue;
        }

        Component* fromComp = circuit.findComponent(wires[i]->getFromComponentId());
        Component* toComp   = circuit.findComponent(wires[i]->getToComponentId());

        if (fromComp == nullptr || toComp == nullptr)
        {
            continue;
        }

        Pin* fromPin = fromComp->findPin(wires[i]->getFromPinName());
        Pin* toPin   = toComp->findPin(wires[i]->getToPinName());

        if (fromPin == nullptr || toPin == nullptr)
        {
            continue;
        }

        wires[i]->updateEndpoints(fromPin->worldPosition,
                                  toPin->worldPosition);
    }
}


void Canvas::updateAllWires()
{
    vector<Wire*>& wires = circuit.getWires();

    for (int i = 0; i < (int)wires.size(); i++)
    {
        Component* fromComp = circuit.findComponent(wires[i]->getFromComponentId());
        Component* toComp   = circuit.findComponent(wires[i]->getToComponentId());

        if (fromComp == nullptr || toComp == nullptr)
        {
            continue;
        }

        Pin* fromPin = fromComp->findPin(wires[i]->getFromPinName());
        Pin* toPin   = toComp->findPin(wires[i]->getToPinName());

        if (fromPin == nullptr || toPin == nullptr)
        {
            continue;
        }

        wires[i]->updateEndpoints(fromPin->worldPosition,
                                  toPin->worldPosition);
    }
}


// ---- Selection helpers ----

void Canvas::selectSingleComponent(Component* component, bool addToSelection)
{
    if (!addToSelection)
    {
        clearSelection();
    }

    if (component != nullptr)
    {
        component->setSelected(true);
    }
}


void Canvas::applyRectSelection(const Vector2D& cornerA,
                                 const Vector2D& cornerB)
{
    // Build the selection rectangle in screen space, because
    // Component::overlapsRect works with screen coordinates.
    Vector2D screenA = worldToScreen(cornerA);
    Vector2D screenB = worldToScreen(cornerB);

    int left   = (int)min(screenA.x, screenB.x);
    int top    = (int)min(screenA.y, screenB.y);
    int width  = (int)fabs(screenB.x - screenA.x);
    int height = (int)fabs(screenB.y - screenA.y);

    // A tiny rectangle means the user just clicked — ignore it
    if (width < 3 && height < 3)
    {
        return;
    }

    SDL_Rect selRect = { left, top, width, height };

    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        if (components[i]->overlapsRect(selRect, zoom, panOffset))
        {
            components[i]->setSelected(true);
        }
    }
}


vector<Component*> Canvas::getSelectedComponents() const
{
    vector<Component*> result;

    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        if (components[i]->isSelected())
        {
            result.push_back(components[i]);
        }
    }

    return result;
}


bool Canvas::isWireSelected(int wireIndex) const
{
    for (int i = 0; i < (int)selectedWireIndices.size(); i++)
    {
        if (selectedWireIndices[i] == wireIndex)
        {
            return true;
        }
    }
    return false;
}


void Canvas::clearSelection()
{
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        components[i]->setSelected(false);
    }

    selectedWireIndices.clear();
}


int Canvas::getSelectedCount() const
{
    return (int)getSelectedComponents().size() +
           (int)selectedWireIndices.size();
}


void Canvas::deleteSelection()
{
    // ---- Delete selected wires first (highest index first,
    //      so earlier indices stay valid while erasing) ----
    vector<int> wireIndices = selectedWireIndices;
    sort(wireIndices.begin(), wireIndices.end());

    for (int i = (int)wireIndices.size() - 1; i >= 0; i--)
    {
        circuit.removeWire(wireIndices[i]);
    }

    selectedWireIndices.clear();

    // ---- Delete selected components ----
    // Collect the IDs first: removeComponent() mutates the vector
    // we would otherwise be iterating over.
    vector<string> idsToDelete;
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        if (components[i]->isSelected())
        {
            idsToDelete.push_back(components[i]->getId());
        }
    }

    for (int i = 0; i < (int)idsToDelete.size(); i++)
    {
        circuit.removeComponent(idsToDelete[i]);
    }

    if (!idsToDelete.empty() || !wireIndices.empty())
    {
        circuitChanged = true;
        hintMessage    = "Deleted " +
                         to_string(idsToDelete.size() + wireIndices.size()) +
                         " item(s)";
    }
}


// ---- Draw ----

void Canvas::draw(SDL_Renderer* renderer) const
{
    // Clip all canvas drawing to the viewport so nothing spills
    // over the toolbar or the side panels.
    SDL_RenderSetClipRect(renderer, &viewportRect);

    // Background
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &viewportRect);

    drawGrid(renderer);
    drawOriginMarker(renderer);

    // Draw order matters: wires under components, junctions on top of wires
    drawWires(renderer);
    drawJunctions(renderer);
    drawComponents(renderer);

    drawWirePreview(renderer);
    drawSelectionRect(renderer);
    drawPlacementGhost(renderer);

    SDL_RenderSetClipRect(renderer, nullptr);
}


void Canvas::drawGrid(SDL_Renderer* renderer) const
{
    float spacingScreen = GRID_SIZE * zoom;

    // When zoomed far out the minor grid becomes visual noise
    bool drawMinor = (spacingScreen >= 6.0f);

    // World-space bounds of the visible viewport
    Vector2D topLeftWorld = screenToWorld(
        Vector2D((float)viewportRect.x, (float)viewportRect.y));

    Vector2D bottomRightWorld = screenToWorld(
        Vector2D((float)(viewportRect.x + viewportRect.w),
                 (float)(viewportRect.y + viewportRect.h)));

    float startX = floor(topLeftWorld.x / GRID_SIZE) * GRID_SIZE;
    float startY = floor(topLeftWorld.y / GRID_SIZE) * GRID_SIZE;

    // ---- Vertical lines ----
    for (float wx = startX; wx <= bottomRightWorld.x; wx += GRID_SIZE)
    {
        // Every 5th line is a major line
        int   index   = (int)round(wx / GRID_SIZE);
        bool  isMajor = (index % 5 == 0);

        if (!isMajor && !drawMinor)
        {
            continue;
        }

        SDL_Color col = isMajor ? gridMajorColor : gridMinorColor;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);

        int sx = (int)worldToScreen(Vector2D(wx, 0.0f)).x;

        SDL_RenderDrawLine(renderer,
                           sx, viewportRect.y,
                           sx, viewportRect.y + viewportRect.h);
    }

    // ---- Horizontal lines ----
    for (float wy = startY; wy <= bottomRightWorld.y; wy += GRID_SIZE)
    {
        int   index   = (int)round(wy / GRID_SIZE);
        bool  isMajor = (index % 5 == 0);

        if (!isMajor && !drawMinor)
        {
            continue;
        }

        SDL_Color col = isMajor ? gridMajorColor : gridMinorColor;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);

        int sy = (int)worldToScreen(Vector2D(0.0f, wy)).y;

        SDL_RenderDrawLine(renderer,
                           viewportRect.x,                    sy,
                           viewportRect.x + viewportRect.w,   sy);
    }
}


void Canvas::drawOriginMarker(SDL_Renderer* renderer) const
{
    Vector2D origin = worldToScreen(Vector2D(0.0f, 0.0f));

    SDL_SetRenderDrawColor(renderer,
                           originColor.r, originColor.g,
                           originColor.b, originColor.a);

    int size = 8;

    SDL_RenderDrawLine(renderer,
                       (int)origin.x - size, (int)origin.y,
                       (int)origin.x + size, (int)origin.y);

    SDL_RenderDrawLine(renderer,
                       (int)origin.x, (int)origin.y - size,
                       (int)origin.x, (int)origin.y + size);
}


void Canvas::drawWires(SDL_Renderer* renderer) const
{
    bool simActive = !clock.isStopped();

    vector<Wire*>& wires = circuit.getWires();

    for (int i = 0; i < (int)wires.size(); i++)
    {
        wires[i]->draw(renderer, panOffset, zoom, simActive);

        // Highlight selected wires with a marker at each waypoint
        if (isWireSelected(i))
        {
            SDL_SetRenderDrawColor(renderer,
                                   selectRectColor.r, selectRectColor.g,
                                   selectRectColor.b, selectRectColor.a);

            const vector<Vector2D>& waypoints = wires[i]->getWaypoints();

            for (int j = 0; j < (int)waypoints.size(); j++)
            {
                Vector2D sp = worldToScreen(waypoints[j]);

                SDL_Rect marker;
                marker.x = (int)sp.x - 3;
                marker.y = (int)sp.y - 3;
                marker.w = 6;
                marker.h = 6;

                SDL_RenderFillRect(renderer, &marker);
            }
        }
    }
}


void Canvas::drawJunctions(SDL_Renderer* renderer) const
{
    bool simActive = !clock.isStopped();

    vector<Junction*>& junctions = circuit.getJunctions();

    for (int i = 0; i < (int)junctions.size(); i++)
    {
        junctions[i]->draw(renderer, panOffset, zoom, simActive);
    }
}


void Canvas::drawComponents(SDL_Renderer* renderer) const
{
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        components[i]->draw(renderer, panOffset, zoom);

        // Component label drawn just below its bounding box
        if (font != nullptr && zoom >= 0.6f)
        {
            Vector2D center = worldToScreen(components[i]->getPosition());

            SDL_Color labelColor = { 70, 70, 70, 255 };

            renderText(renderer,
                       components[i]->getLabel(),
                       (int)center.x - 12,
                       (int)center.y + (int)(22.0f * zoom),
                       labelColor);
        }
    }
}


void Canvas::drawSelectionRect(SDL_Renderer* renderer) const
{
    if (!isSelectingRect)
    {
        return;
    }

    Vector2D startScreen = worldToScreen(selectRectStartWorld);

    int left   = (int)min(startScreen.x, mouseScreenPos.x);
    int top    = (int)min(startScreen.y, mouseScreenPos.y);
    int width  = (int)fabs(mouseScreenPos.x - startScreen.x);
    int height = (int)fabs(mouseScreenPos.y - startScreen.y);

    SDL_Rect rect = { left, top, width, height };

    // Translucent fill
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,
                           selectRectColor.r, selectRectColor.g,
                           selectRectColor.b, 40);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Solid outline
    SDL_SetRenderDrawColor(renderer,
                           selectRectColor.r, selectRectColor.g,
                           selectRectColor.b, 255);
    SDL_RenderDrawRect(renderer, &rect);
}


void Canvas::drawWirePreview(SDL_Renderer* renderer) const
{
    if (!isDrawingWire)
    {
        return;
    }

    Vector2D startScreen = worldToScreen(wireStartWorldPos);

    // Preview follows the same 90-degree rule as a real wire:
    // horizontal first, then vertical.
    Vector2D cornerScreen(mouseScreenPos.x, startScreen.y);

    SDL_SetRenderDrawColor(renderer,
                           wirePreviewColor.r, wirePreviewColor.g,
                           wirePreviewColor.b, wirePreviewColor.a);

    SDL_RenderDrawLine(renderer,
                       (int)startScreen.x,  (int)startScreen.y,
                       (int)cornerScreen.x, (int)cornerScreen.y);

    SDL_RenderDrawLine(renderer,
                       (int)cornerScreen.x,   (int)cornerScreen.y,
                       (int)mouseScreenPos.x, (int)mouseScreenPos.y);

    // Marker at the wire origin pin
    SDL_SetRenderDrawColor(renderer,
                           pinHighlightColor.r, pinHighlightColor.g,
                           pinHighlightColor.b, pinHighlightColor.a);

    SDL_Rect startDot;
    startDot.x = (int)startScreen.x - 4;
    startDot.y = (int)startScreen.y - 4;
    startDot.w = 8;
    startDot.h = 8;
    SDL_RenderFillRect(renderer, &startDot);
}


void Canvas::drawPlacementGhost(SDL_Renderer* renderer) const
{
    if (pendingComponentType.empty())
    {
        return;
    }

    // A simple crosshair plus a dashed bounding box at the snapped
    // position, showing where the component will land.
    Vector2D snapped      = MathUtils::snapToGrid(mouseWorldPos, GRID_SIZE);
    Vector2D snappedScreen = worldToScreen(snapped);

    SDL_SetRenderDrawColor(renderer, 0, 140, 200, 255);

    int cross = 10;

    SDL_RenderDrawLine(renderer,
                       (int)snappedScreen.x - cross, (int)snappedScreen.y,
                       (int)snappedScreen.x + cross, (int)snappedScreen.y);

    SDL_RenderDrawLine(renderer,
                       (int)snappedScreen.x, (int)snappedScreen.y - cross,
                       (int)snappedScreen.x, (int)snappedScreen.y + cross);

    int ghostW = (int)(60.0f * zoom);
    int ghostH = (int)(30.0f * zoom);

    SDL_Rect ghost;
    ghost.x = (int)snappedScreen.x - ghostW / 2;
    ghost.y = (int)snappedScreen.y - ghostH / 2;
    ghost.w = ghostW;
    ghost.h = ghostH;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 140, 200, 60);
    SDL_RenderFillRect(renderer, &ghost);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_SetRenderDrawColor(renderer, 0, 140, 200, 255);
    SDL_RenderDrawRect(renderer, &ghost);
}


void Canvas::renderText(SDL_Renderer* renderer,
                         const string& text,
                         int x, int y,
                         SDL_Color color) const
{
    if (font == nullptr || text.empty())
    {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surface == nullptr)
    {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture == nullptr)
    {
        return;
    }

    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);

    SDL_Rect destRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
}


// ---- View transform access ----

Vector2D Canvas::getPanOffset() const
{
    return panOffset;
}

float Canvas::getZoom() const
{
    return zoom;
}

Vector2D Canvas::getMouseWorldPosition() const
{
    return mouseWorldPos;
}

void Canvas::resetView()
{
    zoom      = 1.0f;
    panOffset = Vector2D((float)viewportRect.x + 60.0f,
                         (float)viewportRect.y + 60.0f);
    hintMessage = "View reset to 100%";
}


// ---- Placement accessors ----

void Canvas::setPendingComponentType(const string& type)
{
    pendingComponentType = type;

    if (!type.empty())
    {
        hintMessage = "Click on the canvas to place " + type;
    }
}

string Canvas::getPendingComponentType() const
{
    return pendingComponentType;
}


// ---- Tool mode ----

void Canvas::setToolMode(Toolbar::ToolMode mode)
{
    toolMode = mode;

    // Switching tools cancels anything in progress
    isDrawingWire        = false;
    isSelectingRect      = false;
    isDraggingComponents = false;

    if (mode == Toolbar::ToolMode::WIRE)
    {
        pendingComponentType = "";
        hintMessage          = "Wire mode — click a pin to start";
    }
    else
    {
        hintMessage = "Select mode";
    }
}

Toolbar::ToolMode Canvas::getToolMode() const
{
    return toolMode;
}


// ---- Change notification ----

bool Canvas::wasCircuitChanged() const
{
    return circuitChanged;
}

void Canvas::clearCircuitChanged()
{
    circuitChanged = false;
}


// ---- Properties dialog request ----

Component* Canvas::getComponentToEdit() const
{
    return componentToEdit;
}

void Canvas::clearComponentToEdit()
{
    componentToEdit = nullptr;
}


// ---- Status hint ----

string Canvas::getHintMessage() const
{
    return hintMessage;
}