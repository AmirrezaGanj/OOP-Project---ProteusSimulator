#include "components/gates/NANDGate.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// NANDGate.cpp
// ============================================================


NANDGate::NANDGate(const string& id,
                   const string& label,
                   const Vector2D& position,
                   int numInputs,
                   double propagationDelayMs)
    : LogicGate(id, label, position, numInputs, propagationDelayMs)
{
}


string NANDGate::getType() const
{
    return "NAND";
}


LogicGate::LogicState NANDGate::evaluateLogic(const vector<LogicState>& inputStates) const
{
    // NAND = NOT AND — invert the AND result
    for (int i = 0; i < (int)inputStates.size(); i++)
    {
        if (inputStates[i] == LogicState::LOW)
        {
            return LogicState::HIGH;  // AND would be LOW, so NAND is HIGH
        }
    }
    return LogicState::LOW;           // AND would be HIGH, so NAND is LOW
}


void NANDGate::draw(SDL_Renderer* renderer,
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

    // Draw AND body (same as ANDGate but shifted left to make room for bubble)
    float bodyRightX = center.x + halfW - 8.0f * zoom;

    // Left vertical line
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)(center.y - halfH),
                       (int)(center.x - halfW), (int)(center.y + halfH));

    // Top horizontal line
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)(center.y - halfH),
                       (int)(center.x),          (int)(center.y - halfH));

    // Bottom horizontal line
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)(center.y + halfH),
                       (int)(center.x),          (int)(center.y + halfH));

    // Right D-shaped arc
    int segments = 12;
    float prevX = center.x;
    float prevY = center.y - halfH;

    for (int i = 1; i <= segments; i++)
    {
        float angle = (float)i / segments * 3.14159f;
        float px    = center.x + (halfW - 8.0f * zoom) * sin(angle);
        float py    = center.y - halfH + (h * (float)i / segments);

        if (i == 1)
        {
            prevX = center.x;
            prevY = center.y - halfH;
        }

        SDL_RenderDrawLine(renderer, (int)prevX, (int)prevY, (int)px, (int)py);
        prevX = px;
        prevY = py;
    }

    // Inversion bubble at the output
    int bubbleR = (int)(4.0f * zoom);
    if (bubbleR < 2) bubbleR = 2;

    float bubbleCX = center.x + halfW - bubbleR;
    float bubbleCY = center.y;

    for (int dy = -bubbleR; dy <= bubbleR; dy++)
    {
        int dx = (int)sqrt((double)(bubbleR * bubbleR - dy * dy));
        SDL_RenderDrawLine(renderer,
                           (int)(bubbleCX - dx), (int)(bubbleCY + dy),
                           (int)(bubbleCX + dx), (int)(bubbleCY + dy));
    }

    // Hollow out the bubble center
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    for (int dy = -(bubbleR - 1); dy <= (bubbleR - 1); dy++)
    {
        int dx = (int)sqrt((double)((bubbleR - 1) * (bubbleR - 1) - dy * dy));
        SDL_RenderDrawLine(renderer,
                           (int)(bubbleCX - dx), (int)(bubbleCY + dy),
                           (int)(bubbleCX + dx), (int)(bubbleCY + dy));
    }

    drawPinLines(renderer, panOffset, zoom);
    drawPinHighlights(renderer, panOffset, zoom);
}