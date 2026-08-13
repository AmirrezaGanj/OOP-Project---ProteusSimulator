#include "components/gates/DFlipFlop.h"
#include "utils/MathUtils.h"
#include <iostream>
#include <cmath>

using namespace std;

// ============================================================
// DFlipFlop.cpp
// ============================================================


DFlipFlop::DFlipFlop(const string& id,
                     const string& label,
                     const Vector2D& position,
                     double propagationDelayMs)
    : LogicGate(id, label, position, 2, propagationDelayMs),
      previousClkState(LogicState::LOW),
      storedQ(LogicState::LOW)
{
    // Rename the pins to match flip-flop convention
    pins[0].name = "D";
    pins[1].name = "CLK";

    // DFF is drawn as a rectangle
    width  = 60.0f;
    height = 50.0f;
    updateAllPinPositions();
}


string DFlipFlop::getType() const
{
    return "DFF";
}


// evaluateLogic() is not used for DFF — edge detection
// is handled directly in evaluate() below.
// We still implement it to satisfy the pure virtual requirement.
LogicGate::LogicState DFlipFlop::evaluateLogic(
    const vector<LogicState>& inputStates) const
{
    return storedQ;
}


void DFlipFlop::evaluate()
{
    // Read D pin
    Pin* dPin = findPin("D");
    if (dPin == nullptr || !dPin->isConnected)
    {
        cout << "WARNING: Floating input detected on DFF " << id << " pin D" << endl;
        writeOutputState(LogicState::UNDEFINED);
        return;
    }

    // Read CLK pin
    Pin* clkPin = findPin("CLK");
    if (clkPin == nullptr || !clkPin->isConnected)
    {
        cout << "WARNING: Floating input detected on DFF " << id << " pin CLK" << endl;
        writeOutputState(LogicState::UNDEFINED);
        return;
    }

    LogicState currentClk = voltageToLogicState(clkPin->voltage);
    LogicState currentD   = voltageToLogicState(dPin->voltage);

    // Detect RISING EDGE: CLK was LOW last tick, is HIGH this tick
    bool risingEdge = (previousClkState == LogicState::LOW &&
                       currentClk        == LogicState::HIGH);

    if (risingEdge)
    {
        // Capture D into Q on the rising edge
        if (currentD == LogicState::UNDEFINED)
        {
            cout << "WARNING: Floating input detected on DFF "
                 << id << " pin D at clock edge" << endl;
            storedQ = LogicState::UNDEFINED;
        }
        else
        {
            storedQ = currentD;
        }

        writeOutputState(storedQ);
    }

    // Update CLK state for next tick
    previousClkState = currentClk;
}


void DFlipFlop::draw(SDL_Renderer* renderer,
                     const Vector2D& panOffset,
                     float zoom) const
{
    Vector2D center = MathUtils::worldToScreen(position, panOffset, zoom);

    float w     = width  * zoom;
    float h     = height * zoom;
    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    if (isSelected())
    {
        drawSelectionHighlight(renderer, panOffset, zoom);
    }

    // Draw rectangular body
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    SDL_Rect body;
    body.x = (int)(center.x - halfW);
    body.y = (int)(center.y - halfH);
    body.w = (int)w;
    body.h = (int)h;
    SDL_RenderDrawRect(renderer, &body);

    // Draw small clock arrow symbol (>) on the CLK pin side
    // to indicate edge-triggered behavior
    Pin* clkPin = const_cast<DFlipFlop*>(this)->findPin("CLK");
    if (clkPin != nullptr)
    {
        Vector2D clkScreen = MathUtils::worldToScreen(clkPin->worldPosition,
                                                       panOffset, zoom);
        float arrowSize = 5.0f * zoom;

        SDL_RenderDrawLine(renderer,
                           (int)(clkScreen.x + halfW * 0.1f),
                           (int)(clkScreen.y - arrowSize),
                           (int)(clkScreen.x + halfW * 0.1f + arrowSize),
                           (int)(clkScreen.y));

        SDL_RenderDrawLine(renderer,
                           (int)(clkScreen.x + halfW * 0.1f + arrowSize),
                           (int)(clkScreen.y),
                           (int)(clkScreen.x + halfW * 0.1f),
                           (int)(clkScreen.y + arrowSize));
    }

    drawPinLines(renderer, panOffset, zoom);
    drawPinHighlights(renderer, panOffset, zoom);
}