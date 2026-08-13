#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// LED — a Light Emitting Diode component.
//
// Pins:
//   anode   = positive terminal (left)
//   cathode = negative terminal (right)
//
// Behavior:
//   Lights up when:
//     1. Current flows from anode to cathode (forward biased)
//     2. Voltage across it exceeds the forward threshold (~2V)
//
//   When lit, the LED renders in its assigned color.
//   When unlit, it renders as a dim/dark version of that color.
//
//   Supported colors: RED, GREEN, BLUE
//
// Drawn as the standard diode triangle symbol pointing right,
// with two small arrows above indicating light emission when lit.
// ============================================================

class LED : public Component
{
public:

    // Supported LED colors
    enum class LEDColor
    {
        RED,
        GREEN,
        BLUE
    };

    LED(const string& id,
        const string& label,
        const Vector2D& position,
        LEDColor color            = LEDColor::RED,
        float forwardVoltage      = 2.0f);

    // ---- Overrides from Component ----

    // Checks voltage across anode/cathode and updates isLit
    void evaluate() override;

    string getType()   const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Getters / Setters ----

    LEDColor getColor()          const;
    float    getForwardVoltage() const;
    bool     isLit()             const;

    void setColor(LEDColor color);
    void setForwardVoltage(float volts);

private:

    LEDColor color;
    float    forwardVoltageVolts;
    bool     lit;

    // Helper: returns the RGB values for the current LED color
    void getColorRGB(bool bright,
                     Uint8& r, Uint8& g, Uint8& b) const;
};