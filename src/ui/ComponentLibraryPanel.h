#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

// ============================================================
// ComponentLibraryPanel — the left-side panel showing all
// available component types grouped by category.

// When the user clicks a component in the list, it becomes
// the "selected" component type. The Canvas reads this via
// getSelectedComponentType() to know what to place next.
//
// Text input for the search bar is handled via SDL_TEXTINPUT
// and SDL_KEYDOWN events.
// ============================================================

class ComponentLibraryPanel
{
public:

    // ---- Component entry in the registry ----

    struct ComponentEntry
    {
        string type;          // matches Component::getType() e.g. "RESISTOR"
        string displayName;   // shown in the list e.g. "Resistor"
        string category;      // e.g. "Passive", "Gates", "Sources"
        string description;   // shown in the preview area

        ComponentEntry(const string& type,
                       const string& displayName,
                       const string& category,
                       const string& description)
            : type(type), displayName(displayName),
              category(category), description(description)
        {}
    };

    // panelRect: screen rectangle for this panel
    ComponentLibraryPanel(const SDL_Rect& panelRect, TTF_Font* font);

    // ---- Event handling ----

    // Returns true if event was consumed by this panel
    bool handleEvent(const SDL_Event& event);

    // ---- Draw ----

    void draw(SDL_Renderer* renderer) const;

    // ---- Result access ----

    // Returns the type string of the currently selected component,
    // or "" if nothing is selected.
    string getSelectedComponentType() const;

    // Clears the selection (called when Canvas places a component)
    void clearSelection();

private:

    // ---- UI section heights ----
    static const int SEARCH_HEIGHT   = 32;
    static const int CATEGORY_HEIGHT = 130;   // fixed area for category list
    static const int PREVIEW_HEIGHT  = 80;
    static const int LINE_HEIGHT     = 22;
    static const int HEADER_HEIGHT   = 24;

    SDL_Rect  panelRect;
    TTF_Font* font;         

    // ---- Component registry ----
    vector<ComponentEntry> allComponents;

    // ---- Filtered list (rebuilt whenever search or category changes) ----
    vector<int> filteredIndices;   // indices into allComponents

    // ---- State ----
    string searchText;
    bool   searchBarActive;        // true when user has clicked the search bar

    string selectedCategory;       // "" = All
    int    selectedEntryIndex;     // index into filteredIndices, -1 = none
    int    listScrollOffset;       // how many lines scrolled in the component list

    // Category hover
    int    hoveredCategoryIndex;

    // Component list hover
    int    hoveredEntryIndex;

    // ---- Sub-rects (computed from panelRect) ----
    SDL_Rect searchRect;
    SDL_Rect categoryRect;
    SDL_Rect listRect;
    SDL_Rect previewRect;

    // ---- Colors ----
    SDL_Color bgColor;
    SDL_Color headerColor;
    SDL_Color borderColor;
    SDL_Color textColor;
    SDL_Color textDimColor;
    SDL_Color selectedColor;
    SDL_Color hoveredColor;
    SDL_Color searchBgColor;
    SDL_Color searchActiveBorderColor;
    SDL_Color categoryActiveColor;

    // ---- Setup ----

    void buildRegistry();
    void computeSubRects();
    void rebuildFilteredList();

    // ---- Category helpers ----
    vector<string> getUniqueCategories() const;

    // ---- Drawing helpers ----

    void drawSearchBar(SDL_Renderer* renderer)   const;
    void drawCategoryList(SDL_Renderer* renderer) const;
    void drawComponentList(SDL_Renderer* renderer) const;
    void drawPreview(SDL_Renderer* renderer)      const;

    void renderText(SDL_Renderer* renderer,
                    const string& text,
                    int x, int y,
                    SDL_Color color) const;

    // ---- Hit testing ----

    int  categoryIndexAt(int mouseX, int mouseY) const;
    int  entryIndexAt(int mouseX, int mouseY)    const;
    int  visibleEntryCount()                     const;
};