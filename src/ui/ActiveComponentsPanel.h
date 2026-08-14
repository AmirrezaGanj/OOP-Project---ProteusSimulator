#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

// ============================================================
// ActiveComponentsPanel — the quick-access panel showing
// components the user has added for fast placement.
//
// This is the "Devices" panel in Proteus — a short personal
// list of the components the user is currently working with,
// so they don't have to search the full library every time.
//
// Workflow:
//   1. User finds a component in ComponentLibraryPanel
//   2. MainEditorScreen calls addComponent() to pin it here
//   3. User clicks an item in this panel to select it
//   4. Canvas reads getSelectedComponentType() and places it
//   5. User can remove items by clicking the [X] on each row
//
// ============================================================

class ActiveComponentsPanel
{
public:

    struct ActiveEntry
    {
        string type;
        string displayName;

        ActiveEntry(const string& type, const string& displayName)
            : type(type), displayName(displayName)
        {}
    };

    // panelRect: screen rectangle for this panel
    ActiveComponentsPanel(const SDL_Rect& panelRect, TTF_Font* font);

    // ---- Component management ----

    // Adds a component to the active list.
    // Does nothing if the same type already exists in the list.
    void addComponent(const string& type, const string& displayName);

    // Removes the component at the given list index
    void removeComponent(int index);

    // Clears the entire active list
    void clear();

    // ---- Event handling ----

    // Returns true if event was consumed
    bool handleEvent(const SDL_Event& event);

    // ---- Draw ----

    void draw(SDL_Renderer* renderer) const;

    // ---- Result access ----

    // Returns the type of the currently selected component, or ""
    string getSelectedComponentType() const;

    // Clears the selection (called after Canvas places a component)
    void clearSelection();

    // Returns whether the panel wants to signal "add from library"
    // MainEditorScreen checks this and opens ComponentLibraryPanel
    bool wantsAddFromLibrary() const;
    void clearAddRequest();

private:

    static const int HEADER_HEIGHT = 28;
    static const int LINE_HEIGHT   = 26;
    static const int REMOVE_BTN_W  = 24;

    SDL_Rect  panelRect;
    TTF_Font* font;               

    vector<ActiveEntry> entries;

    int  selectedIndex;             // -1 = none
    int  hoveredIndex;              // -1 = none
    int  hoveredRemoveIndex;        // -1 = no remove button hovered
    int  scrollOffset;

    bool addButtonHovered;
    bool addRequested;              // set true when user clicks [+ADD]

    // Colors
    SDL_Color bgColor;
    SDL_Color headerColor;
    SDL_Color borderColor;
    SDL_Color textColor;
    SDL_Color textDimColor;
    SDL_Color selectedColor;
    SDL_Color hoveredColor;
    SDL_Color removeBtnColor;
    SDL_Color removeBtnHoverColor;
    SDL_Color addBtnColor;
    SDL_Color addBtnHoverColor;

    // ---- Helpers ----

    int  visibleCount()            const;
    int  entryIndexAt(int x, int y) const;
    int  removeButtonIndexAt(int x, int y) const;
    bool isOverAddButton(int x, int y) const;

    void renderText(SDL_Renderer* renderer,
                    const string& text,
                    int x, int y,
                    SDL_Color color) const;
};