#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

// ============================================================
// SimulationLogPanel — the text terminal at the bottom of the
// main editor screen showing simulation messages, warnings,
// and errors.
//
// Features:
//   - Displays messages from SimulationEngine::getLog()
//   - Color codes lines automatically:
//       Contains "ERROR"   -> red
//       Contains "WARNING" -> orange
//       Otherwise          -> light gray
//   - Mouse wheel scrolling through history
//   - Auto-scrolls to the newest message when updated
//   - Clear button inside the panel header
//
// MainEditorScreen calls syncMessages() each frame to copy
// the latest log from SimulationEngine into this panel.
// ============================================================

class SimulationLogPanel
{
public:

    // panelRect: screen rectangle for this panel
    // font:      pre-loaded TTF_Font* (owned by Application)
    SimulationLogPanel(const SDL_Rect& panelRect, TTF_Font* font);

    // ---- Message sync ----

    // Copies the current log vector from SimulationEngine into
    // the panel. Call this once per frame.
    void syncMessages(const vector<string>& logMessages);

    // Clears all displayed messages
    void clearMessages();

    // ---- Event handling ----

    // Handles mouse wheel scrolling and clear button click.
    // Returns true if the event was consumed.
    bool handleEvent(const SDL_Event& event);

    // ---- Draw ----

    void draw(SDL_Renderer* renderer) const;

private:

    SDL_Rect  panelRect;
    TTF_Font* font;             // not owned

    vector<string> messages;

    // How many lines are scrolled from the top (0 = show latest)
    int scrollOffset;

    // Height of one text line in pixels
    static const int LINE_HEIGHT = 18;

    // Height of the header bar (title + clear button)
    static const int HEADER_HEIGHT = 26;

    // Clear button rect (computed in constructor)
    SDL_Rect clearButtonRect;
    bool     clearButtonHovered;

    // Colors
    SDL_Color bgColor;
    SDL_Color headerColor;
    SDL_Color borderColor;
    SDL_Color textNormalColor;
    SDL_Color textErrorColor;
    SDL_Color textWarningColor;
    SDL_Color textDimColor;
    SDL_Color clearBtnColor;
    SDL_Color clearBtnHoverColor;

    // ---- Helpers ----

    // Returns how many lines fit in the visible area
    int visibleLineCount() const;

    // Determines the text color for a message based on its content
    SDL_Color colorForMessage(const string& message) const;

    // Renders text at (x, y) with the given color
    void renderText(SDL_Renderer* renderer,
                    const string& text,
                    int x, int y,
                    SDL_Color color) const;

    // Scrolls to show the most recent message
    void scrollToBottom();
};