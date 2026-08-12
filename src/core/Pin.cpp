#include "core/Pin.h"

// ============================================================
// Pin.cpp — implementation of the Pin class.
// See Pin.h for full documentation.
// ============================================================


// ---- Constructor ----

Pin::Pin(const std::string& pinName,
         const Vector2D& offset,
         float radius)
    : name(pinName),
      localOffset(offset),
      worldPosition(offset),
      sensitivityRadius(radius),
      isHighlighted(false),
      isConnected(false),
      voltage(0.0f)
{
}


// ---- checkMouseOver ----

bool Pin::checkMouseOver(const Vector2D& mouseWorldPos)
{
    float dist = MathUtils::distance(worldPosition, mouseWorldPos);

    if (dist <= sensitivityRadius)
    {
        isHighlighted = true;
        return true;
    }

    isHighlighted = false;
    return false;
}


// ---- updateWorldPosition ----

void Pin::updateWorldPosition(const Vector2D& parentWorldPos,
                               float parentRotationDegrees,
                               bool isMirroredHorizontally)
{
    // Start with the local offset
    Vector2D adjustedOffset = localOffset;

    // Apply horizontal mirroring if needed (flip X axis of the offset)
    if (isMirroredHorizontally)
    {
        adjustedOffset.x = -adjustedOffset.x;
    }

    // The pin's unrotated world position is parent origin + adjusted offset
    Vector2D unrotatedWorldPos = parentWorldPos + adjustedOffset;

    // Rotate around the parent's world origin by the parent's rotation angle
    worldPosition = MathUtils::rotatePoint(unrotatedWorldPos,
                                            parentWorldPos,
                                            parentRotationDegrees);
}


// ---- reset ----

void Pin::reset()
{
    voltage = 0.0f;
    isHighlighted = false;
    isConnected = false;
}