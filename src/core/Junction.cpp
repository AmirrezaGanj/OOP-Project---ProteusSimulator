#include "core/Junction.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// Junction.cpp — implementation of the Junction class.
// See Junction.h for full documentation.
// ============================================================


// ---- Constructor ----

Junction::Junction(const Vector2D& worldPosition)
    : position(worldPosition),
      radius(5.0f),
      voltage(0.0f),
      selected(false)
{
}


// ---- Getters ----

Vector2D Junction::getPosition() const { return position;  }
float    Junction::getVoltage()  const { return voltage;   }
bool     Junction::isSelected()  const { return selected;  }


// ---- Setters ----

void Junction::setVoltage(float v)
{
    voltage = v;
}

void Junction::setSelected(bool sel)
{
    selected = sel;
}


// ---- Hit detection ----

bool Junction::isPointInside(const Vector2D& worldPoint) const
{
    return MathUtils::distance(position, worldPoint) <= radius;
}


// ---- Rendering ----

void Junction::draw(SDL_Renderer* renderer,
                    const Vector2D& panOffset,
                    float zoom,
                    bool simulationRunning) const
{
    // Choose color based on voltage, same logic as Wire
    if (simulationRunning)
    {
        if (voltage >= 4.0f)
        {
            // Logic HIGH -> red
            SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
        }
        else if (voltage <= 1.0f && voltage >= 0.0f)
        {
            // Logic LOW -> blue
            SDL_SetRenderDrawColor(renderer, 50, 100, 220, 255);
        }
        else
        {
            // Undefined / floating -> gray
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        }
    }
    else
    {
        // Idle schematic color -> dark green
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);
    }

    // Convert world position to screen position
    Vector2D screenPos = MathUtils::worldToScreen(position, panOffset, zoom);

    // Scale radius with zoom so it looks consistent at all zoom levels
    int screenRadius = (int)(radius * zoom);
    if (screenRadius < 3) screenRadius = 3;  // minimum visible size

    drawFilledCircle(renderer, (int)screenPos.x, (int)screenPos.y, screenRadius);

    // If selected, draw a highlight ring around it
    if (selected)
    {
        SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
        drawFilledCircle(renderer,
                         (int)screenPos.x,
                         (int)screenPos.y,
                         screenRadius + 3);

        // Redraw the junction on top of the highlight ring
        if (simulationRunning)
        {
            if (voltage >= 4.0f)
                SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
            else if (voltage <= 1.0f)
                SDL_SetRenderDrawColor(renderer, 50, 100, 220, 255);
            else
                SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);
        }

        drawFilledCircle(renderer, (int)screenPos.x, (int)screenPos.y, screenRadius);
    }
}


// ---- Serialization ----

string Junction::serialize() const
{
    // Format: JUNCTION x y
    return "JUNCTION " + to_string(position.x) + " " + to_string(position.y);
}


// ---- Simulation ----

void Junction::reset()
{
    voltage  = 0.0f;
    selected = false;
}


// ---- Private helpers ----

void Junction::drawFilledCircle(SDL_Renderer* renderer,
                                 int centerX, int centerY,
                                 int r) const
{
    // SDL2 has no native filled circle function.
    // We draw it by filling horizontal scan lines from top to bottom.
    for (int dy = -r; dy <= r; dy++)
    {
        // Width of the circle at this vertical offset
        int dx = (int)sqrt((double)(r * r - dy * dy));

        SDL_RenderDrawLine(renderer,
                           centerX - dx, centerY + dy,
                           centerX + dx, centerY + dy);
    }
}