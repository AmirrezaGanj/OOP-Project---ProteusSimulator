#include "components/interactive/Switch.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// Switch.cpp
// ============================================================


// ---- Constructor ----

Switch::Switch(const string& id,
               const string& label,
               const Vector2D& position,
               bool startClosed)
    : Component(id, label, position),
      closed(startClosed)
{
    width  = 60.0f;
    height = 25.0f;

    pins.push_back(Pin("pin1", Vector2D(-width / 2.0f, 0.0f)));
    pins.push_back(Pin("pin2", Vector2D( width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string Switch::getType() const
{
    return "SWITCH";
}


void Switch::evaluate()
{
    if (closed)
    {
        // When closed, both pins share the same voltage.
        // Take the higher voltage (from whatever source drives either side)
        // and apply it to both. SimulationEngine also handles this at the
        // node level, but we set it here as a safety measure.
        Pin* p1 = findPin("pin1");
        Pin* p2 = findPin("pin2");

        if (p1 != nullptr && p2 != nullptr)
        {
            float sharedVoltage = (p1->voltage > p2->voltage)
                                   ? p1->voltage
                                   : p2->voltage;
            p1->voltage = sharedVoltage;
            p2->voltage = sharedVoltage;
        }
    }
    // When open: pins are isolated — no action needed here.
    // The SimulationEngine sees them as separate unconnected nodes.
}


string Switch::serialize() const
{
    return "SWITCH "
        + id                    + " "
        + label                 + " "
        + to_string(position.x) + " "
        + to_string(position.y) + " "
        + (closed ? "1" : "0")  + " "
        + to_string(rotation)   + " "
        + (mirrored ? "1" : "0");
}


void Switch::draw(SDL_Renderer* renderer,
                  const Vector2D& panOffset,
                  float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float h     = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    // Contact node positions (small circles on each terminal)
    float contactRadius = 3.0f * zoom;
    float leftContactX  = center.x - halfW * 0.45f;
    float rightContactX = center.x + halfW * 0.45f;
    float contactY      = center.y;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Left lead line: pin1 -> left contact node
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)center.y,
                       (int)leftContactX,         (int)contactY);

    // Right lead line: right contact node -> pin2
    SDL_RenderDrawLine(renderer,
                       (int)rightContactX,        (int)contactY,
                       (int)(center.x + halfW),   (int)contactY);

    // Draw left contact dot
    SDL_Rect leftDot;
    leftDot.x = (int)(leftContactX - contactRadius);
    leftDot.y = (int)(contactY - contactRadius);
    leftDot.w = (int)(contactRadius * 2);
    leftDot.h = (int)(contactRadius * 2);
    SDL_RenderFillRect(renderer, &leftDot);

    // Draw right contact dot
    SDL_Rect rightDot;
    rightDot.x = (int)(rightContactX - contactRadius);
    rightDot.y = (int)(contactY - contactRadius);
    rightDot.w = (int)(contactRadius * 2);
    rightDot.h = (int)(contactRadius * 2);
    SDL_RenderFillRect(renderer, &rightDot);

    if (closed)
    {
        // CLOSED: draw a straight horizontal arm connecting the two contacts
        SDL_RenderDrawLine(renderer,
                           (int)leftContactX,  (int)contactY,
                           (int)rightContactX, (int)contactY);
    }
    else
    {
        // OPEN: draw the arm raised at an upward angle from the left contact
        float armEndX = rightContactX;
        float armEndY = contactY - halfH * 0.9f;

        SDL_RenderDrawLine(renderer,
                           (int)leftContactX, (int)contactY,
                           (int)armEndX,       (int)armEndY);
    }

    // Pin highlight dots when hovered
    Switch* self = const_cast<Switch*>(this);
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


// ---- Interaction ----

void Switch::handleClick()
{
    closed = !closed;
}


// ---- Getters ----

bool Switch::isClosed() const
{
    return closed;
}