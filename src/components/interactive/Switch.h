#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Switch — a two-terminal interactive toggle component.
//
// Pins:
//   pin1 = left terminal
//   pin2 = right terminal
//
// Behavior:
//   The user clicks on it during simulation to toggle state.
//   - CLOSED: pin1 and pin2 are electrically shorted together.
//             SimulationEngine treats them as the same node.
//   - OPEN:   pin1 and pin2 are disconnected. No current flows.
//
//   The state change is LATCHING — it holds its position after
//   the user releases the mouse (unlike PushButton which resets).
//
// Drawn as a horizontal lead line with an angled contact arm.
// Open = arm is raised at an angle. Closed = arm is flat/connected.
// ============================================================

class Switch : public Component
{
public:

    Switch(const string& id,
           const string& label,
           const Vector2D& position,
           bool startClosed = false);

    // ---- Overrides from Component ----

    // When closed: copies pin1 voltage to pin2 and vice versa.
    // When open:   does nothing (SimulationEngine sees disconnection).
    void evaluate() override;

    string getType()   const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

    // ---- Interaction ----

    // Called by Canvas when user clicks this component during simulation.
    // Toggles between open and closed state.
    void handleClick();

    // ---- Getters ----

    bool isClosed() const;

private:

    bool closed;    // true = switch closed (conducting), false = open
};