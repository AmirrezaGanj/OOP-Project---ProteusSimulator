#include "components/interactive/LED.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// LED.cpp
// ============================================================


// ---- Constructor ----

LED::LED(const string& id,
         const string& label,
         const Vector2D& position,
         LEDColor color,
         float forwardVoltage)
    : Component(id, label, position),
      color(color),
      forwardVoltageVolts(forwardVoltage),
      lit(false)
{
    width  = 50.0f;
    height = 30.0f;

    // Anode on left, cathode on right
    pins.push_back(Pin("anode",   Vector2D(-width / 2.0f, 0.0f)));
    pins.push_back(Pin("cathode", Vector2D( width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string LED::getType() const
{
    return "LED";
}


void LED::evaluate()
{
    Pin* anode   = findPin("anode");
    Pin* cathode = findPin("cathode");

    if (anode == nullptr || cathode == nullptr)
    {
        lit = false;
        return;
    }

    float voltageAcross = anode->voltage - cathode->voltage;

    // LED lights up when forward biased above threshold
    if (voltageAcross >= forwardVoltageVolts)
    {
        lit = true;
    }
    else
    {
        lit = false;
    }
}


string LED::serialize() const
{
    int colorCode = 0;
    if (color == LEDColor::GREEN) colorCode = 1;
    if (color == LEDColor::BLUE)  colorCode = 2;

    return "LED "
        + id                           + " "
        + label                        + " "
        + to_string(position.x)        + " "
        + to_string(position.y)        + " "
        + to_string(colorCode)         + " "
        + to_string(forwardVoltageVolts) + " "
        + to_string(rotation)          + " "
        + (mirrored ? "1" : "0");
}


void LED::draw(SDL_Renderer* renderer,
               const Vector2D& panOffset,
               float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float h     = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    // Lead line length before the diode body
    float leadLength = halfW * 0.25f;
    float bodyLeft   = center.x - halfW + leadLength;
    float bodyRight  = center.x + halfW - leadLength;
    float bodyWidth  = bodyRight - bodyLeft;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    // Get colors for lit and unlit states
    Uint8 r, g, b;
    getColorRGB(lit, r, g, b);

    // ---- Lead lines ----
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)center.y,
                       (int)bodyLeft,             (int)center.y);

    SDL_RenderDrawLine(renderer,
                       (int)bodyRight,            (int)center.y,
                       (int)(center.x + halfW),   (int)center.y);

    // ---- Diode triangle body ----
    // Triangle points right: tip at bodyRight, base at bodyLeft
    float tipX  = bodyLeft + bodyWidth * 0.75f;
    float baseX = bodyLeft;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    // Triangle outline: base-top -> base-bottom -> tip -> base-top
    SDL_RenderDrawLine(renderer,
                       (int)baseX, (int)(center.y - halfH * 0.8f),
                       (int)baseX, (int)(center.y + halfH * 0.8f));

    SDL_RenderDrawLine(renderer,
                       (int)baseX, (int)(center.y - halfH * 0.8f),
                       (int)tipX,  (int)center.y);

    SDL_RenderDrawLine(renderer,
                       (int)baseX, (int)(center.y + halfH * 0.8f),
                       (int)tipX,  (int)center.y);

    // Fill the triangle when lit (scan lines top to bottom)
    if (lit)
    {
        float triH = halfH * 0.8f * 2.0f;
        int   rows = (int)triH;

        for (int row = 0; row < rows; row++)
        {
            float t    = (float)row / rows;
            float rowY = center.y - halfH * 0.8f + triH * t;
            float rowX = baseX + (tipX - baseX) * t;

            SDL_RenderDrawLine(renderer,
                               (int)baseX, (int)rowY,
                               (int)rowX,  (int)rowY);
        }
    }

    // ---- Cathode bar (vertical line at tip) ----
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    SDL_RenderDrawLine(renderer,
                       (int)tipX, (int)(center.y - halfH * 0.8f),
                       (int)tipX, (int)(center.y + halfH * 0.8f));

    // ---- Light emission arrows when lit ----
    if (lit)
    {
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);

        float arrowBaseX  = tipX - bodyWidth * 0.15f;
        float arrowBaseY  = center.y - halfH * 0.5f;
        float arrowLen    = halfH * 0.6f;
        float arrowSpacing = 7.0f * zoom;

        // Two diagonal arrows pointing up-right
        for (int arrow = 0; arrow < 2; arrow++)
        {
            float ax = arrowBaseX + arrow * arrowSpacing;
            float ay = arrowBaseY - arrow * arrowSpacing * 0.3f;

            // Arrow shaft
            SDL_RenderDrawLine(renderer,
                               (int)ax,
                               (int)ay,
                               (int)(ax + arrowLen * 0.6f),
                               (int)(ay - arrowLen * 0.6f));

            // Arrow head — two short lines
            SDL_RenderDrawLine(renderer,
                               (int)(ax + arrowLen * 0.6f),
                               (int)(ay - arrowLen * 0.6f),
                               (int)(ax + arrowLen * 0.6f - 4.0f * zoom),
                               (int)(ay - arrowLen * 0.6f + 2.0f * zoom));

            SDL_RenderDrawLine(renderer,
                               (int)(ax + arrowLen * 0.6f),
                               (int)(ay - arrowLen * 0.6f),
                               (int)(ax + arrowLen * 0.6f - 2.0f * zoom),
                               (int)(ay - arrowLen * 0.6f + 4.0f * zoom));
        }
    }

    // ---- Pin highlight dots when hovered ----
    LED* self = const_cast<LED*>(this);
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

LED::LEDColor LED::getColor()          const { return color;               }
float         LED::getForwardVoltage() const { return forwardVoltageVolts; }
bool          LED::isLit()             const { return lit;                 }

void LED::setColor(LEDColor newColor)
{
    color = newColor;
}

void LED::setForwardVoltage(float volts)
{
    if (volts > 0.0f)
    {
        forwardVoltageVolts = volts;
    }
}


// ---- Private helpers ----

void LED::getColorRGB(bool bright,
                       Uint8& r, Uint8& g, Uint8& b) const
{
    if (bright)
    {
        // Fully lit — vivid color
        if (color == LEDColor::RED)
        {
            r = 255; g = 30;  b = 30;
        }
        else if (color == LEDColor::GREEN)
        {
            r = 30;  g = 220; b = 30;
        }
        else  // BLUE
        {
            r = 30;  g = 80;  b = 255;
        }
    }
    else
    {
        // Unlit — dim/dark version of the color
        if (color == LEDColor::RED)
        {
            r = 100; g = 20; b = 20;
        }
        else if (color == LEDColor::GREEN)
        {
            r = 20;  g = 80; b = 20;
        }
        else  // BLUE
        {
            r = 20;  g = 30; b = 100;
        }
    }
}