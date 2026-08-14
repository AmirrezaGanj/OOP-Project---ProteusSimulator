#pragma once

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "simulation/SimulationClock.h"
#include "utils/Vector2D.h"

using namespace std;

// ============================================================
// StatusBar — the thin information bar at the bottom of the
// main editor screen.
//
// Displays in real time:
//   - Mouse world-space coordinates (X, Y)
//   - Current zoom level as a percentage
//   - Current simulation state (RUNNING / PAUSED / STOPPED)
//   - A short hint message (updated by Canvas or other systems)
//
// The Application loads one TTF_Font and passes it to all
// UI components that need text, so fonts are not loaded
// multiple times.
// ============================================================

class StatusBar
{
public:

    // barRect: the screen rectangle this bar occupies
    //          (typically { 0, windowHeight - barHeight, windowWidth, barHeight })
    // font:    a pre-loaded TTF_Font* (owned by Application, not StatusBar)
    StatusBar(const SDL_Rect& barRect, TTF_Font* font);

    // ---- Update (called every frame before draw) ----

    void setMouseWorldPosition(const Vector2D& worldPos);
    void setZoom(float zoomFactor);
    void setSimulationState(SimulationClock::SimState state);
    void setHintMessage(const string& hint);

    // ---- Draw ----

    void draw(SDL_Renderer* renderer) const;

private:

    SDL_Rect barRect;
    TTF_Font* font;         // not owned — do not free in destructor

    // Displayed values (updated each frame)
    Vector2D              mouseWorldPos;
    float                 zoom;
    SimulationClock::SimState simState;
    string                hintMessage;

    // Colors
    SDL_Color bgColor;      // bar background
    SDL_Color textColor;    // normal text
    SDL_Color runColor;     // color for RUNNING label
    SDL_Color pauseColor;   // color for PAUSED label

    // ---- Helpers ----

    // Renders a text string at (x, y) using the given color
    void renderText(SDL_Renderer* renderer,
                    const string& text,
                    int x, int y,
                    SDL_Color color) const;

    // Converts SimState to a display string
    string simStateToString() const;

    // Returns the color for the current sim state label
    SDL_Color simStateColor() const;
};