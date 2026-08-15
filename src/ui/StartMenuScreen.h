#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "io/ProjectManager.h"

using namespace std;

// ============================================================
// StartMenuScreen — the first screen shown when the program
// launches.
//
// It has three internal states:
//
//   MAIN_MENU
//     Title, three large buttons (New Project / Open Project /
//     Exit) and a list of recent projects on the right.
//
//   NEW_PROJECT_CONFIG
//     Canvas size selection. Four presets (A4 portrait,
//     A4 landscape, A3 landscape, A2 landscape) plus custom
//     width/height text fields. Create / Back buttons.
//
//   OPEN_PROJECT_LIST
//     A scrollable list of every recent project so the user can
//     pick one. Back button.
//
// The screen never acts on its own. It raises an action flag
// that Application polls each frame:
//
//   NEW_PROJECT   -> read getCanvasWidth() / getCanvasHeight()
//   OPEN_PROJECT  -> read getSelectedProjectPath()
//   EXIT          -> quit the program
//
// Application calls clearAction() after handling it.
// ============================================================

class StartMenuScreen
{
public:

    enum class MenuAction
    {
        NONE,
        NEW_PROJECT,
        OPEN_PROJECT,
        EXIT
    };

    enum class ScreenState
    {
        MAIN_MENU,
        NEW_PROJECT_CONFIG,
        OPEN_PROJECT_LIST
    };

    // windowW / windowH: full window size — the screen fills it
    // projectManager: used to read the recent projects list
    // titleFont: larger font for headings (may be the same as font)
    // font: normal UI font
    StartMenuScreen(int windowW,
                    int windowH,
                    ProjectManager& projectManager,
                    TTF_Font* titleFont,
                    TTF_Font* font);

    // ---- Event handling ----

    bool handleEvent(const SDL_Event& event);

    // ---- Draw ----

    void draw(SDL_Renderer* renderer) const;

    // ---- Action polling ----

    MenuAction getAction() const;
    void       clearAction();

    // Valid after a NEW_PROJECT action
    float getCanvasWidth()  const;
    float getCanvasHeight() const;

    // Valid after an OPEN_PROJECT action
    string getSelectedProjectPath() const;

    // ---- Refresh ----

    // Re-reads the recent projects list from ProjectManager.
    // Call this whenever returning to the start menu.
    void refresh();

private:

    // ---- Button definition ----

    enum class ButtonId
    {
        NONE,
        NEW_PROJECT,
        OPEN_PROJECT,
        EXIT,
        PRESET_A4_PORTRAIT,
        PRESET_A4_LANDSCAPE,
        PRESET_A3_LANDSCAPE,
        PRESET_A2_LANDSCAPE,
        CREATE,
        BACK
    };

    struct Button
    {
        SDL_Rect rect;
        string   label;
        ButtonId id;
        bool     isHovered;
        bool     isPrimary;      // drawn in accent colour
        bool     isActive;       // used by the preset toggle group

        Button()
            : id(ButtonId::NONE), isHovered(false),
              isPrimary(false), isActive(false)
        {}
    };

    // ---- Layout constants ----
    static const int BUTTON_W;
    static const int BUTTON_H;
    static const int BUTTON_GAP;
    static const int RECENT_LINE_H;
    static const int FIELD_H;

    int windowW;
    int windowH;

    ProjectManager& projectManager;
    TTF_Font*       titleFont;      
    TTF_Font*       font;          

    ScreenState state;
    MenuAction  action;

    // ---- Main menu buttons ----
    vector<Button> mainButtons;

    // ---- New project config buttons ----
    vector<Button> configButtons;

    // ---- Open project list ----
    vector<string> recentProjects;
    int            hoveredRecentIndex;
    int            recentScrollOffset;
    string         selectedProjectPath;

    // ---- Canvas size state ----
    float canvasWidth;
    float canvasHeight;

    // Custom size text fields
    string customWidthText;
    string customHeightText;
    bool   customWidthFocused;
    bool   customHeightFocused;

    SDL_Rect customWidthRect;
    SDL_Rect customHeightRect;

    // ---- Areas ----
    SDL_Rect recentPanelRect;      // main menu recent list area
    SDL_Rect openListRect;         // open-project state list area

    // ---- Colors ----
    SDL_Color bgColor;
    SDL_Color panelColor;
    SDL_Color borderColor;
    SDL_Color titleColor;
    SDL_Color subtitleColor;
    SDL_Color textColor;
    SDL_Color textDimColor;
    SDL_Color buttonColor;
    SDL_Color buttonHoverColor;
    SDL_Color buttonPrimaryColor;
    SDL_Color buttonPrimaryHoverColor;
    SDL_Color buttonActiveColor;
    SDL_Color fieldBgColor;
    SDL_Color fieldBorderColor;
    SDL_Color fieldActiveBorderColor;

    // ---- Setup ----

    void buildMainButtons();
    void buildConfigButtons();
    void computeAreas();

    // ---- Preset helper ----

    // Applies a preset size and marks the matching button active
    void applyPreset(ButtonId presetId);

    // Reads the custom text fields into canvasWidth/canvasHeight.
    // Returns false if either value is not a valid positive number.
    bool applyCustomSize();

    // ---- Event sub-handlers ----

    bool handleMainMenuEvent(const SDL_Event& event);
    bool handleConfigEvent(const SDL_Event& event);
    bool handleOpenListEvent(const SDL_Event& event);

    // ---- Draw sub-routines ----

    void drawMainMenu(SDL_Renderer* renderer)   const;
    void drawConfig(SDL_Renderer* renderer)     const;
    void drawOpenList(SDL_Renderer* renderer)   const;

    void drawButton(SDL_Renderer* renderer, const Button& btn) const;

    void drawTextField(SDL_Renderer* renderer,
                       const SDL_Rect& rect,
                       const string& value,
                       bool focused) const;

    void renderText(SDL_Renderer* renderer,
                    const string& text,
                    int x, int y,
                    SDL_Color color,
                    bool useTitleFont = false) const;

    // ---- Helpers ----

    // Strips directory and extension from a file path for display
    string prettyProjectName(const string& path) const;

    int  buttonIndexAt(const vector<Button>& list,
                       int mouseX, int mouseY) const;

    int  recentIndexAt(const SDL_Rect& area,
                       int scrollOffset,
                       int mouseX, int mouseY) const;

    int  visibleRecentCount(const SDL_Rect& area) const;
};