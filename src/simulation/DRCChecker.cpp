#include "simulation/DRCChecker.h"
#include <algorithm>

using namespace std;

// ============================================================
// DRCChecker.cpp
// ============================================================


// ---- Constructor ----

DRCChecker::DRCChecker()
    : lastRunHadErrors(false),
      lastRunHadWarnings(false)
{
}


// ---- Main entry point ----

vector<DRCChecker::DRCResult> DRCChecker::runChecks(Circuit& circuit)
{
    vector<DRCResult> results;

    lastRunHadErrors   = false;
    lastRunHadWarnings = false;

    // Run all checks — order matters:
    // Empty and GND checks come first since other checks
    // are meaningless if the circuit is empty or ungrounded.
    checkEmptyCircuit(circuit, results);

    if (!lastRunHadErrors)
    {
        checkGNDExists(circuit, results);
        checkFloatingPins(circuit, results);
        checkShortCircuits(circuit, results);
        checkHasOutputComponent(circuit, results);
    }

    // Update summary flags
    for (int i = 0; i < (int)results.size(); i++)
    {
        if (results[i].severity == Severity::ERROR)
        {
            lastRunHadErrors = true;
        }
        else if (results[i].severity == Severity::WARNING)
        {
            lastRunHadWarnings = true;
        }
    }

    return results;
}

bool DRCChecker::hasErrors() const
{
    return lastRunHadErrors;
}

bool DRCChecker::hasWarnings() const
{
    return lastRunHadWarnings;
}


// ---- Individual checks ----

void DRCChecker::checkEmptyCircuit(Circuit& circuit,
                                    vector<DRCResult>& results)
{
    if (circuit.isEmpty())
    {
        results.push_back(DRCResult(
            Severity::ERROR,
            "Circuit is empty. Place at least one component before running.",
            ""
        ));
        lastRunHadErrors = true;
    }
}


void DRCChecker::checkGNDExists(Circuit& circuit,
                                 vector<DRCResult>& results)
{
    vector<Component*>& components = circuit.getComponents();
    bool gndFound = false;

    for (int i = 0; i < (int)components.size(); i++)
    {
        if (components[i]->getType() == "GND")
        {
            gndFound = true;
            break;
        }
    }

    if (!gndFound)
    {
        results.push_back(DRCResult(
            Severity::ERROR,
            "No GND component found. The circuit needs a ground "
            "reference node to simulate correctly.",
            ""
        ));
        lastRunHadErrors = true;
    }
}


void DRCChecker::checkFloatingPins(Circuit& circuit,
                                    vector<DRCResult>& results)
{
    vector<Component*>& components = circuit.getComponents();

    for (int i = 0; i < (int)components.size(); i++)
    {
        Component* comp = components[i];

        if (!typeRequiresConnectedInputs(comp->getType()))
        {
            continue;
        }

        vector<string> inputNames = getInputPinNames(comp);

        for (int j = 0; j < (int)inputNames.size(); j++)
        {
            Pin* pin = comp->findPin(inputNames[j]);

            if (pin != nullptr && !pin->isConnected)
            {
                results.push_back(DRCResult(
                    Severity::ERROR,
                    "Floating input detected on component '"
                        + comp->getId()
                        + "' (type: " + comp->getType() + ")"
                        + " pin '" + inputNames[j] + "'. "
                        "Connect all input pins before running.",
                    comp->getId()
                ));
                lastRunHadErrors = true;
            }
        }
    }
}


void DRCChecker::checkShortCircuits(Circuit& circuit,
                                     vector<DRCResult>& results)
{
    vector<Component*>& components = circuit.getComponents();
    vector<Wire*>&       wires     = circuit.getWires();

    // For each pin in the circuit, count how many voltage sources
    // are driving it directly. If more than one source drives the
    // same pin with potentially different voltages, it is a short.
    //
    // Strategy: collect all (componentId, pinName) pairs that are
    // directly output-driven by a source. Then check if any single
    // pin appears connected to two or more such sources via wires.

    // Build a list of source-driven pins: (compId, pinName, voltage)
    struct DrivenPin
    {
        string compId;
        string pinName;
        string sourceId;
    };

    vector<DrivenPin> sourcePins;

    for (int i = 0; i < (int)components.size(); i++)
    {
        Component* comp = components[i];

        if (!isVoltageSource(comp->getType()))
        {
            continue;
        }

        // All pins of a voltage source are considered driven
        vector<Pin>& pins = comp->getPins();
        for (int j = 0; j < (int)pins.size(); j++)
        {
            DrivenPin dp;
            dp.compId   = comp->getId();
            dp.pinName  = pins[j].name;
            dp.sourceId = comp->getId();
            sourcePins.push_back(dp);
        }
    }

    // For each wire, check if both its endpoint pins are driven
    // by DIFFERENT source components — that is a short circuit.
    for (int i = 0; i < (int)wires.size(); i++)
    {
        Wire* wire = wires[i];

        string fromCompId  = wire->getFromComponentId();
        string fromPinName = wire->getFromPinName();
        string toCompId    = wire->getToComponentId();
        string toPinName   = wire->getToPinName();

        string fromSource = "";
        string toSource   = "";

        for (int j = 0; j < (int)sourcePins.size(); j++)
        {
            if (sourcePins[j].compId  == fromCompId &&
                sourcePins[j].pinName == fromPinName)
            {
                fromSource = sourcePins[j].sourceId;
            }
            if (sourcePins[j].compId  == toCompId &&
                sourcePins[j].pinName == toPinName)
            {
                toSource = sourcePins[j].sourceId;
            }
        }

        // If both ends are driven by different sources — short circuit
        if (!fromSource.empty() && !toSource.empty() &&
            fromSource != toSource)
        {
            results.push_back(DRCResult(
                Severity::ERROR,
                "Short circuit detected: voltage source '"
                    + fromSource
                    + "' and source '"
                    + toSource
                    + "' are directly connected. "
                    "Add a load component (e.g. Resistor) between them.",
                fromSource
            ));
            lastRunHadErrors = true;
        }
    }
}


void DRCChecker::checkHasOutputComponent(Circuit& circuit,
                                          vector<DRCResult>& results)
{
    vector<Component*>& components = circuit.getComponents();
    bool outputFound = false;

    for (int i = 0; i < (int)components.size(); i++)
    {
        if (isOutputComponent(components[i]->getType()))
        {
            outputFound = true;
            break;
        }
    }

    if (!outputFound)
    {
        results.push_back(DRCResult(
            Severity::WARNING,
            "No output component found (LED, 7-segment, etc.). "
            "Simulation will run but results may not be visible.",
            ""
        ));
        lastRunHadWarnings = true;
    }
}


// ---- Helpers ----

bool DRCChecker::typeRequiresConnectedInputs(const string& type) const
{
    // These component types have input pins that must be wired
    return (type == "AND"   ||
            type == "OR"    ||
            type == "NOT"   ||
            type == "NAND"  ||
            type == "XOR"   ||
            type == "DFF"   ||
            type == "LED");
}

bool DRCChecker::isVoltageSource(const string& type) const
{
    return (type == "DCVOLTAGE" ||
            type == "BATTERY"   ||
            type == "CLOCK");
    // GND is a reference node, not a conflicting source — excluded
}

bool DRCChecker::isOutputComponent(const string& type) const
{
    return (type == "LED"    ||
            type == "7SEG"   ||
            type == "SWITCH" ||
            type == "BUTTON");
}

vector<string> DRCChecker::getInputPinNames(Component* component) const
{
    vector<string> inputNames;
    string type = component->getType();

    if (type == "NOT" || type == "DFF")
    {
        // NOT has one input; DFF has D and CLK
        vector<Pin>& pins = component->getPins();
        for (int i = 0; i < (int)pins.size(); i++)
        {
            // All pins except the output pin are inputs
            if (pins[i].name != "output" && pins[i].name != "Q")
            {
                inputNames.push_back(pins[i].name);
            }
        }
    }
    else if (type == "AND"  || type == "OR" ||
             type == "NAND" || type == "XOR")
    {
        // Multi-input gates: all input pins follow the naming "input1", "input2"...
        vector<Pin>& pins = component->getPins();
        for (int i = 0; i < (int)pins.size(); i++)
        {
            if (pins[i].name != "output")
            {
                inputNames.push_back(pins[i].name);
            }
        }
    }
    else if (type == "LED")
    {
        inputNames.push_back("anode");
        // cathode floating is allowed if connected to GND implicitly
    }

    return inputNames;
}