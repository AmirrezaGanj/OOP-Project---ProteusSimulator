#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

// ============================================================
// Toolbar — the horizontal button bar at the top of the
// main editor screen.
//
// Buttons (left to right):
//   [SELECT]  [WIRE]  |  [RUN]  [PAUSE]  [STOP]  [STEP]  |
//   [UNDO]  [REDO]  |  [SAVE]  [EXPORT]
//
// Each frame, MainEditorScreen calls handleEvent() to let the
// toolbar process mouse input. After that it calls
// getLastAction() to check if a button was clicked, then
// dispatches the action to the appropriate system.
// After reading, it calls clearLastAction().
//
// Toggle buttons (SELECT, WIRE) reflect the active tool mode.
// ============================================================

class Toolbar
{
public:

    // ---- Actions ----

    enum class ToolbarAction
    {
        NONE,
        SELECT_TOOL,
        WIRE_TOOL,
        RUN,
        PAUSE,
        STOP,
        STEP,
        UNDO,
        REDO,
        SAVE,
        EXPORT_IMAGE
    };

    // ---- Active tool mode ----

    enum class ToolMode
    {
        SELECT,
        WIRE
    };

    // barRect: the screen rectangle this toolbar occupies
    // font:    pre-loaded TTF_Font* (owned by Application)
    Toolbar(const SDL_Rect& barRect, TTF_Font* font);

    // ---- Event handling ----

    // Pass SDL events here. Returns true if event was consumed.
    bool handleEvent(const SDL_Event& event);

    // ---- Draw ----

    void draw(SDL_Renderer* renderer) const;

    // ---- Action polling ----

    ToolbarAction getLastAction() const;
    void          clearLastAction();

    // ---- State setters (called by MainEditorScreen) ----

    void setToolMode(ToolMode mode);
    void setCanUndo(bool canUndo);
    void setCanRedo(bool canRedo);
    void setSimRunning(bool running);
    void setSimPaused(bool paused);

    ToolMode getCurrentToolMode() const;

private:

    // ---- Button definition ----

    struct Button
    {
        SDL_Rect      rect;
        string        label;
        ToolbarAction action;
        bool          isToggle;
        bool          isActive;
        bool          isHovered;
        bool          isEnabled;
        bool          isDivider;

        Button()
            : action(ToolbarAction::NONE),
              isToggle(false), isActive(false),
              isHovered(false), isEnabled(true), isDivider(false)
        {}
    };

    SDL_Rect  barRect;
    TTF_Font* font;             // not owned

    vector<Button>  buttons;
    ToolbarAction   lastAction;
    ToolMode        currentToolMode;

    // Colors
    SDL_Color bgColor;
    SDL_Color buttonNormalColor;
    SDL_Color buttonHoverColor;
    SDL_Color buttonActiveColor;
    SDL_Color buttonDisabledColor;
    SDL_Color textColor;
    SDL_Color textDisabledColor;
    SDL_Color dividerColor;

    // ---- Setup ----

    void buildButtons();

    // Helper: creates a regular button and appends it to buttons vector.
    // Advances xCursor by the button width + gap.
    void appendButton(const string& label,
                      ToolbarAction action,
                      bool isToggle,
                      int& xCursor);

    // Helper: creates a vertical divider and appends it to buttons vector.
    // Advances xCursor by the divider width + padding.
    void appendDivider(int& xCursor);

    // ---- Drawing helpers ----

    void drawButton(SDL_Renderer* renderer, const Button& btn) const;

    void renderText(SDL_Renderer* renderer,
                    const string& text,
                    int x, int y,
                    SDL_Color color) const;

    // ---- Hit testing ----

    int findButtonAt(int mouseX, int mouseY) const;
};