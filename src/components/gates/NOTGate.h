#pragma once

#include "components/gates/LogicGate.h"

using namespace std;

// ============================================================
// NOTGate — single input, output is the inverse of input.
// Drawn as a triangle pointing right with a small circle
// (bubble) at the output tip.
// ============================================================

class NOTGate : public LogicGate
{
public:

    // NOT gate always has exactly 1 input
    NOTGate(const string& id,
            const string& label,
            const Vector2D& position,
            double propagationDelayMs = 1.0);

    LogicState evaluateLogic(const vector<LogicState>& inputStates) const override;

    string getType() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;
};