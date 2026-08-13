#include "components/passive/Capacitor.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// Capacitor.cpp
// ============================================================


// ---- Constructor ----

Capacitor::Capacitor(const string& id,
                     const string& label,
                     const Vector2D& position,
                     float capacitanceFarads)
    : Component(id, label, position),
      capacitanceFarads(capacitanceFarads),
      voltageAcross(0.0f)
{
    width  = 50.0f;
    height = 30.0f;

    // Two terminals: left and right
    pins.push_back(Pin("pin1", Vector2D(-width / 2.0f, 0.0f)));
    pins.push_back(Pin("pin2", Vector2D( width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string Capacitor::getType() const
{
    return "CAPACITOR";
}


void Capacitor::evaluate()
{
    // Passive component — handled by SimulationEngine
}


string Capacitor::serialize() const
{
    return "CAPACITOR "
        + id                             + " "
        + label                          + " "
        + to_string(position.x)          + " "
        + to_string(position.y)          + " "
        + to_string(capacitanceFarads)   + " "
        + to_string(rotation)            + " "
        + (mirrored ? "1" : "0");
}


void Capacitor::draw(SDL_Renderer* renderer,
                     const Vector2D& panOffset,
                     float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float h     = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    // Gap between the two capacitor plates (in screen pixels)
    float plateGap  = 6.0f * zoom;
    float plateHeight = halfH;

    // Lead line extends from pin to just before the plate
    float leadEndLeft  = center.x - plateGap / 2.0f;
    float leadEndRight = center.x + plateGap / 2.0f;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Left lead line: pin1 -> left plate
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)center.y,
                       (int)leadEndLeft,          (int)center.y);

    // Right lead line: right plate -> pin2
    SDL_RenderDrawLine(renderer,
                       (int)leadEndRight,         (int)center.y,
                       (int)(center.x + halfW),   (int)center.y);

    // Left plate — vertical line
    SDL_RenderDrawLine(renderer,
                       (int)leadEndLeft, (int)(center.y - plateHeight),
                       (int)leadEndLeft, (int)(center.y + plateHeight));

    // Right plate — vertical line
    SDL_RenderDrawLine(renderer,
                       (int)leadEndRight, (int)(center.y - plateHeight),
                       (int)leadEndRight, (int)(center.y + plateHeight));

    // Draw pin highlight dots when hovered
    Capacitor* self = const_cast<Capacitor*>(this);
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

float Capacitor::getCapacitance() const { return capacitanceFarads; }

void Capacitor::setCapacitance(float farads)
{
    if (farads > 0.0f)
    {
        capacitanceFarads = farads;
    }
}

float Capacitor::getVoltage() const { return voltageAcross; }

void Capacitor::setVoltage(float volts)
{
    voltageAcross = volts;
}