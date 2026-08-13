#include "components/passive/Inductor.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// Inductor.cpp
// ============================================================

// Fix: The following line wasn't needed.
//const int Inductor::COIL_BUMPS = 4;


// ---- Constructor ----

Inductor::Inductor(const string& id,
                   const string& label,
                   const Vector2D& position,
                   float inductanceHenries)
    : Component(id, label, position),
      inductanceHenries(inductanceHenries),
      currentAmps(0.0f)
{
    width  = 70.0f;
    height = 25.0f;

    // Two terminals: left and right
    pins.push_back(Pin("pin1", Vector2D(-width / 2.0f, 0.0f)));
    pins.push_back(Pin("pin2", Vector2D( width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string Inductor::getType() const
{
    return "INDUCTOR";
}


void Inductor::evaluate()
{
    // Passive component — handled by SimulationEngine
}


string Inductor::serialize() const
{
    return "INDUCTOR "
        + id                            + " "
        + label                         + " "
        + to_string(position.x)         + " "
        + to_string(position.y)         + " "
        + to_string(inductanceHenries)  + " "
        + to_string(rotation)           + " "
        + (mirrored ? "1" : "0");
}


void Inductor::draw(SDL_Renderer* renderer,
                    const Vector2D& panOffset,
                    float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float halfW = w / 2.0f;

    // Lead line length before coil starts
    float leadLength = halfW * 0.2f;

    float coilStartX = center.x - halfW + leadLength;
    float coilEndX   = center.x + halfW - leadLength;
    float coilWidth  = coilEndX - coilStartX;

    // Each bump occupies an equal share of the coil width
    float bumpWidth  = coilWidth / COIL_BUMPS;
    float bumpRadius = bumpWidth / 2.0f;

    // Bumps are drawn above the center line
    float bumpTopY   = center.y - bumpRadius;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Left lead line
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)center.y,
                       (int)coilStartX,          (int)center.y);

    // Right lead line
    SDL_RenderDrawLine(renderer,
                       (int)coilEndX,            (int)center.y,
                       (int)(center.x + halfW),  (int)center.y);

    // Draw each coil bump as a semicircle arc above the line
    int arcSegments = 12;

    for (int bump = 0; bump < COIL_BUMPS; bump++)
    {
        float bumpCenterX = coilStartX + bumpWidth * bump + bumpRadius;

        for (int seg = 0; seg < arcSegments; seg++)
        {
            // Angle goes from PI (left of bump) to 0 (right of bump)
            // This draws the upper half of a circle (above the line)
            float angle0 = 3.14159f - (3.14159f * (float)seg       / arcSegments);
            float angle1 = 3.14159f - (3.14159f * (float)(seg + 1) / arcSegments);

            float x0 = bumpCenterX + bumpRadius * cos(angle0);
            float y0 = center.y    - bumpRadius * sin(angle0);
            float x1 = bumpCenterX + bumpRadius * cos(angle1);
            float y1 = center.y    - bumpRadius * sin(angle1);

            SDL_RenderDrawLine(renderer,
                               (int)x0, (int)y0,
                               (int)x1, (int)y1);
        }
    }

    // Draw pin highlight dots when hovered
    Inductor* self = const_cast<Inductor*>(this);
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

float Inductor::getInductance() const { return inductanceHenries; }

void Inductor::setInductance(float henries)
{
    if (henries > 0.0f)
    {
        inductanceHenries = henries;
    }
}

float Inductor::getCurrent() const { return currentAmps; }

void Inductor::setCurrent(float amps)
{
    currentAmps = amps;
}