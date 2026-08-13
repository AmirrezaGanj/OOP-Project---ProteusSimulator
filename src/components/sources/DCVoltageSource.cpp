#include "components/sources/DCVoltageSource.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// DCVoltageSource.cpp
// ============================================================


// ---- Constructor ----

DCVoltageSource::DCVoltageSource(const string& id,
                                 const string& label,
                                 const Vector2D& position,
                                 float voltageVolts)
    : Component(id, label, position),
      voltageVolts(voltageVolts)
{
    width  = 50.0f;
    height = 50.0f;

    // Positive terminal on the left (anode)
    // Negative terminal on the right (cathode)
    pins.push_back(Pin("anode",   Vector2D(-width / 2.0f, 0.0f)));
    pins.push_back(Pin("cathode", Vector2D( width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string DCVoltageSource::getType() const
{
    return "DCVOLTAGE";
}


void DCVoltageSource::evaluate()
{
    // Ideal source — drives fixed voltage unconditionally
    Pin* anode   = findPin("anode");
    Pin* cathode = findPin("cathode");

    if (anode != nullptr)
    {
        anode->voltage = voltageVolts;
    }
    if (cathode != nullptr)
    {
        cathode->voltage = 0.0f;
    }
}


string DCVoltageSource::serialize() const
{
    return "DCVOLTAGE "
        + id                    + " "
        + label                 + " "
        + to_string(position.x) + " "
        + to_string(position.y) + " "
        + to_string(voltageVolts) + " "
        + to_string(rotation)   + " "
        + (mirrored ? "1" : "0");
}


void DCVoltageSource::draw(SDL_Renderer* renderer,
                            const Vector2D& panOffset,
                            float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float halfW  = (width  * zoom) / 2.0f;
    float halfH  = (height * zoom) / 2.0f;

    // Circle radius — fits snugly inside the bounding box
    int circleRadius = (int)(halfH * 0.85f);

    // Lead line length from pin to circle edge
    float leadEndLeft  = center.x - circleRadius;
    float leadEndRight = center.x + circleRadius;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Left lead line: anode pin -> circle
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)center.y,
                       (int)leadEndLeft,          (int)center.y);

    // Right lead line: circle -> cathode pin
    SDL_RenderDrawLine(renderer,
                       (int)leadEndRight,         (int)center.y,
                       (int)(center.x + halfW),   (int)center.y);

    // Draw the circle body
    drawCircle(renderer, (int)center.x, (int)center.y, circleRadius);

    // Draw + symbol on the left half (anode side)
    float plusX    = center.x - circleRadius * 0.4f;
    float plusY    = center.y;
    float plusSize = 5.0f * zoom;
    if (plusSize < 3.0f) plusSize = 3.0f;

    SDL_RenderDrawLine(renderer,
                       (int)(plusX - plusSize), (int)plusY,
                       (int)(plusX + plusSize), (int)plusY);
    SDL_RenderDrawLine(renderer,
                       (int)plusX, (int)(plusY - plusSize),
                       (int)plusX, (int)(plusY + plusSize));

    // Draw - symbol on the right half (cathode side)
    float minusX = center.x + circleRadius * 0.4f;
    float minusY = center.y;

    SDL_RenderDrawLine(renderer,
                       (int)(minusX - plusSize), (int)minusY,
                       (int)(minusX + plusSize), (int)minusY);

    // Pin highlight dots when hovered
    DCVoltageSource* self = const_cast<DCVoltageSource*>(this);
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

float DCVoltageSource::getVoltage() const
{
    return voltageVolts;
}

void DCVoltageSource::setVoltage(float volts)
{
    if (volts >= 0.0f)
    {
        voltageVolts = volts;
    }
}


// ---- Private helpers ----

void DCVoltageSource::drawCircle(SDL_Renderer* renderer,
                                  int centerX, int centerY,
                                  int radius) const
{
    // Midpoint circle algorithm — draws the outline of a circle
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
        SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);

        y   += 1;
        err += 1 + 2 * y;

        if (2 * (err - x) + 1 > 0)
        {
            x   -= 1;
            err += 1 - 2 * x;
        }
    }
}