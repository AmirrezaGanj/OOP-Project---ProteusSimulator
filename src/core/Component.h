#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include "core/Pin.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Component — abstract base class for every electronic element
// on the canvas (Resistor, Gate, LED, GND, etc.)
//
// Every derived class MUST implement:
//   - draw()       : how it looks on screen
//   - getType()    : returns a string like "RESISTOR", "AND"
//   - serialize()  : converts itself to a saveable string
//   - evaluate()   : simulation logic (voltage/state update)
// ============================================================

class Component
{
public:

    Component(const string& id,
              const string& label,
              const Vector2D& position);

    virtual ~Component() {}

    // ---- Pure virtual methods (must be overridden) ----

    // Draws the component symbol using SDL2 primitives
    virtual void draw(SDL_Renderer* renderer,
                      const Vector2D& panOffset,
                      float zoom) const = 0;

    // Returns the component type as a string, e.g. "RESISTOR"
    virtual string getType() const = 0;

    // Serializes all properties to a string for file saving
    virtual string serialize() const = 0;

    // Runs one simulation step — updates output pins based on inputs
    virtual void evaluate() = 0;

    // ---- Getters ----

    string getId() const;
    string getLabel() const;
    Vector2D getPosition() const;
    float getRotation() const;
    bool getMirrored() const;
    bool isSelected() const;
    vector<Pin>& getPins();

    // ---- Setters ----

    void setLabel(const string& newLabel);
    void setPosition(const Vector2D& newPosition);
    void setSelected(bool selected);

    // ---- Transformation methods ----

    // Moves component to a new position and updates all pin world positions
    void moveTo(const Vector2D& newPosition);

    // Rotates by 90 degrees clockwise and updates pin world positions
    void rotate90();

    // Flips horizontally and updates pin world positions
    void mirrorHorizontal();

    // ---- Hit detection ----

    // Returns true if the given world-space point is inside this component's
    // bounding box — used for click selection on the canvas
    bool isPointInside(const Vector2D& worldPoint) const;

    // Returns true if this component overlaps with a selection rectangle
    bool overlapsRect(const SDL_Rect& selectionRect, float zoom,
                      const Vector2D& panOffset) const;

    // ---- Simulation helpers ----

    // Finds and returns a pin by name, returns nullptr if not found
    Pin* findPin(const string& pinName);

    // Resets all pins voltage and state (called on simulation stop)
    void resetPins();

protected:

    string id;
    string label;
    Vector2D position;

    float rotation;       // current rotation in degrees: 0, 90, 180, or 270
    bool mirrored;        // true if horizontally flipped
    bool selected;        // true when user has clicked to select this component

    // All electrical connection points on this component.
    // Populated by derived class constructors.
    vector<Pin> pins;

    // Bounding box dimensions in world units (set by each derived class)
    float width;
    float height;

    // Recomputes every pin's worldPosition after a move, rotate, or mirror.
    // Called internally — derived classes don't need to call this manually.
    void updateAllPinPositions();

    // Helper: draws a highlight border when component is selected
    void drawSelectionHighlight(SDL_Renderer* renderer,
                                const Vector2D& panOffset,
                                float zoom) const;
};