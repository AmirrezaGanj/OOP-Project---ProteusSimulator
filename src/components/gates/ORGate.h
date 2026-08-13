#pragma once

#include "components/gates/LogicGate.h"

using namespace std;

// ============================================================
// ORGate — output is HIGH when AT LEAST ONE input is HIGH.
// ============================================================

class ORGate : public LogicGate
{
public:

    ORGate(const string& id,
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