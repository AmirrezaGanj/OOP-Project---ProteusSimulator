#pragma once

#include <string>
#include <SDL2/SDL.h>
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Junction — a dot placed at the intersection of two wires
// to indicate they are electrically connected.
//
// Without a junction dot, two crossing wires are NOT connected
// (they just visually cross over each other).
// With a junction dot, they share the same electrical node.
//
// Visually it appears as a small filled circle on the canvas.
// ============================================================

class Junction
{
public:

    Junction(const Vector2D& worldPosition);

    // ---- Getters ----

    Vector2D getPosition() const;
    float    getVoltage()  const;
    bool     isSelected()  const;

    // ---- Setters ----

    void setVoltage(float v);
    void setSelected(bool sel);

    // ---- Hit detection ----

    // Returns true if worldPoint is within the junction's click radius.
    // Used by Circuit::getJunctionAtPosition() for removal on click.
    bool isPointInside(const Vector2D& worldPoint) const;

    // ---- Rendering ----

    // Draws the junction as a filled circle.
    // Color follows the same voltage-based rules as Wire:
    //   HIGH -> red, LOW -> blue, Undefined -> gray, idle -> dark green
    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom,
              bool simulationRunning) const;

    // ---- Serialization ----

    string serialize() const;

    // ---- Simulation ----

    // Resets voltage back to 0 when simulation stops
    void reset();

private:

    Vector2D position;

    // Radius used for both drawing and hit detection (in world units)
    float radius;

    float voltage;
    bool  selected;

    // Helper: draws a filled circle using SDL2 line-by-line approach
    // since SDL2 has no native circle fill function
    void drawFilledCircle(SDL_Renderer* renderer,
                          int centerX, int centerY,
                          int r) const;
};