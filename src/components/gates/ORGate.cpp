#include "components/gates/ORGate.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// ORGate.cpp
// ============================================================


ORGate::ORGate(const string& id,
               const string& label,
               const Vector2D& position,
               int numInputs,
               double propagationDelayMs)
    : LogicGate(id, label, position, numInputs, propagationDelayMs)
{
}


string ORGate::getType() const
{
    return "OR";
}


LogicGate::LogicState ORGate::evaluateLogic(const vector<LogicState>& inputStates) const
{
    for (int i = 0; i < (int)inputStates.size(); i++)
    {
        if (inputStates[i] == LogicState::HIGH)
        {
            return LogicState::HIGH;
        }
    }
    return LogicState::LOW;
}


void ORGate::draw(SDL_Renderer* renderer,
                  const Vector2D& panOffset,
                  float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float h     = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    int segments = 16;

    // Left curved input side — slight inward bow
    for (int i = 0; i < segments; i++)
    {
        float t0 = (float)i       / segments;
        float t1 = (float)(i + 1) / segments;

        float x0 = center.x - halfW + halfW * 0.3f * sin(t0 * 3.14159f);
        float y0 = center.y - halfH + h * t0;
        float x1 = center.x - halfW + halfW * 0.3f * sin(t1 * 3.14159f);
        float y1 = center.y - halfH + h * t1;

        SDL_RenderDrawLine(renderer, (int)x0, (int)y0, (int)x1, (int)y1);
    }

    // Top curved edge — from left-top to right tip
    for (int i = 0; i < segments; i++)
    {
        float t0 = (float)i       / segments;
        float t1 = (float)(i + 1) / segments;

        float x0 = center.x - halfW + w * t0;
        float y0 = center.y - halfH + halfH * t0 * t0;
        float x1 = center.x - halfW + w * t1;
        float y1 = center.y - halfH + halfH * t1 * t1;

        SDL_RenderDrawLine(renderer, (int)x0, (int)y0, (int)x1, (int)y1);
    }

    // Bottom curved edge — mirror of top
    for (int i = 0; i < segments; i++)
    {
        float t0 = (float)i       / segments;
        float t1 = (float)(i + 1) / segments;

        float x0 = center.x - halfW + w * t0;
        float y0 = center.y + halfH - halfH * t0 * t0;
        float x1 = center.x - halfW + w * t1;
        float y1 = center.y + halfH - halfH * t1 * t1;

        SDL_RenderDrawLine(renderer, (int)x0, (int)y0, (int)x1, (int)y1);
    }

    drawPinLines(renderer, panOffset, zoom);
    drawPinHighlights(renderer, panOffset, zoom);
}