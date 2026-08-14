#include "ui/ComponentLibraryPanel.h"
#include <algorithm>

using namespace std;

// ============================================================
// ComponentLibraryPanel.cpp
// ============================================================

//const int ComponentLibraryPanel::SEARCH_HEIGHT   = 32;
//const int ComponentLibraryPanel::CATEGORY_HEIGHT = 130;
//const int ComponentLibraryPanel::PREVIEW_HEIGHT  = 80;
//const int ComponentLibraryPanel::LINE_HEIGHT     = 22;
//const int ComponentLibraryPanel::HEADER_HEIGHT   = 24;


// ---- Constructor ----

ComponentLibraryPanel::ComponentLibraryPanel(const SDL_Rect& panelRect,
                                              TTF_Font* font)
    : panelRect(panelRect),
      font(font),
      searchText(""),
      searchBarActive(false),
      selectedCategory(""),
      selectedEntryIndex(-1),
      listScrollOffset(0),
      hoveredCategoryIndex(-1),
      hoveredEntryIndex(-1)
{
    bgColor                = { 38,  38,  42,  255 };
    headerColor            = { 48,  48,  53,  255 };
    borderColor            = { 60,  60,  65,  255 };
    textColor              = { 210, 210, 210, 255 };
    textDimColor           = { 120, 120, 125, 255 };
    selectedColor          = { 0,   100, 200, 255 };
    hoveredColor           = { 55,  55,  60,  255 };
    searchBgColor          = { 28,  28,  32,  255 };
    searchActiveBorderColor= { 0,   120, 215, 255 };
    categoryActiveColor    = { 0,   80,  160, 255 };

    buildRegistry();
    computeSubRects();
    rebuildFilteredList();
}


// ---- Registry ----

void ComponentLibraryPanel::buildRegistry()
{
    allComponents.clear();

    // Sources
    allComponents.push_back(ComponentEntry(
        "GND",       "GND",           "Sources",
        "Ground reference node. Voltage = 0V."));
    allComponents.push_back(ComponentEntry(
        "DCVOLTAGE", "DC Voltage",    "Sources",
        "Ideal DC voltage source. Configurable voltage."));
    allComponents.push_back(ComponentEntry(
        "BATTERY",   "Battery",       "Sources",
        "DC source with internal resistance."));
    allComponents.push_back(ComponentEntry(
        "CLOCK",     "Clock",         "Sources",
        "Square wave generator. Configurable frequency."));

    // Passive
    allComponents.push_back(ComponentEntry(
        "RESISTOR",  "Resistor",      "Passive",
        "Ohmic resistor. V = I * R."));
    allComponents.push_back(ComponentEntry(
        "CAPACITOR", "Capacitor",     "Passive",
        "Capacitor. I = C * dV/dt."));
    allComponents.push_back(ComponentEntry(
        "INDUCTOR",  "Inductor",      "Passive",
        "Inductor. V = L * dI/dt."));

    // Interactive
    allComponents.push_back(ComponentEntry(
        "SWITCH",    "Switch",        "Interactive",
        "Latching on/off switch. Click to toggle."));
    allComponents.push_back(ComponentEntry(
        "BUTTON",    "Push Button",   "Interactive",
        "Momentary button. HIGH while held, LOW on release."));
    allComponents.push_back(ComponentEntry(
        "LED",       "LED",           "Interactive",
        "Light emitting diode. Lights up when forward biased."));
    allComponents.push_back(ComponentEntry(
        "7SEG",      "7-Segment",     "Interactive",
        "7-segment display. 8 independent segment pins."));

    // Gates
    allComponents.push_back(ComponentEntry(
        "AND",       "AND Gate",      "Gates",
        "Output HIGH only when all inputs are HIGH."));
    allComponents.push_back(ComponentEntry(
        "OR",        "OR Gate",       "Gates",
        "Output HIGH when at least one input is HIGH."));
    allComponents.push_back(ComponentEntry(
        "NOT",       "NOT Gate",      "Gates",
        "Inverts the single input."));
    allComponents.push_back(ComponentEntry(
        "NAND",      "NAND Gate",     "Gates",
        "NOT AND. Output LOW only when all inputs HIGH."));
    allComponents.push_back(ComponentEntry(
        "XOR",       "XOR Gate",      "Gates",
        "Output HIGH when odd number of inputs are HIGH."));
    allComponents.push_back(ComponentEntry(
        "DFF",       "D Flip-Flop",   "Gates",
        "Edge-triggered. Captures D on rising CLK edge."));
}


void ComponentLibraryPanel::computeSubRects()
{
    int x = panelRect.x;
    int y = panelRect.y;
    int w = panelRect.w;

    searchRect = { x, y, w, SEARCH_HEIGHT };

    y += SEARCH_HEIGHT;
    categoryRect = { x, y, w, CATEGORY_HEIGHT };

    y += CATEGORY_HEIGHT;
    int listH = panelRect.h - SEARCH_HEIGHT - CATEGORY_HEIGHT - PREVIEW_HEIGHT;
    listRect = { x, y, w, listH };

    y += listH;
    previewRect = { x, y, w, PREVIEW_HEIGHT };
}


void ComponentLibraryPanel::rebuildFilteredList()
{
    filteredIndices.clear();

    string lowerSearch = searchText;
    for (int i = 0; i < (int)lowerSearch.size(); i++)
    {
        lowerSearch[i] = tolower(lowerSearch[i]);
    }

    for (int i = 0; i < (int)allComponents.size(); i++)
    {
        const ComponentEntry& entry = allComponents[i];

        // Category filter
        if (!selectedCategory.empty() && entry.category != selectedCategory)
        {
            continue;
        }

        // Search filter — match against display name or type
        if (!lowerSearch.empty())
        {
            string lowerName = entry.displayName;
            string lowerType = entry.type;

            for (int j = 0; j < (int)lowerName.size(); j++)
                lowerName[j] = tolower(lowerName[j]);
            for (int j = 0; j < (int)lowerType.size(); j++)
                lowerType[j] = tolower(lowerType[j]);

            bool nameMatch = (lowerName.find(lowerSearch) != string::npos);
            bool typeMatch = (lowerType.find(lowerSearch) != string::npos);

            if (!nameMatch && !typeMatch)
            {
                continue;
            }
        }

        filteredIndices.push_back(i);
    }

    // Reset selection and scroll when filter changes
    selectedEntryIndex = -1;
    listScrollOffset   = 0;
}


// ---- Event handling ----

bool ComponentLibraryPanel::handleEvent(const SDL_Event& event)
{
    SDL_Point mousePos = { 0, 0 };
    SDL_GetMouseState(&mousePos.x, &mousePos.y);

    // ---- Mouse motion: update hover states ----
    if (event.type == SDL_MOUSEMOTION)
    {
        SDL_Point pt = { event.motion.x, event.motion.y };

        if (SDL_PointInRect(&pt, &panelRect) == SDL_TRUE)
        {
            hoveredCategoryIndex = categoryIndexAt(pt.x, pt.y);
            hoveredEntryIndex    = entryIndexAt(pt.x, pt.y);
            return true;
        }
        else
        {
            hoveredCategoryIndex = -1;
            hoveredEntryIndex    = -1;
        }

        return false;
    }

    // ---- Mouse button down ----
    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        SDL_Point pt = { event.button.x, event.button.y };

        if (SDL_PointInRect(&pt, &panelRect) != SDL_TRUE)
        {
            searchBarActive = false;
            return false;
        }

        // Search bar click
        if (SDL_PointInRect(&pt, &searchRect) == SDL_TRUE)
        {
            searchBarActive = true;
            SDL_StartTextInput();
            return true;
        }
        else
        {
            searchBarActive = false;
        }

        // Category click
        int catIdx = categoryIndexAt(pt.x, pt.y);
        if (catIdx >= 0)
        {
            vector<string> cats = getUniqueCategories();

            // Index 0 = "All"
            if (catIdx == 0)
            {
                selectedCategory = "";
            }
            else if (catIdx - 1 < (int)cats.size())
            {
                selectedCategory = cats[catIdx - 1];
            }

            rebuildFilteredList();
            return true;
        }

        // Component list click
        int entryIdx = entryIndexAt(pt.x, pt.y);
        if (entryIdx >= 0)
        {
            selectedEntryIndex = entryIdx;
            return true;
        }

        return true;    // consume all clicks inside the panel
    }

    // ---- Mouse wheel: scroll component list ----
    if (event.type == SDL_MOUSEWHEEL)
    {
        if (SDL_PointInRect(&mousePos, &listRect) == SDL_TRUE)
        {
            listScrollOffset -= event.wheel.y;

            int maxScroll = (int)filteredIndices.size() - visibleEntryCount();
            if (maxScroll < 0) maxScroll = 0;
            if (listScrollOffset < 0)         listScrollOffset = 0;
            if (listScrollOffset > maxScroll) listScrollOffset = maxScroll;

            return true;
        }
    }

    // ---- Text input for search bar ----
    if (event.type == SDL_TEXTINPUT && searchBarActive)
    {
        searchText += event.text.text;
        rebuildFilteredList();
        return true;
    }

    // ---- Keyboard for search bar ----
    if (event.type == SDL_KEYDOWN && searchBarActive)
    {
        if (event.key.keysym.sym == SDLK_BACKSPACE && !searchText.empty())
        {
            searchText.pop_back();
            rebuildFilteredList();
            return true;
        }
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            searchBarActive = false;
            SDL_StopTextInput();
            return true;
        }
    }

    return false;
}


// ---- Draw ----

void ComponentLibraryPanel::draw(SDL_Renderer* renderer) const
{
    // Panel background
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &panelRect);

    drawSearchBar(renderer);
    drawCategoryList(renderer);
    drawComponentList(renderer);
    drawPreview(renderer);

    // Right border
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawLine(renderer,
                       panelRect.x + panelRect.w - 1, panelRect.y,
                       panelRect.x + panelRect.w - 1, panelRect.y + panelRect.h);
}


void ComponentLibraryPanel::drawSearchBar(SDL_Renderer* renderer) const
{
    // Background
    SDL_SetRenderDrawColor(renderer,
                           searchBgColor.r, searchBgColor.g,
                           searchBgColor.b, searchBgColor.a);
    SDL_RenderFillRect(renderer, &searchRect);

    // Border — highlighted when active
    if (searchBarActive)
    {
        SDL_SetRenderDrawColor(renderer,
                               searchActiveBorderColor.r,
                               searchActiveBorderColor.g,
                               searchActiveBorderColor.b,
                               searchActiveBorderColor.a);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer,
                               borderColor.r, borderColor.g,
                               borderColor.b, borderColor.a);
    }
    SDL_RenderDrawRect(renderer, &searchRect);

    // Placeholder or typed text
    string displayText = searchText.empty() ? "Search components..." : searchText;
    SDL_Color col      = searchText.empty() ? textDimColor : textColor;

    renderText(renderer, displayText,
               searchRect.x + 8,
               searchRect.y + (searchRect.h - 14) / 2,
               col);

    // Blinking cursor when active (always show for simplicity)
    if (searchBarActive)
    {
        int cursorX = searchRect.x + 8 + (int)searchText.size() * 8;
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawLine(renderer,
                           cursorX,
                           searchRect.y + 6,
                           cursorX,
                           searchRect.y + searchRect.h - 6);
    }
}


void ComponentLibraryPanel::drawCategoryList(SDL_Renderer* renderer) const
{
    // Header
    SDL_SetRenderDrawColor(renderer,
                           headerColor.r, headerColor.g,
                           headerColor.b, headerColor.a);
    SDL_RenderFillRect(renderer, &categoryRect);

    renderText(renderer, "Categories",
               categoryRect.x + 8,
               categoryRect.y + 4,
               textDimColor);

    // Separator
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawLine(renderer,
                       categoryRect.x,     categoryRect.y + HEADER_HEIGHT,
                       categoryRect.x + categoryRect.w, categoryRect.y + HEADER_HEIGHT);

    // Category items: "All" + each unique category
    vector<string> cats = getUniqueCategories();

    int y = categoryRect.y + HEADER_HEIGHT + 2;

    // Draw "All" entry
    SDL_Rect allRect = { categoryRect.x, y, categoryRect.w, LINE_HEIGHT };
    bool allSelected = selectedCategory.empty();
    bool allHovered  = (hoveredCategoryIndex == 0);

    SDL_Color allBg = allSelected ? categoryActiveColor
                    : allHovered  ? hoveredColor
                    : bgColor;

    SDL_SetRenderDrawColor(renderer, allBg.r, allBg.g, allBg.b, allBg.a);
    SDL_RenderFillRect(renderer, &allRect);
    renderText(renderer, "  All",
               categoryRect.x + 4, y + 3,
               allSelected ? textColor : textDimColor);
    y += LINE_HEIGHT;

    for (int i = 0; i < (int)cats.size(); i++)
    {
        SDL_Rect catRect = { categoryRect.x, y, categoryRect.w, LINE_HEIGHT };
        bool isSelected  = (selectedCategory == cats[i]);
        bool isHovered   = (hoveredCategoryIndex == i + 1);

        SDL_Color catBg = isSelected ? categoryActiveColor
                        : isHovered  ? hoveredColor
                        : bgColor;

        SDL_SetRenderDrawColor(renderer, catBg.r, catBg.g, catBg.b, catBg.a);
        SDL_RenderFillRect(renderer, &catRect);

        renderText(renderer, "  " + cats[i],
                   categoryRect.x + 4, y + 3,
                   isSelected ? textColor : textDimColor);

        y += LINE_HEIGHT;
    }

    // Bottom border
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawLine(renderer,
                       categoryRect.x,
                       categoryRect.y + categoryRect.h - 1,
                       categoryRect.x + categoryRect.w,
                       categoryRect.y + categoryRect.h - 1);
}


void ComponentLibraryPanel::drawComponentList(SDL_Renderer* renderer) const
{
    // Background
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &listRect);

    // Header
    SDL_Rect listHeader = { listRect.x, listRect.y, listRect.w, HEADER_HEIGHT };
    SDL_SetRenderDrawColor(renderer,
                           headerColor.r, headerColor.g,
                           headerColor.b, headerColor.a);
    SDL_RenderFillRect(renderer, &listHeader);

    string headerLabel = "Components (" + to_string(filteredIndices.size()) + ")";
    renderText(renderer, headerLabel,
               listRect.x + 8, listRect.y + 4,
               textDimColor);

    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawLine(renderer,
                       listRect.x,      listRect.y + HEADER_HEIGHT,
                       listRect.x + listRect.w, listRect.y + HEADER_HEIGHT);

    if (filteredIndices.empty())
    {
        renderText(renderer, "No components found.",
                   listRect.x + 8,
                   listRect.y + HEADER_HEIGHT + 8,
                   textDimColor);
        return;
    }

    int visCount  = visibleEntryCount();
    int startIdx  = listScrollOffset;
    int endIdx    = startIdx + visCount;
    if (endIdx > (int)filteredIndices.size())
    {
        endIdx = (int)filteredIndices.size();
    }

    int y = listRect.y + HEADER_HEIGHT + 2;

    for (int i = startIdx; i < endIdx; i++)
    {
        int compIdx = filteredIndices[i];
        const ComponentEntry& entry = allComponents[compIdx];

        SDL_Rect entryRect = { listRect.x, y, listRect.w, LINE_HEIGHT };

        bool isSelected = (selectedEntryIndex == i);
        bool isHovered  = (hoveredEntryIndex  == i);

        SDL_Color entryBg = isSelected ? selectedColor
                          : isHovered  ? hoveredColor
                          : bgColor;

        SDL_SetRenderDrawColor(renderer,
                               entryBg.r, entryBg.g, entryBg.b, entryBg.a);
        SDL_RenderFillRect(renderer, &entryRect);

        renderText(renderer, entry.displayName,
                   listRect.x + 8, y + 3,
                   isSelected ? textColor : textColor);

        y += LINE_HEIGHT;
    }

    // Scroll indicator
    if ((int)filteredIndices.size() > visCount)
    {
        int listH      = listRect.h - HEADER_HEIGHT;
        float ratio    = (float)visCount / (float)filteredIndices.size();
        int   barH     = (int)(listH * ratio);
        if (barH < 10) barH = 10;

        float scrollR  = (float)listScrollOffset /
                         (float)(filteredIndices.size() - visCount);
        int   barY     = listRect.y + HEADER_HEIGHT + (int)((listH - barH) * scrollR);

        SDL_Rect scrollBar = { listRect.x + listRect.w - 5, barY, 4, barH };
        SDL_SetRenderDrawColor(renderer, 80, 80, 88, 255);
        SDL_RenderFillRect(renderer, &scrollBar);
    }
}


void ComponentLibraryPanel::drawPreview(SDL_Renderer* renderer) const
{
    // Background
    SDL_SetRenderDrawColor(renderer,
                           headerColor.r, headerColor.g,
                           headerColor.b, headerColor.a);
    SDL_RenderFillRect(renderer, &previewRect);

    // Top border
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawLine(renderer,
                       previewRect.x,
                       previewRect.y,
                       previewRect.x + previewRect.w,
                       previewRect.y);

    if (selectedEntryIndex < 0 ||
        selectedEntryIndex >= (int)filteredIndices.size())
    {
        renderText(renderer, "Select a component",
                   previewRect.x + 8,
                   previewRect.y + 8,
                   textDimColor);
        return;
    }

    int compIdx = filteredIndices[selectedEntryIndex];
    const ComponentEntry& entry = allComponents[compIdx];

    // Component name (bold via larger text not possible without bold font,
    // so we repeat for emphasis)
    renderText(renderer, entry.displayName,
               previewRect.x + 8, previewRect.y + 8,
               textColor);

    // Type string in dim color
    renderText(renderer, "Type: " + entry.type,
               previewRect.x + 8, previewRect.y + 26,
               textDimColor);

    // Description
    renderText(renderer, entry.description,
               previewRect.x + 8, previewRect.y + 44,
               textDimColor);
}


// ---- Private helpers ----

vector<string> ComponentLibraryPanel::getUniqueCategories() const
{
    vector<string> cats;

    for (int i = 0; i < (int)allComponents.size(); i++)
    {
        const string& cat = allComponents[i].category;
        bool found = false;

        for (int j = 0; j < (int)cats.size(); j++)
        {
            if (cats[j] == cat)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            cats.push_back(cat);
        }
    }

    return cats;
}


void ComponentLibraryPanel::renderText(SDL_Renderer* renderer,
                                        const string& text,
                                        int x, int y,
                                        SDL_Color color) const
{
    if (font == nullptr || text.empty())
    {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surface == nullptr)
    {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture == nullptr)
    {
        return;
    }

    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);

    int maxW = panelRect.w - (x - panelRect.x) - 6;
    if (texW > maxW) texW = maxW;

    SDL_Rect destRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
}


int ComponentLibraryPanel::categoryIndexAt(int mouseX, int mouseY) const
{
    vector<string> cats = getUniqueCategories();
    int y = categoryRect.y + HEADER_HEIGHT + 2;

    // Check "All" row
    if (mouseY >= y && mouseY < y + LINE_HEIGHT &&
        mouseX >= categoryRect.x && mouseX < categoryRect.x + categoryRect.w)
    {
        return 0;
    }
    y += LINE_HEIGHT;

    for (int i = 0; i < (int)cats.size(); i++)
    {
        if (mouseY >= y && mouseY < y + LINE_HEIGHT &&
            mouseX >= categoryRect.x && mouseX < categoryRect.x + categoryRect.w)
        {
            return i + 1;
        }
        y += LINE_HEIGHT;
    }

    return -1;
}


int ComponentLibraryPanel::entryIndexAt(int mouseX, int mouseY) const
{
    int y = listRect.y + HEADER_HEIGHT + 2;

    if (mouseX < listRect.x || mouseX >= listRect.x + listRect.w)
    {
        return -1;
    }

    int visCount = visibleEntryCount();
    int endIdx   = listScrollOffset + visCount;
    if (endIdx > (int)filteredIndices.size())
    {
        endIdx = (int)filteredIndices.size();
    }

    for (int i = listScrollOffset; i < endIdx; i++)
    {
        if (mouseY >= y && mouseY < y + LINE_HEIGHT)
        {
            return i;
        }
        y += LINE_HEIGHT;
    }

    return -1;
}


int ComponentLibraryPanel::visibleEntryCount() const
{
    int usableH = listRect.h - HEADER_HEIGHT - 4;
    return usableH / LINE_HEIGHT;
}


// ---- Result access ----

string ComponentLibraryPanel::getSelectedComponentType() const
{
    if (selectedEntryIndex < 0 ||
        selectedEntryIndex >= (int)filteredIndices.size())
    {
        return "";
    }

    int compIdx = filteredIndices[selectedEntryIndex];
    return allComponents[compIdx].type;
}

void ComponentLibraryPanel::clearSelection()
{
    selectedEntryIndex = -1;
}