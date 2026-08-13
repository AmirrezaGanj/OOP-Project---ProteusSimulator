#include "core/Wire.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// Wire.cpp — implementation of the Wire class.
// See Wire.h for full documentation.
// ============================================================


// ---- Constructor ----

Wire::Wire(const string& fromComponentId, const string& fromPinName,
           const string& toComponentId,   const string& toPinName,
           const Vector2D& startWorldPos,
           const Vector2D& endWorldPos)
    : fromComponentId(fromComponentId),
      fromPinName(fromPinName),
      toComponentId(toComponentId),
      toPinName(toPinName),
      startPos(startWorldPos),
      endPos(endWorldPos),
      voltage(0.0f)
{
    computeSegments();
}


// ---- Getters ----

string Wire::getFromComponentId() const { return fromComponentId; }
string Wire::getFromPinName()     const { return fromPinName;     }
string Wire::getToComponentId()   const { return toComponentId;   }
string Wire::getToPinName()       const { return toPinName;       }
float  Wire::getVoltage()         const { return voltage;         }

void Wire::setVoltage(float v)
{
    voltage = v;
}

const vector<Vector2D>& Wire::getWaypoints() const
{
    return waypoints;
}


// ---- Update ----

void Wire::updateEndpoints(const Vector2D& newStartWorldPos,
                            const Vector2D& newEndWorldPos)
{
    startPos = newStartWorldPos;
    endPos   = newEndWorldPos;
    computeSegments();
}


// ---- Hit detection ----

bool Wire::isPointNearWire(const Vector2D& worldPoint, float tolerance) const
{
    // Check each segment formed by consecutive waypoints
    for (int i = 0; i < (int)waypoints.size() - 1; i++)
    {
        float dist = distanceToSegment(worldPoint, waypoints[i], waypoints[i + 1]);
        if (dist <= tolerance)
        {
            return true;
        }
    }
    return false;
}

bool Wire::isConnectedToComponent(const string& componentId) const
{
    return (fromComponentId == componentId || toComponentId == componentId);
}


// ---- Rendering ----

void Wire::draw(SDL_Renderer* renderer,
                const Vector2D& panOffset,
                float zoom,
                bool simulationRunning) const
{
    // Choose wire color based on voltage during simulation
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
        // Default schematic color when not simulating -> dark green
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);
    }

    // Draw each segment between consecutive waypoints
    for (int i = 0; i < (int)waypoints.size() - 1; i++)
    {
        Vector2D screenA = MathUtils::worldToScreen(waypoints[i],     panOffset, zoom);
        Vector2D screenB = MathUtils::worldToScreen(waypoints[i + 1], panOffset, zoom);

        SDL_RenderDrawLine(renderer,
                           (int)screenA.x, (int)screenA.y,
                           (int)screenB.x, (int)screenB.y);
    }

    // Draw a small filled circle at each waypoint junction for clarity
    // (only when zoomed in enough to see them)
    if (zoom >= 1.0f)
    {
        for (int i = 1; i < (int)waypoints.size() - 1; i++)
        {
            Vector2D screenP = MathUtils::worldToScreen(waypoints[i], panOffset, zoom);
            SDL_Rect dot;
            dot.x = (int)screenP.x - 2;
            dot.y = (int)screenP.y - 2;
            dot.w = 4;
            dot.h = 4;
            SDL_RenderFillRect(renderer, &dot);
        }
    }
}


// ---- Serialization ----

string Wire::serialize() const
{
    // Format: WIRE fromCompId fromPinName toCompId toPinName
    return "WIRE " + fromComponentId + " " + fromPinName + " "
                   + toComponentId   + " " + toPinName;
}


// ---- Private helpers ----

void Wire::computeSegments()
{
    waypoints.clear();

    // Always build an L-shaped path: horizontal first, then vertical.
    // Three points: start -> corner -> end
    //
    // Example: start=(10,10), end=(50,40)
    //   corner = (50, 10)
    //   path: (10,10) -> (50,10) -> (50,40)

    Vector2D corner(endPos.x, startPos.y);

    waypoints.push_back(startPos);
    waypoints.push_back(corner);
    waypoints.push_back(endPos);
}

float Wire::distanceToSegment(const Vector2D& point,
                               const Vector2D& a,
                               const Vector2D& b) const
{
    // Vector from a to b
    float abX = b.x - a.x;
    float abY = b.y - a.y;

    float lengthSquared = abX * abX + abY * abY;

    // If the segment has zero length, just return distance to the point
    if (lengthSquared == 0.0f)
    {
        return MathUtils::distance(point, a);
    }

    // Project point onto the line segment, clamped to [0, 1]
    float t = ((point.x - a.x) * abX + (point.y - a.y) * abY) / lengthSquared;
    t = MathUtils::clamp(t, 0.0f, 1.0f);

    // Find the closest point on the segment
    Vector2D closest(a.x + t * abX, a.y + t * abY);

    return MathUtils::distance(point, closest);
}