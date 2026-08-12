#pragma once

#include "Vector2D.h"
#include <cmath>

// ============================================================
// MathUtils — stateless utility functions used throughout
//
// Key responsibilities:
//   - Distance calculation between two points
//   - Snapping positions to the grid
//   - Rotating a point around a pivot (for component rotation)
//   - Converting between screen space and world/canvas space
// ============================================================

namespace MathUtils {

    // ---- Distance ----

    // Euclidean distance between two 2D points
    inline float distance(const Vector2D& a, const Vector2D& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    // ---- Grid Snapping ----

    // Snaps a single float value to the nearest grid interval
    inline float snapToInterval(float value, float gridSize) {
        return std::round(value / gridSize) * gridSize;
    }

    // Snaps a full Vector2D position to the nearest grid point
    // Used whenever a component is placed or dragged on the canvas
    inline Vector2D snapToGrid(const Vector2D& position, float gridSize) {
        return Vector2D(
            snapToInterval(position.x, gridSize),
            snapToInterval(position.y, gridSize)
        );
    }

    // ---- Coordinate Space Conversion ----

    // Converts a screen-space position (pixels) to canvas/world-space position
    // taking into account pan offset and zoom level.
    // This is needed whenever the user clicks on screen and we need to know
    // where that click lands in the actual circuit world.
    inline Vector2D screenToWorld(const Vector2D& screenPos,
                                  const Vector2D& panOffset,
                                  float zoom) {
        return (screenPos - panOffset) / zoom;
    }

    // Converts a canvas/world-space position back to screen-space.
    // Used when drawing components — we know where they are in world space
    // and need to know where to draw them on screen.
    inline Vector2D worldToScreen(const Vector2D& worldPos,
                                  const Vector2D& panOffset,
                                  float zoom) {
        return (worldPos * zoom) + panOffset;
    }

    // ---- Rotation ----

    // Rotates a point around a given pivot by angleDegrees.
    // Used to recompute pin world-positions when a component is rotated.
    // Positive angle = counter-clockwise.
    inline Vector2D rotatePoint(const Vector2D& point,
                                const Vector2D& pivot,
                                float angleDegrees) {
        float rad = angleDegrees * (3.14159265f / 180.0f);
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        // Translate point relative to pivot
        float translatedX = point.x - pivot.x;
        float translatedY = point.y - pivot.y;

        // Apply 2D rotation matrix
        float rotatedX = translatedX * cosA - translatedY * sinA;
        float rotatedY = translatedX * sinA + translatedY * cosA;

        // Translate back
        return Vector2D(pivot.x + rotatedX, pivot.y + rotatedY);
    }

    // ---- Clamping ----

    // Clamps a float value between min and max
    // Used for keeping zoom level within reasonable bounds
    inline float clamp(float value, float minVal, float maxVal) {
        if (value < minVal) return minVal;
        if (value > maxVal) return maxVal;
        return value;
    }

} // namespace MathUtils