#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// ClockGenerator — a digital square wave signal source.
//
// Pins:
//   output = single digital output pin
//
// Behavior:
//   Alternates between HIGH (5V) and LOW (0V) at a fixed
//   frequency, producing a square wave for clocking sequential
//   circuits like the DFlipFlop.
//
//   The toggle period is measured in simulation ticks.
//   halfPeriodTicks = how many evaluate() calls make up
//   one half-cycle (one HIGH phase or one LOW phase).
//
//   Example: if SimulationEngine calls evaluate() 1000 times
//   per second and halfPeriodTicks = 500, the clock runs at 1Hz.
//
// Drawn as a rectangle with a square wave symbol inside.
// ============================================================

class ClockGenerator : public Component
{
public:

    ClockGenerator(const string& id,
                   const string& label,
                   const Vector2D& position,
                   int halfPeriodTicks = 500);

    // ---- Overrides from Component ----

    // Counts ticks and toggles the output pin at the right moment
    void evaluate() override;

    string getType()   const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Getters / Setters ----

    int  getHalfPeriodTicks() const;
    void setHalfPeriodTicks(int ticks);

    // Returns true if output is currently HIGH
    bool isOutputHigh() const;

    // Resets tick counter and output to LOW (called on simulation stop)
    void resetClock();

private:

    // How many evaluate() calls per half-cycle
    int halfPeriodTicks;

    // Current tick count within the half-cycle
    int tickCounter;

    // Current output state: true = HIGH, false = LOW
    bool outputHigh;
};