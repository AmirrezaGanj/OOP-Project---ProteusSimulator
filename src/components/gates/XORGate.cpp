#include "components/gates/XORGate.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// XORGate.cpp
// ============================================================


XORGate::XORGate(const string& id,
                 const string& label,
                 const Vector2D& position,
                 int numInputs,
                 double propagationDelayMs)
    : LogicGate(id, label, position, numInputs, propagationDelayMs)
{
}


string XORGate::getType() const
{
    return "XOR";
}


LogicGate::LogicState XORGate::evaluateLogic(const vector<LogicState>& inputStates) const
{
    // XOR = true when an odd number of inputs are HIGH
    int highCount = 0;
    for (int i = 0; i < (int)inputStates.size(); i++)
    {
        if (inputStates[i] == LogicState::HIGH)
        {
            highCount++;
        }
    }
    return (highCount % 2 == 1) ? LogicState::HIGH : LogicState::LOW;
}


void XORGate::draw(SDL_Renderer* renderer,
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

    // Same curved body as OR gate
    // Left curved input side
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

    // Top curved edge
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

    // Bottom curved edge
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

    // XOR extra curved line — parallel to the left input side, offset left
    float offset = 6.0f * zoom;
    for (int i = 0; i < segments; i++)
    {
        float t0 = (float)i       / segments;
        float t1 = (float)(i + 1) / segments;

        float x0 = center.x - halfW - offset + halfW * 0.3f * sin(t0 * 3.14159f);
        float y0 = center.y - halfH + h * t0;
        float x1 = center.x - halfW - offset + halfW * 0.3f * sin(t1 * 3.14159f);
        float y1 = center.y - halfH + h * t1;

        SDL_RenderDrawLine(renderer, (int)x0, (int)y0, (int)x1, (int)y1);
    }

    drawPinLines(renderer, panOffset, zoom);
    drawPinHighlights(renderer, panOffset, zoom);
}