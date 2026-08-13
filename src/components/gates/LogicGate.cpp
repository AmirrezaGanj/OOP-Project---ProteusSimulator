#include "components/gates/LogicGate.h"
#include "utils/MathUtils.h"
#include <iostream>
#include <cmath>

using namespace std;

// ============================================================
// LogicGate.cpp — implementation of the abstract LogicGate base.
// See LogicGate.h for full documentation.
// ============================================================


// ---- Static constant definitions ----

const float LogicGate::VOLTAGE_HIGH          = 4.0f;
const float LogicGate::VOLTAGE_LOW           = 1.0f;
const float LogicGate::OUTPUT_HIGH_VOLTAGE   = 5.0f;
const float LogicGate::OUTPUT_LOW_VOLTAGE    = 0.0f;


// ---- Constructor ----

LogicGate::LogicGate(const string& id,
                     const string& label,
                     const Vector2D& position,
                     int numInputs,
                     double propagationDelayMs)
    : Component(id, label, position),
      numInputs(numInputs),
      propagationDelayMs(propagationDelayMs),
      outputState(LogicState::UNDEFINED),
      pendingDelayMs(0.0),
      hasPendingOutput(false),
      pendingOutputState(LogicState::UNDEFINED)
{
    // Set bounding box size based on number of inputs
    // More inputs = taller gate body
    width  = 60.0f;
    height = 20.0f + (numInputs - 2) * 10.0f;
    if (height < 30.0f) height = 30.0f;

    // Create input pins spaced evenly on the left side of the gate
    float pinSpacing = height / (numInputs + 1);
    for (int i = 0; i < numInputs; i++)
    {
        float yOffset = -height / 2.0f + pinSpacing * (i + 1);
        pins.push_back(Pin("input" + to_string(i + 1),
                           Vector2D(-width / 2.0f, yOffset)));
    }

    // Single output pin on the right side
    pins.push_back(Pin("output", Vector2D(width / 2.0f, 0.0f)));

    updateAllPinPositions();
}


// ---- evaluate() ----

void LogicGate::evaluate()
{
    vector<LogicState> inputStates = readInputStates();

    // If any input is UNDEFINED, output is immediately UNDEFINED
    bool hasUndefined = false;
    for (int i = 0; i < (int)inputStates.size(); i++)
    {
        if (inputStates[i] == LogicState::UNDEFINED)
        {
            hasUndefined = true;
            break;
        }
    }

    LogicState result;

    if (hasUndefined)
    {
        result = LogicState::UNDEFINED;
    }
    else
    {
        result = evaluateLogic(inputStates);
    }

    // Apply propagation delay:
    // Store the pending result and only commit it after the delay has elapsed.
    // For simplicity in this project the SimulationEngine passes delta time
    // and we accumulate it here.
    if (result != outputState)
    {
        if (!hasPendingOutput)
        {
            hasPendingOutput   = true;
            pendingOutputState = result;
            pendingDelayMs     = 0.0;
        }
    }

    // Commit pending output if delay has elapsed
    // (SimulationEngine calls evaluate() once per tick — delay is approximated)
    if (hasPendingOutput)
    {
        pendingDelayMs += 1.0;   // each evaluate() call = ~1ms simulation tick

        if (pendingDelayMs >= propagationDelayMs)
        {
            writeOutputState(pendingOutputState);
            hasPendingOutput = false;
            pendingDelayMs   = 0.0;
        }
    }
}


// ---- serialize() ----

string LogicGate::serialize() const
{
    return getType()
        + " " + id
        + " " + label
        + " " + to_string(position.x)
        + " " + to_string(position.y)
        + " " + to_string(numInputs)
        + " " + to_string(propagationDelayMs)
        + " " + to_string(rotation)
        + " " + (mirrored ? "1" : "0");
}


// ---- Getters / Setters ----

int LogicGate::getNumInputs() const
{
    return numInputs;
}

double LogicGate::getPropagationDelayMs() const
{
    return propagationDelayMs;
}

void LogicGate::setPropagationDelayMs(double delayMs)
{
    propagationDelayMs = delayMs;
}

LogicGate::LogicState LogicGate::getOutputState() const
{
    return outputState;
}


// ---- Protected helpers ----

LogicGate::LogicState LogicGate::voltageToLogicState(float voltage) const
{
    if (voltage >= VOLTAGE_HIGH)
    {
        return LogicState::HIGH;
    }
    else if (voltage <= VOLTAGE_LOW)
    {
        return LogicState::LOW;
    }
    else
    {
        return LogicState::UNDEFINED;
    }
}

float LogicGate::logicStateToVoltage(LogicState state) const
{
    if (state == LogicState::HIGH)
    {
        return OUTPUT_HIGH_VOLTAGE;
    }
    else if (state == LogicState::LOW)
    {
        return OUTPUT_LOW_VOLTAGE;
    }
    else
    {
        // UNDEFINED — use a mid-rail voltage as marker
        return 2.5f;
    }
}

vector<LogicGate::LogicState> LogicGate::readInputStates() const
{
    vector<LogicState> states;

    for (int i = 0; i < numInputs; i++)
    {
        string pinName = "input" + to_string(i + 1);

        // We need a non-const way to find the pin — cast is safe here
        // because we are only reading voltage, not modifying
        LogicGate* self = const_cast<LogicGate*>(this);
        Pin* pin = self->findPin(pinName);

        if (pin == nullptr || !pin->isConnected)
        {
            // Floating input — warn and mark UNDEFINED
            cout << "WARNING: Floating input detected on gate "
                 << id << " pin " << pinName << endl;
            states.push_back(LogicState::UNDEFINED);
        }
        else
        {
            states.push_back(voltageToLogicState(pin->voltage));
        }
    }

    return states;
}

void LogicGate::writeOutputState(LogicState state)
{
    outputState = state;

    Pin* outPin = findPin("output");
    if (outPin != nullptr)
    {
        outPin->voltage = logicStateToVoltage(state);
    }
}


// ---- Drawing helpers ----

void LogicGate::drawPinLines(SDL_Renderer* renderer,
                              const Vector2D& panOffset,
                              float zoom) const
{
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);

    LogicGate* self = const_cast<LogicGate*>(this);

    // Draw input pin lines (short horizontal lines from pin to gate body)
    for (int i = 0; i < numInputs; i++)
    {
        string pinName = "input" + to_string(i + 1);
        Pin* pin = self->findPin(pinName);
        if (pin == nullptr) continue;

        Vector2D screenPin  = MathUtils::worldToScreen(pin->worldPosition, panOffset, zoom);
        Vector2D bodyEdge   = MathUtils::worldToScreen(
                                  Vector2D(position.x - width / 2.0f * 0.6f,
                                           pin->worldPosition.y),
                                  panOffset, zoom);

        SDL_RenderDrawLine(renderer,
                           (int)screenPin.x,  (int)screenPin.y,
                           (int)bodyEdge.x,   (int)bodyEdge.y);
    }

    // Draw output pin line
    Pin* outPin = self->findPin("output");
    if (outPin != nullptr)
    {
        Vector2D screenOut  = MathUtils::worldToScreen(outPin->worldPosition, panOffset, zoom);
        Vector2D bodyEdge   = MathUtils::worldToScreen(
                                  Vector2D(position.x + width / 2.0f * 0.6f,
                                           outPin->worldPosition.y),
                                  panOffset, zoom);

        SDL_RenderDrawLine(renderer,
                           (int)bodyEdge.x,  (int)bodyEdge.y,
                           (int)screenOut.x, (int)screenOut.y);
    }
}

void LogicGate::drawPinHighlights(SDL_Renderer* renderer,
                                   const Vector2D& panOffset,
                                   float zoom) const
{
    LogicGate* self = const_cast<LogicGate*>(this);

    for (int i = 0; i < (int)pins.size(); i++)
    {
        Pin* pin = &self->getPins()[i];

        if (pin->isHighlighted)
        {
            Vector2D screenPos = MathUtils::worldToScreen(pin->worldPosition,
                                                           panOffset, zoom);
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);

            SDL_Rect dot;
            dot.x = (int)screenPos.x - 4;
            dot.y = (int)screenPos.y - 4;
            dot.w = 8;
            dot.h = 8;
            SDL_RenderFillRect(renderer, &dot);
        }
    }
}