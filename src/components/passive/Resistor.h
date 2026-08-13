#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Resistor — a passive two-terminal component.
//
// Pins:
//   pin1 = left terminal
//   pin2 = right terminal
//
// Behavior:
//   V = I * R  (Ohm's law)
//   The SimulationEngine uses the resistance value when
//   computing node voltages across the circuit.
//
// Drawn as a rectangle body with horizontal lead lines
// extending from both sides (IEC/European standard symbol).
// ============================================================

class Resistor : public Component
{
public:

    Resistor(const string& id,
             const string& label,
             const Vector2D& position,
             float resistanceOhms = 1000.0f);

    // ---- Overrides from Component ----

    void   evaluate()      override;
    string getType()  const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Getters / Setters ----

    float getResistance() const;
    void  setResistance(float ohms);

    // Returns the current flowing through the resistor (A).
    // Updated by SimulationEngine each tick.
    float getCurrent() const;
    void  setCurrent(float amps);

private:

    float resistanceOhms;
    float currentAmps;      // updated each simulation tick by SimulationEngine
};