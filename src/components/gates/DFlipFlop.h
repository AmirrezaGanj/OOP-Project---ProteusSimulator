#pragma once

#include "components/gates/LogicGate.h"

using namespace std;

// ============================================================
// DFlipFlop — edge-triggered D flip-flop.
//
// Pins:
//   input1 = D   (data input)
//   input2 = CLK (clock input)
//   output = Q   (stored output)
//
// Behavior:
//   On every RISING EDGE of CLK (LOW->HIGH transition),
//   the value of D is captured and held at Q.
//   Q does not change at any other time.
//
// This is independent of simulation tick rate — it only
// reacts to the clock edge event, not to how often
// evaluate() is called.
// ============================================================

class DFlipFlop : public LogicGate
{
public:

    DFlipFlop(const string& id,
              const string& label,
              const Vector2D& position,
              double propagationDelayMs = 1.0);

    // DFlipFlop overrides evaluate() completely because
    // its behavior is edge-triggered, not combinational
    void evaluate() override;

    // evaluateLogic() is not used by DFlipFlop directly
    // but must be implemented to satisfy the abstract base
    LogicState evaluateLogic(const vector<LogicState>& inputStates) const override;

    string getType() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

private:

    // Tracks the CLK state from the previous evaluate() call
    // so we can detect the LOW -> HIGH rising edge
    LogicState previousClkState;

    // The stored Q value (updated only on rising CLK edge)
    LogicState storedQ;
};