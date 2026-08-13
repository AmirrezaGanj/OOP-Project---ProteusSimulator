#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Capacitor — a passive two-terminal energy storage component.
//
// Pins:
//   pin1 = left terminal
//   pin2 = right terminal
//
// Behavior:
//   I = C * dV/dt
//   The SimulationEngine handles voltage/current computation.
//
// Drawn as two parallel vertical plates with lead lines.
// ============================================================

class Capacitor : public Component
{
public:

    Capacitor(const string& id,
              const string& label,
              const Vector2D& position,
              float capacitanceFarads = 0.000001f);  // default 1 uF

    // ---- Overrides from Component ----

    void   evaluate()       override;
    string getType()  const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Getters / Setters ----

    float getCapacitance() const;
    void  setCapacitance(float farads);

    float getVoltage() const;
    void  setVoltage(float volts);

private:

    float capacitanceFarads;
    float voltageAcross;    // updated each simulation tick by SimulationEngine
};