#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Wire — represents an electrical connection between two pins.
//
// A wire is stored as a list of waypoints that form an
// L-shaped (90-degree) path between its two endpoints.
// For example, from (10,10) to (50,40) the waypoints are:
//   (10,10) -> (50,10) -> (50,40)
//   i.e. horizontal first, then vertical.
//
// The wire knows which component IDs and pin names it connects
// so that when a component moves, the wire can update itself,
// and when a component is deleted, Circuit can find and remove
// all wires attached to it.
// ============================================================

class Wire
{
public:

    Wire(const string& fromComponentId, const string& fromPinName,
         const string& toComponentId,   const string& toPinName,
         const Vector2D& startWorldPos,
         const Vector2D& endWorldPos);

    // ---- Getters ----

    string getFromComponentId() const;
    string getFromPinName()     const;
    string getToComponentId()   const;
    string getToPinName()       const;

    // The voltage carried by this wire (set by SimulationEngine)
    float getVoltage() const;
    void  setVoltage(float v);

    // Returns all waypoints forming the routed path
    const vector<Vector2D>& getWaypoints() const;

    // ---- Update ----

    // Called by Circuit whenever a connected component moves or rotates.
    // Recalculates the start/end world positions and re-routes the segments.
    void updateEndpoints(const Vector2D& newStartWorldPos,
                         const Vector2D& newEndWorldPos);

    // ---- Hit detection ----

    // Returns true if worldPoint is within tolerance units of any wire segment.
    // Used by Circuit::getWireIndexAtPosition() for click-to-delete.
    bool isPointNearWire(const Vector2D& worldPoint, float tolerance) const;

    // Returns true if this wire is connected to the given component ID
    // (either as start or end). Used when deleting a component.
    bool isConnectedToComponent(const string& componentId) const;

    // ---- Rendering ----

    // Draws all wire segments. Color depends on voltage state:
    //   HIGH (>=4V)  -> red
    //   LOW  (<=1V)  -> blue
    //   Undefined    -> gray
    //   Not simulating -> dark green (default schematic color)
    void draw(SDL_Renderer* renderer,
              const Vector2D& panOffset,
              float zoom,
              bool simulationRunning) const;

    // ---- Serialization ----

    string serialize() const;

private:

    string fromComponentId;
    string fromPinName;
    string toComponentId;
    string toPinName;

    // The two endpoint positions in world space
    Vector2D startPos;
    Vector2D endPos;

    // The routed waypoints (always 3 points for a single-bend L-shape,
    // start -> corner -> end)
    vector<Vector2D> waypoints;

    // Voltage on this wire, updated each simulation step
    float voltage;

    // ---- Private helpers ----

    // Computes the L-shaped route from startPos to endPos.
    // Goes horizontal first, then vertical.
    // Clears and repopulates the waypoints vector.
    void computeSegments();

    // Returns the minimum distance from worldPoint to a line segment (a -> b)
    float distanceToSegment(const Vector2D& point,
                             const Vector2D& a,
                             const Vector2D& b) const;
};