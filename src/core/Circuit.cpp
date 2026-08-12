#include "core/Circuit.h"
#include "utils/MathUtils.h"
#include <cmath>

using namespace std;

// ============================================================
// Circuit.cpp — implementation of the Circuit class.
// See Circuit.h for full documentation.
// ============================================================


// ---- Constructor / Destructor ----

Circuit::Circuit()
{
}

Circuit::~Circuit()
{
    clear();
}


// ---- Component management ----

void Circuit::addComponent(Component* component)
{
    if (component != nullptr)
    {
        components.push_back(component);
    }
}

void Circuit::removeComponent(const string& componentId)
{
    // First remove all wires that touch any pin of this component
    for (int i = (int)wires.size() - 1; i >= 0; i--)
    {
        if (wires[i]->isConnectedToComponent(componentId))
        {
            delete wires[i];
            wires.erase(wires.begin() + i);
        }
    }

    // Now remove and delete the component itself
    for (int i = 0; i < (int)components.size(); i++)
    {
        if (components[i]->getId() == componentId)
        {
            delete components[i];
            components.erase(components.begin() + i);
            return;
        }
    }
}

Component* Circuit::findComponent(const string& componentId)
{
    for (int i = 0; i < (int)components.size(); i++)
    {
        if (components[i]->getId() == componentId)
        {
            return components[i];
        }
    }
    return nullptr;
}

vector<Component*>& Circuit::getComponents()
{
    return components;
}


// ---- Wire management ----

bool Circuit::addWire(const string& fromComponentId, const string& fromPinName,
                      const string& toComponentId,   const string& toPinName)
{
    Component* fromComp = findComponent(fromComponentId);
    Component* toComp   = findComponent(toComponentId);

    if (fromComp == nullptr || toComp == nullptr)
    {
        return false;
    }

    Pin* fromPin = fromComp->findPin(fromPinName);
    Pin* toPin   = toComp->findPin(toPinName);

    if (fromPin == nullptr || toPin == nullptr)
    {
        return false;
    }

    // Create the wire and store the endpoint info
    Wire* newWire = new Wire(fromComponentId, fromPinName,
                             toComponentId,   toPinName,
                             fromPin->worldPosition,
                             toPin->worldPosition);

    fromPin->isConnected = true;
    toPin->isConnected   = true;

    wires.push_back(newWire);
    return true;
}

void Circuit::removeWire(int wireIndex)
{
    if (wireIndex < 0 || wireIndex >= (int)wires.size())
    {
        return;
    }

    Wire* w = wires[wireIndex];

    // Mark the pins as disconnected if no other wire still connects them
    string fromCompId  = w->getFromComponentId();
    string fromPinName = w->getFromPinName();
    string toCompId    = w->getToComponentId();
    string toPinName   = w->getToPinName();

    delete wires[wireIndex];
    wires.erase(wires.begin() + wireIndex);

    // Check if those pins still have any other wire — if not, mark disconnected
    auto pinStillConnected = [&](const string& compId, const string& pinName) -> bool
    {
        for (int i = 0; i < (int)wires.size(); i++)
        {
            if ((wires[i]->getFromComponentId() == compId &&
                 wires[i]->getFromPinName()     == pinName) ||
                (wires[i]->getToComponentId()   == compId &&
                 wires[i]->getToPinName()       == pinName))
            {
                return true;
            }
        }
        return false;
    };

    Component* fromComp = findComponent(fromCompId);
    if (fromComp != nullptr)
    {
        Pin* fromPin = fromComp->findPin(fromPinName);
        if (fromPin != nullptr && !pinStillConnected(fromCompId, fromPinName))
        {
            fromPin->isConnected = false;
        }
    }

    Component* toComp = findComponent(toCompId);
    if (toComp != nullptr)
    {
        Pin* toPin = toComp->findPin(toPinName);
        if (toPin != nullptr && !pinStillConnected(toCompId, toPinName))
        {
            toPin->isConnected = false;
        }
    }
}

void Circuit::removeWiresConnectedToPin(const string& componentId,
                                         const string& pinName)
{
    for (int i = (int)wires.size() - 1; i >= 0; i--)
    {
        if ((wires[i]->getFromComponentId() == componentId &&
             wires[i]->getFromPinName()     == pinName) ||
            (wires[i]->getToComponentId()   == componentId &&
             wires[i]->getToPinName()       == pinName))
        {
            removeWire(i);
        }
    }
}

vector<Wire*>& Circuit::getWires()
{
    return wires;
}


// ---- Junction management ----

void Circuit::addJunction(const Vector2D& worldPosition)
{
    junctions.push_back(new Junction(worldPosition));
}

void Circuit::removeJunction(const Vector2D& worldPosition)
{
    float tolerance = 8.0f;

    for (int i = 0; i < (int)junctions.size(); i++)
    {
        if (MathUtils::distance(junctions[i]->getPosition(), worldPosition) <= tolerance)
        {
            delete junctions[i];
            junctions.erase(junctions.begin() + i);
            return;
        }
    }
}

vector<Junction*>& Circuit::getJunctions()
{
    return junctions;
}


// ---- Hit detection ----

Component* Circuit::getComponentAtPosition(const Vector2D& worldPoint)
{
    // Iterate in reverse so topmost (last added) component is checked first
    for (int i = (int)components.size() - 1; i >= 0; i--)
    {
        if (components[i]->isPointInside(worldPoint))
        {
            return components[i];
        }
    }
    return nullptr;
}

int Circuit::getWireIndexAtPosition(const Vector2D& worldPoint, float tolerance)
{
    for (int i = 0; i < (int)wires.size(); i++)
    {
        if (wires[i]->isPointNearWire(worldPoint, tolerance))
        {
            return i;
        }
    }
    return -1;
}

Junction* Circuit::getJunctionAtPosition(const Vector2D& worldPoint, float tolerance)
{
    for (int i = 0; i < (int)junctions.size(); i++)
    {
        if (MathUtils::distance(junctions[i]->getPosition(), worldPoint) <= tolerance)
        {
            return junctions[i];
        }
    }
    return nullptr;
}


// ---- Circuit state ----

void Circuit::clear()
{
    for (int i = 0; i < (int)components.size(); i++)
    {
        delete components[i];
    }
    components.clear();

    for (int i = 0; i < (int)wires.size(); i++)
    {
        delete wires[i];
    }
    wires.clear();

    for (int i = 0; i < (int)junctions.size(); i++)
    {
        delete junctions[i];
    }
    junctions.clear();

    idCounters.clear();
}

bool Circuit::isEmpty() const
{
    return components.empty();
}

string Circuit::generateUniqueId(const string& componentType)
{
    incrementIdCounter(componentType);
    int count = getIdCounter(componentType);

    // Build prefix from first letter(s) of type
    // e.g. "RESISTOR" -> "R", "AND" -> "AND", "GND" -> "GND"
    string prefix = "";
    if (componentType == "RESISTOR")       prefix = "R";
    else if (componentType == "CAPACITOR") prefix = "C";
    else if (componentType == "INDUCTOR")  prefix = "L";
    else if (componentType == "DCVOLTAGE") prefix = "V";
    else if (componentType == "BATTERY")   prefix = "BAT";
    else if (componentType == "GND")       prefix = "GND";
    else if (componentType == "CLOCK")     prefix = "CLK";
    else if (componentType == "LED")       prefix = "LED";
    else if (componentType == "SWITCH")    prefix = "SW";
    else if (componentType == "BUTTON")    prefix = "BTN";
    else if (componentType == "7SEG")      prefix = "SEG";
    else if (componentType == "AND")       prefix = "AND";
    else if (componentType == "OR")        prefix = "OR";
    else if (componentType == "NOT")       prefix = "NOT";
    else if (componentType == "NAND")      prefix = "NAND";
    else if (componentType == "XOR")       prefix = "XOR";
    else if (componentType == "DFF")       prefix = "DFF";
    else                                   prefix = componentType;

    return prefix + to_string(count);
}


// ---- Private helpers ----

int Circuit::getIdCounter(const string& componentType)
{
    for (int i = 0; i < (int)idCounters.size(); i++)
    {
        if (idCounters[i].first == componentType)
        {
            return idCounters[i].second;
        }
    }
    return 0;
}

void Circuit::incrementIdCounter(const string& componentType)
{
    for (int i = 0; i < (int)idCounters.size(); i++)
    {
        if (idCounters[i].first == componentType)
        {
            idCounters[i].second += 1;
            return;
        }
    }
    // Type not seen before — add it
    idCounters.push_back(make_pair(componentType, 1));
}