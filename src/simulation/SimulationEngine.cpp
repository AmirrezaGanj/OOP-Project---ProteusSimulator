#include "simulation/SimulationEngine.h"
#include "components/sources/ClockGenerator.h"
#include <algorithm>
#include <cmath>

using namespace std;

// ============================================================
// SimulationEngine.cpp
// ============================================================

//const int SimulationEngine::EVALUATION_PASSES = 5;


// ---- Constructor ----

SimulationEngine::SimulationEngine(Circuit& circuit, SimulationClock& clock)
    : circuit(circuit),
      clock(clock)
{
}


// ---- Main update ----

void SimulationEngine::update()
{
    if (clock.isStopped()) return;

    bool shouldEvaluate = clock.tick();
    if (!shouldEvaluate) return;

    // ---- Run the full evaluation cycle iteratively ----
    for (int pass = 0; pass < EVALUATION_PASSES; pass++)
    {
        vector<Component*>& components = circuit.getComponents();
        for (int i = 0; i < (int)components.size(); i++)
        {
            components[i]->resetPins();
        }

        evaluateSources();
        propagateWires();

        evaluateInteractive();
        propagateWires();

        evaluateGates();
        propagateWires();

        evaluateInteractive();
        propagateWires();
    }

    updateWireVoltages();
}


// ---- Direct state control ----

void SimulationEngine::run()
{
    if (clock.isStopped())
    {
        // Fresh start — clear the log and reset components
        clearLog();
        log("Simulation started.");
        resetAllComponents();
    }
    else if (clock.isPaused())
    {
        log("Simulation resumed.");
    }

    clock.run();
}

void SimulationEngine::pause()
{
    clock.pause();
    log("Simulation paused at tick " + to_string(clock.getTickCount()) + ".");
}

void SimulationEngine::stop()
{
    clock.stop();
    resetAllComponents();
    log("Simulation stopped.");
}

void SimulationEngine::stepOnce()
{
    if (clock.isStopped())
    {
        clearLog();
        log("Step mode — simulation started.");
        resetAllComponents();
        clock.run();
        clock.pause();
    }

    for (int pass = 0; pass < EVALUATION_PASSES; pass++)
    {
        vector<Component*>& components = circuit.getComponents();
        for (int i = 0; i < (int)components.size(); i++)
        {
            components[i]->resetPins();
        }

        evaluateSources();
        propagateWires();

        evaluateInteractive();
        propagateWires();

        evaluateGates();
        propagateWires();

        evaluateInteractive();
        propagateWires();
    }

    updateWireVoltages();
    clock.stepOnce();

    log("Step: tick " + to_string(clock.getTickCount()));
}


// ---- Log access ----

const vector<string>& SimulationEngine::getLog() const
{
    return logMessages;
}

void SimulationEngine::clearLog()
{
    logMessages.clear();
}

void SimulationEngine::log(const string& message)
{
    logMessages.push_back(message);

    // Keep the log from growing unbounded — cap at 200 messages
    if ((int)logMessages.size() > 200)
    {
        logMessages.erase(logMessages.begin());
    }
}


// ---- Core simulation steps ----

void SimulationEngine::evaluateSources()
{
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        if (isSourceType(components[i]->getType()))
        {
            components[i]->evaluate();
        }
    }
}

void SimulationEngine::propagateWires()
{
    vector<Wire*>& wires = circuit.getWires();
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)wires.size(); i++)
    {
        Wire* wire = wires[i];

        // Find the two components this wire connects
        Component* fromComp = circuit.findComponent(wire->getFromComponentId());
        Component* toComp   = circuit.findComponent(wire->getToComponentId());

        if (fromComp == nullptr || toComp == nullptr)
        {
            continue;
        }

        Pin* fromPin = fromComp->findPin(wire->getFromPinName());
        Pin* toPin   = toComp->findPin(wire->getToPinName());

        if (fromPin == nullptr || toPin == nullptr)
        {
            continue;
        }

        // Propagate the higher voltage along the wire.
        // This handles: source driving a gate input, gate driving LED, etc.
        // A more complete implementation would use nodal analysis,
        // but for this project voltage-max propagation handles
        // all required cases correctly.
        float dominantVoltage = max(fromPin->voltage, toPin->voltage);

        fromPin->voltage = dominantVoltage;
        toPin->voltage   = dominantVoltage;

        // Also propagate voltage to any junctions at this wire's position
        vector<Junction*>& junctions = circuit.getJunctions();
        for (int j = 0; j < (int)junctions.size(); j++)
        {
            Vector2D jPos = junctions[j]->getPosition();

            // Check if this junction sits on either endpoint of the wire
            const vector<Vector2D>& waypoints = wire->getWaypoints();
            if (waypoints.size() >= 2)
            {
                float distStart = sqrt(pow(jPos.x - waypoints.front().x, 2) +
                                       pow(jPos.y - waypoints.front().y, 2));
                float distEnd   = sqrt(pow(jPos.x - waypoints.back().x, 2) +
                                       pow(jPos.y - waypoints.back().y, 2));

                if (distStart <= 8.0f || distEnd <= 8.0f)
                {
                    junctions[j]->setVoltage(dominantVoltage);
                }
            }
        }
    }
}

void SimulationEngine::evaluateGates()
{
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        if (isGateType(components[i]->getType()))
        {
            components[i]->evaluate();
        }
    }
}

void SimulationEngine::evaluateInteractive()
{
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        if (isInteractiveType(components[i]->getType()))
        {
            components[i]->evaluate();
        }
    }
}

void SimulationEngine::updateWireVoltages()
{
    vector<Wire*>& wires = circuit.getWires();

    for (int i = 0; i < (int)wires.size(); i++)
    {
        Wire* wire = wires[i];

        Component* fromComp = circuit.findComponent(wire->getFromComponentId());
        Component* toComp   = circuit.findComponent(wire->getToComponentId());

        if (fromComp == nullptr || toComp == nullptr)
        {
            continue;
        }

        Pin* fromPin = fromComp->findPin(wire->getFromPinName());
        Pin* toPin   = toComp->findPin(wire->getToPinName());

        if (fromPin == nullptr || toPin == nullptr)
        {
            continue;
        }

        // Use the dominant pin voltage as the wire's display voltage
        float displayVoltage = max(fromPin->voltage, toPin->voltage);
        wire->setVoltage(displayVoltage);
    }
}

void SimulationEngine::resetAllComponents()
{
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        // Reset all pins to 0V and disconnected state
        components[i]->resetPins();

        // Reset ClockGenerator tick counters specifically
        if (components[i]->getType() == "CLOCK")
        {
            ClockGenerator* clk = dynamic_cast<ClockGenerator*>(components[i]);
            if (clk != nullptr)
            {
                clk->resetClock();
            }
        }
    }

    // Reset all wire display voltages to 0
    vector<Wire*>& wires = circuit.getWires();
    for (int i = 0; i < (int)wires.size(); i++)
    {
        wires[i]->setVoltage(0.0f);
    }

    // Reset all junction display voltages to 0
    vector<Junction*>& junctions = circuit.getJunctions();
    for (int i = 0; i < (int)junctions.size(); i++)
    {
        junctions[i]->reset();
    }
}


// ---- Helpers ----

bool SimulationEngine::isSourceType(const string& type) const
{
    return (type == "GND"       ||
            type == "DCVOLTAGE" ||
            type == "BATTERY"   ||
            type == "CLOCK");
}

bool SimulationEngine::isGateType(const string& type) const
{
    return (type == "AND"  ||
            type == "OR"   ||
            type == "NOT"  ||
            type == "NAND" ||
            type == "XOR"  ||
            type == "DFF");
}

bool SimulationEngine::isInteractiveType(const string& type) const
{
    return (type == "SWITCH"    ||
            type == "BUTTON"    ||
            type == "LED"       ||
            type == "7SEG"      ||
            type == "RESISTOR"  ||
            type == "CAPACITOR" ||
            type == "INDUCTOR");
}