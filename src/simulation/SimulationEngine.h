#pragma once

#include <string>
#include <vector>
#include "core/Circuit.h"
#include "simulation/SimulationClock.h"

using namespace std;

// ============================================================
// SimulationEngine — the brain of the simulation.
//
// Responsibilities:
//   1. Drive the evaluation loop each tick
//   2. Propagate voltages through wires between components
//   3. Evaluate components in the correct order:
//        Sources first -> propagate -> Gates -> propagate ->
//        Interactive/Output components -> propagate
//   4. Repeat for a few iterations so cascaded gate chains
//      settle to a stable state within one tick
//   5. Update wire voltages for color animation
//   6. Reset all components cleanly on Stop
//   7. Collect and expose log messages for SimulationLogPanel
//
// SimulationEngine does NOT own Circuit or SimulationClock —
// it holds references to them (they are owned by Application).
// ============================================================

class SimulationEngine
{
public:

    SimulationEngine(Circuit& circuit, SimulationClock& clock);

    // ---- Main update — called once per frame by Application ----

    // Asks the clock for a tick. If the clock is RUNNING,
    // runs one full evaluation cycle. If STOPPED, resets everything.
    void update();

    // ---- Direct state control (forwarded to clock) ----

    void run();
    void pause();
    void stop();
    void stepOnce();

    // ---- Log access (read by SimulationLogPanel) ----

    const vector<string>& getLog() const;
    void clearLog();

    // Adds a message to the simulation log
    void log(const string& message);

private:

    Circuit&          circuit;
    SimulationClock&  clock;

    // Simulation log messages shown in the log panel
    vector<string> logMessages;

    // How many evaluation passes to run per tick to let
    // cascaded gate chains settle. 5 is enough for most circuits.
    static const int EVALUATION_PASSES = 5;

    // ---- Core simulation steps ----

    // Step 1: evaluate all voltage sources and GND nodes.
    // These anchor the circuit with known driven voltages.
    void evaluateSources();

    // Step 2: copy voltages from driven pins to connected pins
    // through every wire in the circuit.
    void propagateWires();

    // Step 3: evaluate all logic gates.
    // Reads input pin voltages, computes output, writes output pin.
    void evaluateGates();

    // Step 4: evaluate interactive and output components
    // (Switch, PushButton, LED, SevenSegment).
    void evaluateInteractive();

    // Step 5: after all evaluation passes, write the final
    // wire voltage (average of its two endpoint pin voltages)
    // so the Canvas can color wires correctly.
    void updateWireVoltages();

    // Step 6: called when clock transitions to STOPPED.
    // Resets all component pin voltages, clears wire voltages,
    // resets ClockGenerator tick counters.
    void resetAllComponents();

    // ---- Helper ----

    // Returns true if the given component type string is a source
    bool isSourceType(const string& type) const;

    // Returns true if the given component type string is a gate
    bool isGateType(const string& type) const;

    // Returns true if the given component type string is interactive
    bool isInteractiveType(const string& type) const;
};