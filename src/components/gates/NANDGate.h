#pragma once

#include "components/gates/LogicGate.h"

using namespace std;

// ============================================================
// NANDGate — NOT AND. Output is LOW only when ALL inputs HIGH.
// Drawn as an AND gate body with an inversion bubble at output.
// ============================================================

class NANDGate : public LogicGate
{
public:

    NANDGate(const string& id,
             const string& label,
             const Vector2D& position,
             int numInputs = 2,
             double propagationDelayMs = 1.0);

    LogicState evaluateLogic(const vector<LogicState>& inputStates) const override;

    string getType() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;
};