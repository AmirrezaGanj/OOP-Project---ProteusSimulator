#pragma once

#include "components/gates/LogicGate.h"

using namespace std;

// ============================================================
// XORGate — output is HIGH when an ODD number of inputs
// are HIGH (for 2 inputs: output HIGH when inputs differ).
// Drawn as an OR gate with an extra curved line on the left.
// ============================================================

class XORGate : public LogicGate
{
public:

    XORGate(const string& id,
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