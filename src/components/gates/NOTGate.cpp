#include "components/gates/NOTGate.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// NOTGate.cpp
// ============================================================


NOTGate::NOTGate(const string& id,
                 const string& label,
                 const Vector2D& position,
                 double propagationDelayMs)
    : LogicGate(id, label, position, 1, propagationDelayMs)
{
    // NOT gate is smaller — override the default size
    width  = 40.0f;
    height = 30.0f;
    updateAllPinPositions();
}


string NOTGate::getType() const
{
    return "NOT";
}


LogicGate::LogicState NOTGate::evaluateLogic(const vector<LogicState>& inputStates) const
{
    if (inputStates[0] == LogicState::HIGH) return LogicState::LOW;
    if (inputStates[0] == LogicState::LOW)  return LogicState::HIGH;
    return LogicState::UNDEFINED;
}


void NOTGate::draw(SDL_Renderer* renderer,
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

    // Triangle body: left-top -> left-bottom -> right-tip -> back to left-top
    float leftX  = center.x - halfW;
    float topY   = center.y - halfH;
    float botY   = center.y + halfH;
    float tipX   = center.x + halfW - 6.0f * zoom; // leave room for bubble

    SDL_RenderDrawLine(renderer, (int)leftX, (int)topY, (int)leftX, (int)botY);
    SDL_RenderDrawLine(renderer, (int)leftX, (int)topY, (int)tipX,  (int)center.y);
    SDL_RenderDrawLine(renderer, (int)leftX, (int)botY, (int)tipX,  (int)center.y);

    // Inversion bubble — small circle at the output tip
    int bubbleR = (int)(4.0f * zoom);
    if (bubbleR < 2) bubbleR = 2;

    float bubbleCX = tipX + bubbleR;
    float bubbleCY = center.y;

    for (int dy = -bubbleR; dy <= bubbleR; dy++)
    {
        int dx = (int)sqrt((double)(bubbleR * bubbleR - dy * dy));
        SDL_RenderDrawLine(renderer,
                           (int)(bubbleCX - dx), (int)(bubbleCY + dy),
                           (int)(bubbleCX + dx), (int)(bubbleCY + dy));
    }

    // Redraw outline of bubble in dark color so it's hollow-looking
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    for (int dy = -bubbleR + 1; dy <= bubbleR - 1; dy++)
    {
        int dx = (int)sqrt((double)((bubbleR - 1) * (bubbleR - 1) - dy * dy));
        SDL_RenderDrawLine(renderer,
                           (int)(bubbleCX - dx), (int)(bubbleCY + dy),
                           (int)(bubbleCX + dx), (int)(bubbleCY + dy));
    }

    drawPinLines(renderer, panOffset, zoom);
    drawPinHighlights(renderer, panOffset, zoom);
}