#include "components/sources/ClockGenerator.h"
#include "utils/MathUtils.h"

using namespace std;

// ============================================================
// ClockGenerator.cpp
// ============================================================


// ---- Constructor ----

ClockGenerator::ClockGenerator(const string& id,
                                const string& label,
                                const Vector2D& position,
                                int halfPeriodTicks)
    : Component(id, label, position),
      halfPeriodTicks(halfPeriodTicks),
      tickCounter(0),
      outputHigh(false)
{
    width  = 55.0f;
    height = 35.0f;

    // Single output pin on the right side
    pins.push_back(Pin("output", Vector2D(width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string ClockGenerator::getType() const
{
    return "CLOCK";
}


void ClockGenerator::evaluate()
{
    tickCounter++;

    if (tickCounter >= halfPeriodTicks)
    {
        // Toggle the output state
        outputHigh  = !outputHigh;
        tickCounter = 0;

        // Write new voltage to the output pin
        Pin* outPin = findPin("output");
        if (outPin != nullptr)
        {
            outPin->voltage = outputHigh ? 5.0f : 0.0f;
        }
    }
}


string ClockGenerator::serialize() const
{
    return "CLOCK "
        + id                          + " "
        + label                       + " "
        + to_string(position.x)       + " "
        + to_string(position.y)       + " "
        + to_string(halfPeriodTicks)  + " "
        + to_string(rotation)         + " "
        + (mirrored ? "1" : "0");
}


void ClockGenerator::draw(SDL_Renderer* renderer,
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

    // Outer rectangle body
    SDL_Rect body;
    body.x = (int)(center.x - halfW);
    body.y = (int)(center.y - halfH);
    body.w = (int)w;
    body.h = (int)h;
    SDL_RenderDrawRect(renderer, &body);

    // Square wave symbol drawn inside the body
    // Shows two full cycles: LOW -> HIGH -> LOW -> HIGH -> LOW
    float waveLeft   = center.x - halfW * 0.65f;
    float waveRight  = center.x + halfW * 0.65f;
    float waveWidth  = waveRight - waveLeft;
    float waveHigh   = center.y - halfH * 0.45f;
    float waveLow    = center.y + halfH * 0.45f;
    float segW       = waveWidth / 4.0f;

    // Segment 1: LOW horizontal
    SDL_RenderDrawLine(renderer,
                       (int)waveLeft,           (int)waveLow,
                       (int)(waveLeft + segW),  (int)waveLow);

    // Rise 1: vertical up
    SDL_RenderDrawLine(renderer,
                       (int)(waveLeft + segW),  (int)waveLow,
                       (int)(waveLeft + segW),  (int)waveHigh);

    // Segment 2: HIGH horizontal
    SDL_RenderDrawLine(renderer,
                       (int)(waveLeft + segW),    (int)waveHigh,
                       (int)(waveLeft + 2 * segW), (int)waveHigh);

    // Fall 1: vertical down
    SDL_RenderDrawLine(renderer,
                       (int)(waveLeft + 2 * segW), (int)waveHigh,
                       (int)(waveLeft + 2 * segW), (int)waveLow);

    // Segment 3: LOW horizontal
    SDL_RenderDrawLine(renderer,
                       (int)(waveLeft + 2 * segW), (int)waveLow,
                       (int)(waveLeft + 3 * segW), (int)waveLow);

    // Rise 2: vertical up
    SDL_RenderDrawLine(renderer,
                       (int)(waveLeft + 3 * segW), (int)waveLow,
                       (int)(waveLeft + 3 * segW), (int)waveHigh);

    // Segment 4: HIGH horizontal
    SDL_RenderDrawLine(renderer,
                       (int)(waveLeft + 3 * segW), (int)waveHigh,
                       (int)waveRight,              (int)waveHigh);

    // Output lead line from body right edge to output pin
    SDL_RenderDrawLine(renderer,
                       (int)(center.x + halfW),      (int)center.y,
                       (int)(center.x + halfW * 1.0f), (int)center.y);

    // Pin highlight dot when hovered
    ClockGenerator* self = const_cast<ClockGenerator*>(this);
    Pin* pin = self->findPin("output");
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


// ---- Getters / Setters ----

int ClockGenerator::getHalfPeriodTicks() const
{
    return halfPeriodTicks;
}

void ClockGenerator::setHalfPeriodTicks(int ticks)
{
    if (ticks > 0)
    {
        halfPeriodTicks = ticks;
    }
}

bool ClockGenerator::isOutputHigh() const
{
    return outputHigh;
}

void ClockGenerator::resetClock()
{
    tickCounter = 0;
    outputHigh  = false;

    Pin* outPin = findPin("output");
    if (outPin != nullptr)
    {
        outPin->voltage = 0.0f;
    }
}