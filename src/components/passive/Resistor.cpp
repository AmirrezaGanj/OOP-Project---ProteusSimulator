#include "components/passive/Resistor.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// Resistor.cpp
// ============================================================


// ---- Constructor ----

Resistor::Resistor(const string& id,
                   const string& label,
                   const Vector2D& position,
                   float resistanceOhms)
    : Component(id, label, position),
      resistanceOhms(resistanceOhms),
      currentAmps(0.0f)
{
    width  = 60.0f;
    height = 20.0f;

    // Two terminals: left and right
    pins.push_back(Pin("pin1", Vector2D(-width / 2.0f, 0.0f)));
    pins.push_back(Pin("pin2", Vector2D( width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string Resistor::getType() const
{
    return "RESISTOR";
}


void Resistor::evaluate()
{
    // Passive component — voltage/current is computed by SimulationEngine
    // using nodal analysis. Nothing to do here.
    // currentAmps is set externally by SimulationEngine after solving.
}


string Resistor::serialize() const
{
    // Format: RESISTOR id label x y resistance rotation mirrored
    return "RESISTOR "
        + id                          + " "
        + label                       + " "
        + to_string(position.x)       + " "
        + to_string(position.y)       + " "
        + to_string(resistanceOhms)   + " "
        + to_string(rotation)         + " "
        + (mirrored ? "1" : "0");
}


void Resistor::draw(SDL_Renderer* renderer,
                    const Vector2D& panOffset,
                    float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float h     = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    // Lead line length on each side before the body begins
    float leadLength = halfW * 0.35f;

    // Body rectangle spans the middle portion of the component
    float bodyLeft  = center.x - halfW + leadLength;
    float bodyRight = center.x + halfW - leadLength;
    float bodyTop   = center.y - halfH;
    float bodyBot   = center.y + halfH;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Left lead line: from pin1 to body left edge
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)center.y,
                       (int)bodyLeft,            (int)center.y);

    // Right lead line: from body right edge to pin2
    SDL_RenderDrawLine(renderer,
                       (int)bodyRight,           (int)center.y,
                       (int)(center.x + halfW),  (int)center.y);

    // Body rectangle
    SDL_Rect body;
    body.x = (int)bodyLeft;
    body.y = (int)bodyTop;
    body.w = (int)(bodyRight - bodyLeft);
    body.h = (int)(bodyBot - bodyTop);
    SDL_RenderDrawRect(renderer, &body);

    // Zigzag inside the body to make it clearly look like a resistor
    // Draws 4 peaks inside the rectangle
    int peaks    = 4;
    float segW   = (bodyRight - bodyLeft) / (peaks * 2.0f);
    float peakH  = halfH * 0.7f;

    float prevX = bodyLeft;
    float prevY = center.y;

    for (int i = 0; i < peaks * 2; i++)
    {
        float nextX = bodyLeft + segW * (i + 1);
        float nextY = (i % 2 == 0) ? center.y - peakH : center.y + peakH;

        SDL_RenderDrawLine(renderer,
                           (int)prevX, (int)prevY,
                           (int)nextX, (int)nextY);
        prevX = nextX;
        prevY = nextY;
    }

    // Final line back to center before right lead
    SDL_RenderDrawLine(renderer,
                       (int)prevX,    (int)prevY,
                       (int)bodyRight, (int)center.y);

    // Draw pin highlight dots when hovered
    Resistor* self = const_cast<Resistor*>(this);
    for (int i = 0; i < (int)pins.size(); i++)
    {
        Pin* pin = &self->getPins()[i];
        if (pin->isHighlighted)
        {
            Vector2D screenPin = MathUtils::worldToScreen(pin->worldPosition,
                                                           panOffset, zoom);
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
            SDL_Rect dot;
            dot.x = (int)screenPin.x - 4;
            dot.y = (int)screenPin.y - 4;
            dot.w = 8;
            dot.h = 8;
            SDL_RenderFillRect(renderer, &dot);
        }
    }
}


// ---- Getters / Setters ----

float Resistor::getResistance() const { return resistanceOhms; }

void Resistor::setResistance(float ohms)
{
    if (ohms > 0.0f)
    {
        resistanceOhms = ohms;
    }
}

float Resistor::getCurrent() const { return currentAmps; }

void Resistor::setCurrent(float amps)
{
    currentAmps = amps;
}