#pragma once

#include <vector>
#include <string>
#include "core/Component.h"
#include "core/Wire.h"
#include "core/Junction.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// Circuit — the central data structure of the entire project.
//
// Owns and manages:
//   - All components placed on the canvas
//   - All wires connecting component pins
//   - All junction dots where wires electrically meet
//
// This is purely a data/logic class. It has NO rendering code.
// The Canvas reads from Circuit to know what to draw.
// The SimulationEngine reads from Circuit to run the simulation.
// ProjectManager reads from Circuit to save/load files.
// ============================================================

class Circuit
{
public:

    Circuit();
    ~Circuit();

    // ---- Component management ----

    // Takes ownership of a heap-allocated component and adds it to the circuit
    void addComponent(Component* component);

    // Removes and deletes a component by its ID.
    // Also removes any wires connected to that component's pins.
    void removeComponent(const string& componentId);

    // Finds and returns a component by ID, returns nullptr if not found
    Component* findComponent(const string& componentId);

    // Returns all components (for iteration in Canvas, SimulationEngine, etc.)
    vector<Component*>& getComponents();

    // ---- Wire management ----

    // Adds a new wire between two pins (identified by component ID + pin name)
    // Returns true if the wire was successfully created
    bool addWire(const string& fromComponentId, const string& fromPinName,
                 const string& toComponentId,   const string& toPinName);

    // Removes a wire by index
    void removeWire(int wireIndex);

    // Removes all wires connected to a specific pin
    void removeWiresConnectedToPin(const string& componentId,
                                   const string& pinName);

    // Returns all wires
    vector<Wire*>& getWires();

    // ---- Junction management ----

    // Adds a junction dot at the given world position
    void addJunction(const Vector2D& worldPosition);

    // Removes a junction at approximately the given position
    void removeJunction(const Vector2D& worldPosition);

    // Returns all junctions
    vector<Junction*>& getJunctions();

    // ---- Hit detection (used by Canvas for mouse clicks) ----

    // Returns the topmost component whose bounding box contains worldPoint,
    // or nullptr if no component is at that position
    Component* getComponentAtPosition(const Vector2D& worldPoint);

    // Returns the index of the wire segment closest to worldPoint
    // within a given pixel tolerance, or -1 if none found
    int getWireIndexAtPosition(const Vector2D& worldPoint, float tolerance);

    // Returns the junction at approximately the given position, or nullptr
    Junction* getJunctionAtPosition(const Vector2D& worldPoint, float tolerance);

    // ---- Circuit state ----

    // Clears everything — all components, wires, junctions.
    // Called when user creates a new project or loads a different one.
    void clear();

    // Returns true if the circuit has no components
    bool isEmpty() const;

    // Generates a unique ID for a new component of a given type
    // e.g. calling this for "RESISTOR" might return "R1", then "R2", etc.
    string generateUniqueId(const string& componentType);

private:

    vector<Component*> components;
    vector<Wire*>      wires;
    vector<Junction*>  junctions;

    // Counters used for unique ID generation per component type
    // e.g. idCounters["RESISTOR"] = 3 means 3 resistors have been created
    vector<pair<string, int>> idCounters;

    // Helper: finds the counter for a given type, returns 0 if not found
    int getIdCounter(const string& componentType);

    // Helper: increments the counter for a given type
    void incrementIdCounter(const string& componentType);
};