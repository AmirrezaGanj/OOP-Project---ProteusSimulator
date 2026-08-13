#pragma once

#include "components/gates/LogicGate.h"

using namespace std;

// ============================================================
// ANDGate — output is HIGH only when ALL inputs are HIGH.
// Inherits everything from LogicGate.
// Only needs to implement evaluateLogic() and draw().
// ============================================================

class ANDGate : public LogicGate
{
public:

    ANDGate(const string& id,
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