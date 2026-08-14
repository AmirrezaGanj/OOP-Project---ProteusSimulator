#include "ui/StatusBar.h"
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

// ============================================================
// StatusBar.cpp
// ============================================================


// ---- Constructor ----

StatusBar::StatusBar(const SDL_Rect& barRect, TTF_Font* font)
    : barRect(barRect),
      font(font),
      mouseWorldPos(0.0f, 0.0f),
      zoom(1.0f),
      simState(SimulationClock::SimState::STOPPED),
      hintMessage("Ready")
{
    // Background — dark charcoal
    bgColor   = { 35, 35, 38, 255 };

    // Normal text — light gray
    textColor = { 200, 200, 200, 255 };

    // Running — green
    runColor  = { 80, 200, 80, 255 };

    // Paused — orange
    pauseColor = { 220, 140, 40, 255 };
}


// ---- Update ----

void StatusBar::setMouseWorldPosition(const Vector2D& worldPos)
{
    mouseWorldPos = worldPos;
}

void StatusBar::setZoom(float zoomFactor)
{
    zoom = zoomFactor;
}

void StatusBar::setSimulationState(SimulationClock::SimState state)
{
    simState = state;
}

void StatusBar::setHintMessage(const string& hint)
{
    hintMessage = hint;
}


// ---- Draw ----

void StatusBar::draw(SDL_Renderer* renderer) const
{
    // ---- Background ----
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &barRect);

    // Top border line — slightly lighter than background
    SDL_SetRenderDrawColor(renderer, 70, 70, 75, 255);
    SDL_RenderDrawLine(renderer,
                       barRect.x,
                       barRect.y,
                       barRect.x + barRect.w,
                       barRect.y);

    if (font == nullptr)
    {
        return;     // no font loaded — skip text rendering
    }

    int textY     = barRect.y + (barRect.h - 14) / 2;  // vertically centered
    int padding   = 10;
    int sectionGap = 25;    // gap between sections
    int curX       = barRect.x + padding;

    // ---- Section 1: Mouse coordinates ----
    // Format: "X: 123  Y: -456"
    ostringstream coordStream;
    coordStream << "X: "
                << fixed << setprecision(0) << mouseWorldPos.x
                << "   Y: "
                << fixed << setprecision(0) << mouseWorldPos.y;

    renderText(renderer, coordStream.str(), curX, textY, textColor);
    curX += 160 + sectionGap;

    // ---- Divider ----
    SDL_SetRenderDrawColor(renderer, 70, 70, 75, 255);
    SDL_RenderDrawLine(renderer,
                       curX - sectionGap / 2, barRect.y + 4,
                       curX - sectionGap / 2, barRect.y + barRect.h - 4);

    // ---- Section 2: Zoom level ----
    // Format: "Zoom: 150%"
    ostringstream zoomStream;
    zoomStream << "Zoom: "
               << (int)round(zoom * 100.0f) << "%";

    renderText(renderer, zoomStream.str(), curX, textY, textColor);
    curX += 100 + sectionGap;

    // ---- Divider ----
    SDL_SetRenderDrawColor(renderer, 70, 70, 75, 255);
    SDL_RenderDrawLine(renderer,
                       curX - sectionGap / 2, barRect.y + 4,
                       curX - sectionGap / 2, barRect.y + barRect.h - 4);

    // ---- Section 3: Simulation state ----
    // Format: "● RUNNING" / "|| PAUSED" / "■ STOPPED"
    string stateLabel = simStateToString();
    SDL_Color stateColor = simStateColor();

    renderText(renderer, stateLabel, curX, textY, stateColor);
    curX += 120 + sectionGap;

    // ---- Divider ----
    SDL_SetRenderDrawColor(renderer, 70, 70, 75, 255);
    SDL_RenderDrawLine(renderer,
                       curX - sectionGap / 2, barRect.y + 4,
                       curX - sectionGap / 2, barRect.y + barRect.h - 4);

    // ---- Section 4: Hint message (right-aligned) ----
    // Draw hint text near the right edge of the bar
    if (!hintMessage.empty())
    {
        int hintX = barRect.x + barRect.w - 300;
        renderText(renderer, hintMessage, hintX, textY, textColor);
    }
}


// ---- Private helpers ----

void StatusBar::renderText(SDL_Renderer* renderer,
                            const string& text,
                            int x, int y,
                            SDL_Color color) const
{
    if (font == nullptr || text.empty())
    {
        return;
    }

    // Render the text to a surface
    SDL_Surface* surface = TTF_RenderText_Blended(font,
                                                   text.c_str(),
                                                   color);
    if (surface == nullptr)
    {
        return;
    }

    // Convert surface to texture for GPU rendering
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture == nullptr)
    {
        return;
    }

    // Query texture size and render at the given position
    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);

    SDL_Rect destRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);

    SDL_DestroyTexture(texture);
}


string StatusBar::simStateToString() const
{
    if (simState == SimulationClock::SimState::RUNNING)
    {
        return "RUNNING";
    }
    else if (simState == SimulationClock::SimState::PAUSED)
    {
        return "PAUSED";
    }
    else
    {
        return "STOPPED";
    }
}


SDL_Color StatusBar::simStateColor() const
{
    if (simState == SimulationClock::SimState::RUNNING)
    {
        return runColor;
    }
    else if (simState == SimulationClock::SimState::PAUSED)
    {
        return pauseColor;
    }
    else
    {
        // STOPPED — dim gray
        return { 130, 130, 130, 255 };
    }
}