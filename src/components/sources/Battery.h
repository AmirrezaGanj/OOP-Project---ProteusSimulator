#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Battery — a DC voltage source with internal resistance.
//
// Pins:
//   anode   = positive terminal (left, higher potential)
//   cathode = negative terminal (right, lower potential, 0V
//             if cathode is connected to GND)
//
// Behavior:
//   Drives a fixed EMF (voltage) between its two terminals.
//   Unlike the ideal DCVoltageSource, a battery also models
//   internal resistance which causes terminal voltage to
//   drop slightly under load.
//
//   Terminal voltage = EMF - (current * internalResistance)
//
//   The SimulationEngine reads emfVolts and internalResistance
//   when solving the circuit.
//
// Drawn as alternating long (negative) and short (positive)
// vertical bars — the standard multi-cell battery symbol.
// ============================================================

class Battery : public Component
{
public:

    Battery(const string& id,
            const string& label,
            const Vector2D& position,
            float emfVolts            = 9.0f,
            float internalResistance  = 1.0f);

    // ---- Overrides from Component ----

    // Drives the anode pin to emfVolts and cathode to 0V
    // (SimulationEngine adjusts for internal resistance under load)
    void evaluate() override;

    string getType()   const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Getters / Setters ----

    float getEmf()               const;
    float getInternalResistance() const;

    void setEmf(float volts);
    void setInternalResistance(float ohms);

private:

    float emfVolts;
    float internalResistanceOhms;

    // Number of cell pairs drawn in the battery symbol
    static const int CELL_COUNT = 3;
};