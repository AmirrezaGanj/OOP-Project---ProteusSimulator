#include "core/Component.h"
#include "utils/MathUtils.h"

using namespace std;

// ============================================================
// Component.cpp — implementation of the abstract base class.
// See Component.h for full documentation.
// ============================================================


// ---- Constructor ----

Component::Component(const string& id,
                     const string& label,
                     const Vector2D& position)
    : id(id),
      label(label),
      position(position),
      rotation(0.0f),
      mirrored(false),
      selected(false),
      width(40.0f),
      height(20.0f)
{
}


// ---- Getters ----

string Component::getId() const
{
    return id;
}

string Component::getLabel() const
{
    return label;
}

Vector2D Component::getPosition() const
{
    return position;
}

float Component::getRotation() const
{
    return rotation;
}

bool Component::getMirrored() const
{
    return mirrored;
}

bool Component::isSelected() const
{
    return selected;
}

vector<Pin>& Component::getPins()
{
    return pins;
}


// ---- Setters ----

void Component::setLabel(const string& newLabel)
{
    label = newLabel;
}

void Component::setPosition(const Vector2D& newPosition)
{
    position = newPosition;
    updateAllPinPositions();
}

void Component::setSelected(bool sel)
{
    selected = sel;
}


// ---- Transformation methods ----

void Component::moveTo(const Vector2D& newPosition)
{
    position = newPosition;
    updateAllPinPositions();
}

void Component::rotate90()
{
    rotation = fmod(rotation + 90.0f, 360.0f);
    updateAllPinPositions();
}

void Component::mirrorHorizontal()
{
    mirrored = !mirrored;
    updateAllPinPositions();
}


// ---- Hit detection ----

bool Component::isPointInside(const Vector2D& worldPoint) const
{
    float halfW = width / 2.0f;
    float halfH = height / 2.0f;

    return (worldPoint.x >= position.x - halfW &&
            worldPoint.x <= position.x + halfW &&
            worldPoint.y >= position.y - halfH &&
            worldPoint.y <= position.y + halfH);
}

bool Component::overlapsRect(const SDL_Rect& selectionRect,
                              float zoom,
                              const Vector2D& panOffset) const
{
    // Convert component world position to screen position
    Vector2D screenPos = MathUtils::worldToScreen(position, panOffset, zoom);

    float halfW = (width * zoom) / 2.0f;
    float halfH = (height * zoom) / 2.0f;

    SDL_Rect componentRect;
    componentRect.x = (int)(screenPos.x - halfW);
    componentRect.y = (int)(screenPos.y - halfH);
    componentRect.w = (int)(width * zoom);
    componentRect.h = (int)(height * zoom);

    // SDL_HasIntersection checks if two rects overlap
    return SDL_HasIntersection(&componentRect, &selectionRect) == SDL_TRUE;
}


// ---- Simulation helpers ----

Pin* Component::findPin(const string& pinName)
{
    for (int i = 0; i < pins.size(); i++)
    {
        if (pins[i].name == pinName)
        {
            return &pins[i];
        }
    }
    return nullptr;
}

void Component::resetPins()
{
    for (int i = 0; i < pins.size(); i++)
    {
        pins[i].reset();
    }
}


// ---- Protected helpers ----

void Component::updateAllPinPositions()
{
    for (int i = 0; i < pins.size(); i++)
    {
        pins[i].updateWorldPosition(position, rotation, mirrored);
    }
}

void Component::drawSelectionHighlight(SDL_Renderer* renderer,
                                        const Vector2D& panOffset,
                                        float zoom) const
{
    Vector2D screenPos = MathUtils::worldToScreen(position, panOffset, zoom);

    float halfW = (width * zoom) / 2.0f + 4.0f;
    float halfH = (height * zoom) / 2.0f + 4.0f;

    SDL_Rect highlightRect;
    highlightRect.x = (int)(screenPos.x - halfW);
    highlightRect.y = (int)(screenPos.y - halfH);
    highlightRect.w = (int)(halfW * 2.0f);
    highlightRect.h = (int)(halfH * 2.0f);

    // Draw highlight in cyan
    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
    SDL_RenderDrawRect(renderer, &highlightRect);
}