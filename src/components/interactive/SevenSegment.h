#pragma once

#include <string>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// SevenSegment — a 7-segment LED display component.
//
// Pins (8 total):
//   a  = top horizontal segment
//   b  = upper right vertical segment
//   c  = lower right vertical segment
//   d  = bottom horizontal segment
//   e  = lower left vertical segment
//   f  = upper left vertical segment
//   g  = middle horizontal segment
//   dp = decimal point (optional, bottom right dot)
//
// Standard segment layout:
//    _
//   |_|
//   |_|.
//
//    a
//   f b
//    g
//   e c
//    d  dp
//
// Behavior:
//   Each segment pin is independent. When a pin voltage
//   is >= HIGH threshold, that segment lights up bright red.
//   When LOW or floating, it is drawn as a dim dark red bar.
//
// Pins are arranged along the bottom of the component body.
// ============================================================

class SevenSegment : public Component
{
public:

    SevenSegment(const string& id,
                 const string& label,
                 const Vector2D& position);

    // ---- Overrides from Component ----

    // Reads each pin's voltage and updates the lit state of each segment
    void evaluate() override;

    string getType()   const override;
    string serialize() const override;

    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom) const override;

private:

    // Lit state for each of the 8 segments
    // Index: 0=a, 1=b, 2=c, 3=d, 4=e, 5=f, 6=g, 7=dp
    bool segmentLit[8];

    // Voltage threshold above which a segment is considered HIGH
    static const float SEGMENT_HIGH_THRESHOLD;

    // ---- Drawing helpers ----

    // Draws a single horizontal segment (a, d, g)
    void drawHorizontalSegment(SDL_Renderer* renderer,
                                float startX, float startY,
                                float segLength, float segThickness,
                                bool lit) const;

    // Draws a single vertical segment (b, c, e, f)
    void drawVerticalSegment(SDL_Renderer* renderer,
                              float startX, float startY,
                              float segLength, float segThickness,
                              bool lit) const;
};