#include "components/sources/Battery.h"
#include "utils/MathUtils.h"

using namespace std;

// ============================================================
// Battery.cpp
// ============================================================

//const int Battery::CELL_COUNT = 3;


// ---- Constructor ----

Battery::Battery(const string& id,
                 const string& label,
                 const Vector2D& position,
                 float emfVolts,
                 float internalResistance)
    : Component(id, label, position),
      emfVolts(emfVolts),
      internalResistanceOhms(internalResistance)
{
    width  = 70.0f;
    height = 30.0f;

    // Positive terminal on the left (anode)
    // Negative terminal on the right (cathode)
    pins.push_back(Pin("anode",   Vector2D(-width / 2.0f, 0.0f)));
    pins.push_back(Pin("cathode", Vector2D( width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string Battery::getType() const
{
    return "BATTERY";
}


void Battery::evaluate()
{
    // Drive anode to EMF voltage and cathode to 0V.
    // SimulationEngine will apply internal resistance correction
    // when computing actual terminal voltages under load.
    Pin* anode   = findPin("anode");
    Pin* cathode = findPin("cathode");

    if (anode != nullptr)
    {
        anode->voltage = emfVolts;
    }
    if (cathode != nullptr)
    {
        cathode->voltage = 0.0f;
    }
}


string Battery::serialize() const
{
    return "BATTERY "
        + id                                 + " "
        + label                              + " "
        + to_string(position.x)              + " "
        + to_string(position.y)              + " "
        + to_string(emfVolts)                + " "
        + to_string(internalResistanceOhms)  + " "
        + to_string(rotation)                + " "
        + (mirrored ? "1" : "0");
}


void Battery::draw(SDL_Renderer* renderer,
                   const Vector2D& panOffset,
                   float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float h     = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    // Lead line length before the cell stack begins
    float leadLength = halfW * 0.2f;

    float cellStackLeft  = center.x - halfW + leadLength;
    float cellStackRight = center.x + halfW - leadLength;
    float cellStackWidth = cellStackRight - cellStackLeft;

    // Each cell pair occupies equal width inside the stack
    float cellPairWidth  = cellStackWidth / CELL_COUNT;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Left lead line: anode pin -> cell stack
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)center.y,
                       (int)cellStackLeft,        (int)center.y);

    // Right lead line: cell stack -> cathode pin
    SDL_RenderDrawLine(renderer,
                       (int)cellStackRight,       (int)center.y,
                       (int)(center.x + halfW),   (int)center.y);

    // Draw each cell as a long bar (negative) and short bar (positive)
    // Alternating from left (positive/anode side) to right (cathode side)
    for (int i = 0; i < CELL_COUNT; i++)
    {
        float cellLeft = cellStackLeft + cellPairWidth * i;

        // Short bar = positive plate (closer to anode)
        float shortBarX   = cellLeft + cellPairWidth * 0.3f;
        float shortBarH   = halfH * 0.6f;

        SDL_RenderDrawLine(renderer,
                           (int)shortBarX, (int)(center.y - shortBarH),
                           (int)shortBarX, (int)(center.y + shortBarH));

        // Long bar = negative plate (closer to cathode)
        float longBarX    = cellLeft + cellPairWidth * 0.7f;
        float longBarH    = halfH;

        SDL_RenderDrawLine(renderer,
                           (int)longBarX, (int)(center.y - longBarH),
                           (int)longBarX, (int)(center.y + longBarH));

        // Horizontal connector between the two bars at center
        SDL_RenderDrawLine(renderer,
                           (int)shortBarX, (int)center.y,
                           (int)longBarX,  (int)center.y);
    }

    // Draw + label near the anode (left side)
    // (Full text rendering via SDL2_ttf handled in a future FontRenderer)
    // For now draw a small + using two lines
    float plusX = center.x - halfW + leadLength * 0.5f;
    float plusY = center.y - halfH * 0.8f;
    float plusSize = 4.0f * zoom;

    SDL_RenderDrawLine(renderer,
                       (int)(plusX - plusSize), (int)plusY,
                       (int)(plusX + plusSize), (int)plusY);
    SDL_RenderDrawLine(renderer,
                       (int)plusX, (int)(plusY - plusSize),
                       (int)plusX, (int)(plusY + plusSize));

    // Draw - label near the cathode (right side)
    float minusX = center.x + halfW - leadLength * 0.5f;
    float minusY = center.y - halfH * 0.8f;

    SDL_RenderDrawLine(renderer,
                       (int)(minusX - plusSize), (int)minusY,
                       (int)(minusX + plusSize), (int)minusY);

    // Pin highlight dots when hovered
    Battery* self = const_cast<Battery*>(this);
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

float Battery::getEmf() const
{
    return emfVolts;
}

float Battery::getInternalResistance() const
{
    return internalResistanceOhms;
}

void Battery::setEmf(float volts)
{
    if (volts >= 0.0f)
    {
        emfVolts = volts;
    }
}

void Battery::setInternalResistance(float ohms)
{
    if (ohms >= 0.0f)
    {
        internalResistanceOhms = ohms;
    }
}