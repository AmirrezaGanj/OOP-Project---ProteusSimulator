#include "components/interactive/SevenSegment.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// SevenSegment.cpp
// ============================================================

const float SevenSegment::SEGMENT_HIGH_THRESHOLD = 4.0f;


// ---- Constructor ----

SevenSegment::SevenSegment(const string& id,
                            const string& label,
                            const Vector2D& position)
    : Component(id, label, position)
{
    width  = 60.0f;
    height = 90.0f;

    // Initialize all segments to unlit
    for (int i = 0; i < 8; i++)
    {
        segmentLit[i] = false;
    }

    // Pin names in order: a, b, c, d, e, f, g, dp
    // Placed evenly along the bottom edge of the component
    string pinNames[8] = { "a", "b", "c", "d", "e", "f", "g", "dp" };
    float pinSpacing   = width / 9.0f;

    for (int i = 0; i < 8; i++)
    {
        float xOffset = -width / 2.0f + pinSpacing * (i + 1);
        pins.push_back(Pin(pinNames[i],
                           Vector2D(xOffset, height / 2.0f)));
    }

    updateAllPinPositions();
}


// ---- Overrides ----

string SevenSegment::getType() const
{
    return "7SEG";
}


void SevenSegment::evaluate()
{
    // Read each segment pin and update lit state
    string pinNames[8] = { "a", "b", "c", "d", "e", "f", "g", "dp" };

    for (int i = 0; i < 8; i++)
    {
        Pin* pin = findPin(pinNames[i]);

        if (pin != nullptr && pin->isConnected &&
            pin->voltage >= SEGMENT_HIGH_THRESHOLD)
        {
            segmentLit[i] = true;
        }
        else
        {
            segmentLit[i] = false;
        }
    }
}


string SevenSegment::serialize() const
{
    return "7SEG "
        + id                    + " "
        + label                 + " "
        + to_string(position.x) + " "
        + to_string(position.y) + " "
        + to_string(rotation)   + " "
        + (mirrored ? "1" : "0");
}


void SevenSegment::draw(SDL_Renderer* renderer,
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

    // Draw dark background rectangle
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_Rect bg;
    bg.x = (int)(center.x - halfW);
    bg.y = (int)(center.y - halfH);
    bg.w = (int)w;
    bg.h = (int)h;
    SDL_RenderFillRect(renderer, &bg);

    // Draw border
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderDrawRect(renderer, &bg);

    // ---- Segment geometry ----
    // The display area is inset from the border
    float inset       = w * 0.1f;
    float dispLeft    = center.x - halfW + inset;
    float dispRight   = center.x + halfW - inset * 2.0f;
    float dispTop     = center.y - halfH + inset;
    float dispBottom  = center.y + halfH - inset * 2.5f;
    float dispMidY    = (dispTop + dispBottom) / 2.0f;
    float dispWidth   = dispRight - dispLeft;
    float dispHeight  = dispBottom - dispTop;

    float segThick    = w * 0.08f;
    float segLen      = dispWidth  - segThick;
    float segVLen     = dispHeight / 2.0f - segThick;

    // ---- Draw all 7 segments + decimal point ----

    // a — top horizontal
    drawHorizontalSegment(renderer,
                          dispLeft + segThick / 2.0f,
                          dispTop,
                          segLen, segThick,
                          segmentLit[0]);

    // b — upper right vertical
    drawVerticalSegment(renderer,
                        dispRight - segThick,
                        dispTop + segThick / 2.0f,
                        segVLen, segThick,
                        segmentLit[1]);

    // c — lower right vertical
    drawVerticalSegment(renderer,
                        dispRight - segThick,
                        dispMidY + segThick / 2.0f,
                        segVLen, segThick,
                        segmentLit[2]);

    // d — bottom horizontal
    drawHorizontalSegment(renderer,
                          dispLeft + segThick / 2.0f,
                          dispBottom - segThick,
                          segLen, segThick,
                          segmentLit[3]);

    // e — lower left vertical
    drawVerticalSegment(renderer,
                        dispLeft,
                        dispMidY + segThick / 2.0f,
                        segVLen, segThick,
                        segmentLit[4]);

    // f — upper left vertical
    drawVerticalSegment(renderer,
                        dispLeft,
                        dispTop + segThick / 2.0f,
                        segVLen, segThick,
                        segmentLit[5]);

    // g — middle horizontal
    drawHorizontalSegment(renderer,
                          dispLeft + segThick / 2.0f,
                          dispMidY - segThick / 2.0f,
                          segLen, segThick,
                          segmentLit[6]);

    // dp — decimal point (small square bottom right)
    float dpX = dispRight + segThick * 0.3f;
    float dpY = dispBottom - segThick;
    float dpSize = segThick * 0.9f;

    if (segmentLit[7])
    {
        SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 60, 15, 15, 255);
    }

    SDL_Rect dpRect;
    dpRect.x = (int)dpX;
    dpRect.y = (int)dpY;
    dpRect.w = (int)dpSize;
    dpRect.h = (int)dpSize;
    SDL_RenderFillRect(renderer, &dpRect);

    // Draw pin lead lines from bottom of body to pin world positions
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    SevenSegment* self = const_cast<SevenSegment*>(this);
    for (int i = 0; i < (int)pins.size(); i++)
    {
        Pin* pin = &self->getPins()[i];

        Vector2D screenPin = MathUtils::worldToScreen(pin->worldPosition,
                                                       panOffset, zoom);

        SDL_RenderDrawLine(renderer,
                           (int)screenPin.x, (int)(center.y + halfH),
                           (int)screenPin.x, (int)screenPin.y);

        // Pin highlight dot when hovered
        if (pin->isHighlighted)
        {
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


// ---- Private helpers ----

void SevenSegment::drawHorizontalSegment(SDL_Renderer* renderer,
                                          float startX, float startY,
                                          float segLength, float segThickness,
                                          bool lit) const
{
    if (lit)
    {
        SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 60, 15, 15, 255);
    }

    SDL_Rect seg;
    seg.x = (int)startX;
    seg.y = (int)startY;
    seg.w = (int)segLength;
    seg.h = (int)segThickness;
    SDL_RenderFillRect(renderer, &seg);
}

void SevenSegment::drawVerticalSegment(SDL_Renderer* renderer,
                                        float startX, float startY,
                                        float segLength, float segThickness,
                                        bool lit) const
{
    if (lit)
    {
        SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 60, 15, 15, 255);
    }

    SDL_Rect seg;
    seg.x = (int)startX;
    seg.y = (int)startY;
    seg.w = (int)segThickness;
    seg.h = (int)segLength;
    SDL_RenderFillRect(renderer, &seg);
}