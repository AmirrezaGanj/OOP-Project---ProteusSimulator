#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// DCVoltageSource — an ideal DC voltage source.
//
// Pins:
//   anode   = positive terminal (left, higher potential)
//   cathode = negative terminal (right, reference/0V side)
//
// Behavior:
//   Maintains a perfectly fixed voltage between its terminals
//   regardless of load current. Unlike Battery, it has no
//   internal resistance — it is an ideal source.
//
// Drawn as a circle with + on the anode side and - on the
// cathode side (standard schematic voltage source symbol).
// ============================================================

class DCVoltageSource : public Component
{
public:

    DCVoltageSource(const string& id,
                    const string& label,
                    const Vector2D& position,
                    float voltageVolts = 5.0f);

    // ---- Overrides from Component ----

    // Drives anode to voltageVolts and cathode to 0V every tick
    void evaluate() override;

    string getType()   const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Getters / Setters ----

    float getVoltage() const;
    void  setVoltage(float volts);

private:

    float voltageVolts;

    // Helper: draws a filled circle outline (no SDL2 native circle)
    void drawCircle(SDL_Renderer* renderer,
                    int centerX, int centerY,
                    int radius) const;
};