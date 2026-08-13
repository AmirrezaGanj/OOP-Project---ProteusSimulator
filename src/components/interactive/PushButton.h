#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// PushButton — a momentary two-terminal interactive component.
//
// Pins:
//   pin1 = left terminal
//   pin2 = right terminal
//
// Behavior:
//   Unlike Switch, this is NOT latching. The two terminals
//   are connected ONLY while the user holds the mouse button
//   down on it. The moment the mouse is released, the circuit
//   opens again and output returns to LOW.
//
//   This is used to simulate momentary button presses, for
//   example as a clock pulse or a reset trigger.
//
// Drawn as two vertical contact lines with a horizontal bar
// above them representing the pressable button cap.
// ============================================================

class PushButton : public Component
{
public:

    PushButton(const string& id,
               const string& label,
               const Vector2D& position);

    // ---- Overrides from Component ----

    // When pressed: shares voltage between pin1 and pin2.
    // When released: pins are isolated.
    void evaluate() override;

    string getType()   const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Interaction ----

    // Called by Canvas on mouse button down over this component
    void handleMouseDown();

    // Called by Canvas on mouse button up (anywhere on screen)
    void handleMouseUp();

    // ---- Getters ----

    bool isPressed() const;

private:

    bool pressed;   // true only while mouse button is held down
};