#pragma once

#include <string>
#include "utils/Vector2D.h"
#include "utils/MathUtils.h"

// ============================================================
// Pin — represents a single electrical connection point
// on a component (e.g. the two ends of a Resistor, or the
// input/output of a logic gate).
//
// Each Pin has:
//   - A local offset from its parent component's origin
//   - A world position (recomputed whenever the parent moves
//     or rotates)
//   - A name for identification (e.g. "anode", "input1")
//   - A highlight state for mouse hover feedback during wiring
//   - A connected flag so we know if a wire is attached
// ============================================================

class Pin
{
public:

    // Names, e.g. "VCC", "GND", "Q", "CLK"
    std::string name;

    // Position relative to the parent component's origin (local space)
    // This never changes unless the component definition changes.
    Vector2D localOffset;

    // Absolute position in world/canvas space.
    // Recomputed by the parent component every time it moves or rotates.
    Vector2D worldPosition;

    // How close the mouse must be (in world units) to trigger highlight
    float sensitivityRadius;

    // True when the mouse is hovering close enough to start/end a wire
    bool isHighlighted;

    // True when at least one wire is connected to this pin
    bool isConnected;

    // The voltage currently present at this pin (set by SimulationEngine)
    float voltage;

    // ---- Constructor ----

    Pin(const std::string& pinName,
        const Vector2D& offset,
        float radius = 6.0f);

    // ---- Core Methods ----

    // Call this every frame during the event loop with the current
    // mouse position in world space. Returns true if mouse is close enough.
    bool checkMouseOver(const Vector2D& mouseWorldPos);

    // Recomputes worldPosition based on parent position, parent rotation,
    // and this pin's localOffset. Called by the parent Component whenever
    // it moves, rotates, or is mirrored.
    void updateWorldPosition(const Vector2D& parentWorldPos,
                             float parentRotationDegrees,
                             bool isMirroredHorizontally);

    // Resets voltage and connected state (called when simulation stops)
    void reset();
};