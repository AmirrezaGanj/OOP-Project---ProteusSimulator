#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include "core/Component.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// LogicGate — abstract base class for all digital logic gates.
//
// Inherits from Component and adds:
//   - Static voltage references for HIGH and LOW levels
//   - A LogicState enum (HIGH, LOW, UNDEFINED)
//   - A configurable number of input pins
//   - One output pin
//   - Propagation delay support
//   - A shared method to resolve a voltage to a LogicState
//   - Pure virtual evaluateLogic() that each gate implements
//
// All derived gates (AND, OR, NOT, NAND, XOR, DFlipFlop)
// inherit from this class exactly like your Q2 Room/Suite/Hall
// inherited from Unit.
// ============================================================

class LogicGate : public Component
{
public:

    // ---- Logic voltage thresholds (static / global) ----

    // Any voltage at or above this is Logic HIGH
    static const float VOLTAGE_HIGH;

    // Any voltage at or below this is Logic LOW
    static const float VOLTAGE_LOW;

    // The ideal output voltage driven when gate outputs HIGH
    static const float OUTPUT_HIGH_VOLTAGE;

    // The ideal output voltage driven when gate outputs LOW
    static const float OUTPUT_LOW_VOLTAGE;

    // ---- Logic state enum ----

    enum class LogicState
    {
        HIGH,
        LOW,
        UNDEFINED   // floating input, short circuit, or unresolvable
    };

    // ---- Constructor / Destructor ----

    LogicGate(const string& id,
              const string& label,
              const Vector2D& position,
              int numInputs,
              double propagationDelayMs = 1.0);

    virtual ~LogicGate() {}

    // ---- Pure virtual methods (each gate must implement these) ----

    // Performs the gate's boolean logic on its input states
    // and returns the resulting output LogicState.
    virtual LogicState evaluateLogic(const vector<LogicState>& inputStates) const = 0;

    // Returns gate type string e.g. "AND", "OR", "NOT"
    virtual string getType() const override = 0;

    // ---- Overrides from Component ----

    // Reads input pin voltages, resolves them to LogicStates,
    // calls evaluateLogic(), then writes result to output pin.
    // Respects propagation delay via the simulation clock.
    void evaluate() override;

    // Serializes gate type, position, label, numInputs, delay
    string serialize() const override;

    // ---- Getters / Setters ----

    int    getNumInputs()           const;
    double getPropagationDelayMs()  const;
    void   setPropagationDelayMs(double delayMs);

    // Returns the current output LogicState (cached from last evaluate())
    LogicState getOutputState() const;

protected:

    int    numInputs;
    double propagationDelayMs;

    // Cached output state from the last evaluate() call
    LogicState outputState;

    // Time accumulator used for propagation delay (in milliseconds)
    double pendingDelayMs;
    bool   hasPendingOutput;
    LogicState pendingOutputState;

    // ---- Shared helpers for derived classes ----

    // Converts a pin voltage to a LogicState using the static thresholds
    LogicState voltageToLogicState(float voltage) const;

    // Converts a LogicState back to the ideal output voltage
    float logicStateToVoltage(LogicState state) const;

    // Reads all input pins and returns their LogicStates.
    // Marks any unconnected input pin as UNDEFINED and logs a warning.
    vector<LogicState> readInputStates() const;

    // Writes the given LogicState voltage to the output pin
    void writeOutputState(LogicState state);

    // ---- Drawing helpers shared by all gates ----

    // Draws the input lines, output line, and label.
    // Each derived gate calls this after drawing its own body shape.
    void drawPinLines(SDL_Renderer* renderer,
                      const Vector2D& panOffset,
                      float zoom) const;

    // Draws highlighted pin dots when mouse is near
    void drawPinHighlights(SDL_Renderer* renderer,
                           const Vector2D& panOffset,
                           float zoom) const;
};