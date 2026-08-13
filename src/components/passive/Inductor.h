#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Inductor — a passive two-terminal energy storage component.
//
// Pins:
//   pin1 = left terminal
//   pin2 = right terminal
//
// Behavior:
//   V = L * dI/dt
//   The SimulationEngine handles voltage/current computation.
//
// Drawn as a series of arcs (coil/solenoid symbol)
// above the horizontal lead line.
// ============================================================

class Inductor : public Component
{
public:

    Inductor(const string& id,
             const string& label,
             const Vector2D& position,
             float inductanceHenries = 0.001f);   // default 1 mH

    // ---- Overrides from Component ----

    void   evaluate()       override;
    string getType()  const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Getters / Setters ----

    float getInductance() const;
    void  setInductance(float henries);

    float getCurrent() const;
    void  setCurrent(float amps);

private:

    float inductanceHenries;
    float currentAmps;      // updated each simulation tick by SimulationEngine

    // Number of coil bumps drawn in the symbol
    static const int COIL_BUMPS = 4;
};