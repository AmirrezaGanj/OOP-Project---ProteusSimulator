#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// GND — the ground reference node of the circuit.
//
// Pins:
//   pin1 = the single connection point at the top
//
// Behavior:
//   Always holds 0V. Every voltage in the circuit is
//   measured relative to this node.
//   At least one GND must exist in the circuit for the
//   SimulationEngine to have a reference and run correctly.
//
// Drawn as a lead line going down from the pin into three
// horizontal lines of decreasing width (standard GND symbol).
// ============================================================

class GND : public Component
{
public:

    GND(const string& id,
        const string& label,
        const Vector2D& position);

    // ---- Overrides from Component ----

    // Sets pin1 voltage to 0V every tick — the ground anchor
    void evaluate() override;

    string getType()   const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;
};