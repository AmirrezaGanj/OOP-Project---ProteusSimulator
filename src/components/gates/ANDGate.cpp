#include "components/gates/ANDGate.h"
#include "utils/MathUtils.h"

using namespace std;

// ============================================================
// ANDGate.cpp
// ============================================================


ANDGate::ANDGate(const string& id,
                 const string& label,
                 const Vector2D& position,
                 int numInputs,
                 double propagationDelayMs)
    : LogicGate(id, label, position, numInputs, propagationDelayMs)
{
}


string ANDGate::getType() const
{
    return "AND";
}


LogicGate::LogicState ANDGate::evaluateLogic(const vector<LogicState>& inputStates) const
{
    for (int i = 0; i < (int)inputStates.size(); i++)
    {
        if (inputStates[i] == LogicState::LOW)
        {
            return LogicState::LOW;
        }
    }
    return LogicState::HIGH;
}


void ANDGate::draw(SDL_Renderer* renderer,
                   const Vector2D& panOffset,
                   float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w = width  * zoom;
    float h = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    // Draw selection highlight first if selected
    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Left side (vertical line)
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)(center.y - halfH),
                       (int)(center.x - halfW), (int)(center.y + halfH));

    // Top side (horizontal line to midpoint)
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)(center.y - halfH),
                       (int)(center.x),          (int)(center.y - halfH));

    // Bottom side (horizontal line to midpoint)
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)(center.y + halfH),
                       (int)(center.x),          (int)(center.y + halfH));

    // Right side — D-shaped curve approximated with line segments
    // Draws a semicircle on the right half of the gate body
    int segments = 12;
    float prevX = center.x;
    float prevY = center.y - halfH;

    for (int i = 1; i <= segments; i++)
    {
        float angle = (float)i / segments * 3.14159f; // 0 to PI
        float px = center.x + halfW * 0.9f * sin(angle);
        float py = center.y - halfH + (h * (float)i / segments);

        // Only use the rightmost arc
        if (i == 1)
        {
            prevX = center.x;
            prevY = center.y - halfH;
        }

        SDL_RenderDrawLine(renderer,
                           (int)prevX, (int)prevY,
                           (int)px,    (int)py);
        prevX = px;
        prevY = py;
    }

    // Draw pin lines and highlights
    drawPinLines(renderer, panOffset, zoom);
    drawPinHighlights(renderer, panOffset, zoom);

    // Draw label below the gate
}