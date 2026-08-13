#include "components/interactive/PushButton.h"
#include "utils/MathUtils.h"

using namespace std;

// ============================================================
// PushButton.cpp
// ============================================================


// ---- Constructor ----

PushButton::PushButton(const string& id,
                       const string& label,
                       const Vector2D& position)
    : Component(id, label, position),
      pressed(false)
{
    width  = 50.0f;
    height = 35.0f;

    pins.push_back(Pin("pin1", Vector2D(-width / 2.0f, 0.0f)));
    pins.push_back(Pin("pin2", Vector2D( width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- Overrides ----

string PushButton::getType() const
{
    return "BUTTON";
}


void PushButton::evaluate()
{
    Pin* p1 = findPin("pin1");
    Pin* p2 = findPin("pin2");

    if (p1 == nullptr || p2 == nullptr)
    {
        return;
    }

    if (pressed)
    {
        // Momentarily connect: share the higher of the two voltages
        float sharedVoltage = (p1->voltage > p2->voltage)
                               ? p1->voltage
                               : p2->voltage;
        p1->voltage = sharedVoltage;
        p2->voltage = sharedVoltage;
    }
    // When not pressed: pins are isolated — no action needed
}


string PushButton::serialize() const
{
    return "BUTTON "
        + id                    + " "
        + label                 + " "
        + to_string(position.x) + " "
        + to_string(position.y) + " "
        + to_string(rotation)   + " "
        + (mirrored ? "1" : "0");
}


void PushButton::draw(SDL_Renderer* renderer,
                      const Vector2D& panOffset,
                      float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float h     = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    // Contact node X positions (inner terminals)
    float leftContactX  = center.x - halfW * 0.35f;
    float rightContactX = center.x + halfW * 0.35f;
    float contactY      = center.y + halfH * 0.2f;
    float contactRadius = 3.0f * zoom;

    // Button cap sits above the contacts
    float capY       = contactY - halfH * 0.65f;
    float capHalfW   = halfW * 0.55f;

    // How far down the cap moves when pressed
    float pressOffset = pressed ? halfH * 0.25f : 0.0f;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // Left lead: pin1 -> left contact
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - halfW), (int)center.y,
                       (int)leftContactX,         (int)contactY);

    // Right lead: right contact -> pin2
    SDL_RenderDrawLine(renderer,
                       (int)rightContactX,        (int)contactY,
                       (int)(center.x + halfW),   (int)contactY);

    // Left contact vertical line (fixed lower part)
    SDL_RenderDrawLine(renderer,
                       (int)leftContactX, (int)(contactY - halfH * 0.25f),
                       (int)leftContactX, (int)(contactY + halfH * 0.15f));

    // Right contact vertical line (fixed lower part)
    SDL_RenderDrawLine(renderer,
                       (int)rightContactX, (int)(contactY - halfH * 0.25f),
                       (int)rightContactX, (int)(contactY + halfH * 0.15f));

    // Left contact dot
    SDL_Rect leftDot;
    leftDot.x = (int)(leftContactX - contactRadius);
    leftDot.y = (int)(contactY - contactRadius - halfH * 0.25f);
    leftDot.w = (int)(contactRadius * 2);
    leftDot.h = (int)(contactRadius * 2);
    SDL_RenderFillRect(renderer, &leftDot);

    // Right contact dot
    SDL_Rect rightDot;
    rightDot.x = (int)(rightContactX - contactRadius);
    rightDot.y = (int)(contactY - contactRadius - halfH * 0.25f);
    rightDot.w = (int)(contactRadius * 2);
    rightDot.h = (int)(contactRadius * 2);
    SDL_RenderFillRect(renderer, &rightDot);

    // Button cap — moves down when pressed
    float actualCapY = capY + pressOffset;

    // Vertical stem from cap to left contact top
    SDL_RenderDrawLine(renderer,
                       (int)leftContactX,  (int)(contactY - halfH * 0.25f),
                       (int)leftContactX,  (int)(actualCapY));

    SDL_RenderDrawLine(renderer,
                       (int)rightContactX, (int)(contactY - halfH * 0.25f),
                       (int)rightContactX, (int)(actualCapY));

    // Horizontal cap bar
    if (pressed)
    {
        SDL_SetRenderDrawColor(renderer, 80, 80, 200, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    }

    SDL_RenderDrawLine(renderer,
                       (int)(center.x - capHalfW), (int)actualCapY,
                       (int)(center.x + capHalfW), (int)actualCapY);

    // Second line below for thickness of cap
    SDL_RenderDrawLine(renderer,
                       (int)(center.x - capHalfW), (int)(actualCapY + 2.0f * zoom),
                       (int)(center.x + capHalfW), (int)(actualCapY + 2.0f * zoom));

    // Pin highlight dots when hovered
    PushButton* self = const_cast<PushButton*>(this);
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

void PushButton::handleMouseDown()
{
    pressed = true;
}

void PushButton::handleMouseUp()
{
    pressed = false;
}


// ---- Getters ----

bool PushButton::isPressed() const
{
    return pressed;
}