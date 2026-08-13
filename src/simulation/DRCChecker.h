#pragma once

#include <string>
#include <vector>
#include "core/Circuit.h"

using namespace std;

// ============================================================
// DRCChecker — Design Rule Check.
//
// Runs a set of validation checks on the Circuit before the
// simulation is allowed to start. If any ERROR-level issues
// are found, the SimulationEngine should refuse to run and
// display the errors in the SimulationLogPanel.
//
// Checks performed:
//   1. Empty circuit        — nothing to simulate
//   2. No GND present       — circuit has no voltage reference
//   3. Floating input pins  — unconnected inputs on gates/components
//   4. Short circuits       — a single pin driven by two conflicting
//                             voltage sources simultaneously
//   5. Output-only circuit  — sources exist but no output components
//
// Each check produces a DRCResult with a severity and message.
// Severity ERROR blocks simulation. Severity WARNING allows it
// but informs the user something may be wrong.
// ============================================================

class DRCChecker
{
public:

    // ---- Result struct ----

    enum class Severity
    {
        ERROR,
        WARNING
    };

    struct DRCResult
    {
        Severity severity;
        string   message;
        string   componentId;   // ID of the offending component, if applicable

        DRCResult(Severity sev,
                  const string& msg,
                  const string& compId = "")
            : severity(sev), message(msg), componentId(compId)
        {}
    };

    DRCChecker();

    // ---- Main entry point ----

    // Runs all checks on the given circuit.
    // Returns a list of DRCResults (may be empty if all is well).
    // Caller should check if any result has severity == ERROR
    // before allowing simulation to start.
    vector<DRCResult> runChecks(Circuit& circuit);

    // Returns true if the last runChecks() call found any ERRORs
    bool hasErrors() const;

    // Returns true if the last runChecks() call found any WARNINGs
    bool hasWarnings() const;

private:

    bool lastRunHadErrors;
    bool lastRunHadWarnings;

    // ---- Individual checks ----

    // Check 1: is the circuit completely empty?
    void checkEmptyCircuit(Circuit& circuit,
                           vector<DRCResult>& results);

    // Check 2: does at least one GND component exist?
    void checkGNDExists(Circuit& circuit,
                        vector<DRCResult>& results);

    // Check 3: are any input pins on gates/LEDs/etc. unconnected?
    // An unconnected input pin produces UNDEFINED output and a warning.
    void checkFloatingPins(Circuit& circuit,
                           vector<DRCResult>& results);

    // Check 4: is any single pin connected to two or more voltage
    // sources that would drive conflicting voltages?
    void checkShortCircuits(Circuit& circuit,
                            vector<DRCResult>& results);

    // Check 5: does the circuit have at least one output component
    // (LED, SevenSegment, Voltmeter, etc.) to show results?
    void checkHasOutputComponent(Circuit& circuit,
                                 vector<DRCResult>& results);

    // ---- Helpers ----

    // Returns true if the component type has input pins that
    // must be connected for the simulation to be valid
    bool typeRequiresConnectedInputs(const string& type) const;

    // Returns true if the component type is a voltage source
    bool isVoltageSource(const string& type) const;

    // Returns true if the component type is an output/display component
    bool isOutputComponent(const string& type) const;

    // Returns the names of the input pins for a given component type
    vector<string> getInputPinNames(Component* component) const;
};