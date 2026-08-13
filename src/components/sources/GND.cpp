#include "components/sources/GND.h"
#include "utils/MathUtils.h"

using namespace std;

// ============================================================
// GND.cpp
// ============================================================


// ---- Constructor ----

GND::GND(const string& id,
         const string& label,
         const Vector2D& position)
    : Component(id, label, position)
{
    width  = 30.0f;
    height = 30.0f;

    // Single pin at the top center — this is where wires connect
    pins.push_back(Pin("pin1", Vector2D(0.0f, -height / 2.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string GND::getType() const
{
    return "GND";
}


void GND::evaluate()
{
    // Ground always forces its pin to exactly 0V
    Pin* pin = findPin("pin1");
    if (pin != nullptr)
    {
        pin->voltage = 0.0f;
    }
}


string GND::serialize() const
{
    return "GND "
        + id                    + " "
        + label                 + " "
        + to_string(position.x) + " "
        + to_string(position.y) + " "
        + to_string(rotation)   + " "
        + (mirrored ? "1" : "0");
}


void GND::draw(SDL_Renderer* renderer,
               const Vector2D& panOffset,
               float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float h     = height * zoom;
    float halfH = h / 2.0f;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Vertical lead line from pin (top) down to first horizontal bar
    float pinY       = center.y - halfH;
    float firstBarY  = center.y - halfH * 0.1f;

    SDL_RenderDrawLine(renderer,
                       (int)center.x, (int)pinY,
                       (int)center.x, (int)firstBarY);

    // Three horizontal bars of decreasing width
    // Bar 1 — widest
    float bar1HalfW = halfH * 0.8f;
    float bar1Y     = firstBarY;

    SDL_RenderDrawLine(renderer,
                       (int)(center.x - bar1HalfW), (int)bar1Y,
                       (int)(center.x + bar1HalfW), (int)bar1Y);

    // Bar 2 — medium, spaced below bar 1
    float bar2HalfW = bar1HalfW * 0.65f;
    float bar2Y     = bar1Y + halfH * 0.35f;

    SDL_RenderDrawLine(renderer,
                       (int)(center.x - bar2HalfW), (int)bar2Y,
                       (int)(center.x + bar2HalfW), (int)bar2Y);

    // Bar 3 — narrowest, spaced below bar 2
    float bar3HalfW = bar1HalfW * 0.3f;
    float bar3Y     = bar2Y + halfH * 0.35f;

    SDL_RenderDrawLine(renderer,
                       (int)(center.x - bar3HalfW), (int)bar3Y,
                       (int)(center.x + bar3HalfW), (int)bar3Y);

    // Pin highlight dot when hovered
    GND* self = const_cast<GND*>(this);
    Pin* pin  = self->findPin("pin1");
    if (pin != nullptr && pin->isHighlighted)
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