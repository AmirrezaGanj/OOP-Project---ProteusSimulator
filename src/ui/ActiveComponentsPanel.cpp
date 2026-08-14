#include "ui/ActiveComponentsPanel.h"

using namespace std;

// ============================================================
// ActiveComponentsPanel.cpp
// ============================================================

//const int ActiveComponentsPanel::HEADER_HEIGHT = 28;
//const int ActiveComponentsPanel::LINE_HEIGHT   = 26;
//const int ActiveComponentsPanel::REMOVE_BTN_W  = 24;


// ---- Constructor ----

ActiveComponentsPanel::ActiveComponentsPanel(const SDL_Rect& panelRect,
                                              TTF_Font* font)
    : panelRect(panelRect),
      font(font),
      selectedIndex(-1),
      hoveredIndex(-1),
      hoveredRemoveIndex(-1),
      scrollOffset(0),
      addButtonHovered(false),
      addRequested(false)
{
    bgColor           = { 38,  38,  42,  255 };
    headerColor       = { 48,  48,  53,  255 };
    borderColor       = { 60,  60,  65,  255 };
    textColor         = { 210, 210, 210, 255 };
    textDimColor      = { 120, 120, 125, 255 };
    selectedColor     = { 0,   100, 200, 255 };
    hoveredColor      = { 55,  55,  60,  255 };
    removeBtnColor    = { 90,  40,  40,  255 };
    removeBtnHoverColor={ 160, 50,  50,  255 };
    addBtnColor       = { 40,  80,  40,  255 };
    addBtnHoverColor  = { 50,  120, 50,  255 };
}


// ---- Component management ----

void ActiveComponentsPanel::addComponent(const string& type,
                                          const string& displayName)
{
    // Prevent duplicates
    for (int i = 0; i < (int)entries.size(); i++)
    {
        if (entries[i].type == type)
        {
            return;
        }
    }

    entries.push_back(ActiveEntry(type, displayName));
}

void ActiveComponentsPanel::removeComponent(int index)
{
    if (index < 0 || index >= (int)entries.size())
    {
        return;
    }

    entries.erase(entries.begin() + index);

    // Adjust selectedIndex if needed
    if (selectedIndex == index)
    {
        selectedIndex = -1;
    }
    else if (selectedIndex > index)
    {
        selectedIndex--;
    }

    // Clamp scrollOffset
    int maxScroll = (int)entries.size() - visibleCount();
    if (maxScroll < 0)          maxScroll = 0;
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
}

void ActiveComponentsPanel::clear()
{
    entries.clear();
    selectedIndex = -1;
    scrollOffset  = 0;
}


// ---- Event handling ----

bool ActiveComponentsPanel::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        int mx = event.motion.x;
        int my = event.motion.y;

        SDL_Point pt = { mx, my };
        if (SDL_PointInRect(&pt, &panelRect) != SDL_TRUE)
        {
            hoveredIndex       = -1;
            hoveredRemoveIndex = -1;
            addButtonHovered   = false;
            return false;
        }

        hoveredIndex       = entryIndexAt(mx, my);
        hoveredRemoveIndex = removeButtonIndexAt(mx, my);
        addButtonHovered   = isOverAddButton(mx, my);

        return true;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        int mx = event.button.x;
        int my = event.button.y;

        SDL_Point pt = { mx, my };
        if (SDL_PointInRect(&pt, &panelRect) != SDL_TRUE)
        {
            return false;
        }

        // Add button click
        if (isOverAddButton(mx, my))
        {
            addRequested = true;
            return true;
        }

        // Remove button click — check before entry click
        // so clicking [X] doesn't also select the entry
        int removeIdx = removeButtonIndexAt(mx, my);
        if (removeIdx >= 0)
        {
            removeComponent(removeIdx);
            return true;
        }

        // Entry row click — select it
        int entryIdx = entryIndexAt(mx, my);
        if (entryIdx >= 0)
        {
            selectedIndex = entryIdx;
            return true;
        }

        return true;    // consume all clicks inside the panel
    }

    // Mouse wheel scrolling
    if (event.type == SDL_MOUSEWHEEL)
    {
        SDL_Point mousePos;
        SDL_GetMouseState(&mousePos.x, &mousePos.y);

        if (SDL_PointInRect(&mousePos, &panelRect) == SDL_TRUE)
        {
            scrollOffset -= event.wheel.y;

            int maxScroll = (int)entries.size() - visibleCount();
            if (maxScroll < 0)          maxScroll = 0;
            if (scrollOffset < 0)         scrollOffset = 0;
            if (scrollOffset > maxScroll) scrollOffset = maxScroll;

            return true;
        }
    }

    return false;
}


// ---- Draw ----

void ActiveComponentsPanel::draw(SDL_Renderer* renderer) const
{
    // Background
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &panelRect);

    // ---- Header ----
    SDL_Rect headerRect = { panelRect.x, panelRect.y,
                             panelRect.w, HEADER_HEIGHT };

    SDL_SetRenderDrawColor(renderer,
                           headerColor.r, headerColor.g,
                           headerColor.b, headerColor.a);
    SDL_RenderFillRect(renderer, &headerRect);

    renderText(renderer, "Active Components",
               panelRect.x + 8,
               panelRect.y + (HEADER_HEIGHT - 14) / 2,
               textDimColor);

    // [+ ADD] button in the header right side
    int addBtnW = 40;
    int addBtnH = HEADER_HEIGHT - 8;
    SDL_Rect addBtn = { panelRect.x + panelRect.w - addBtnW - 4,
                         panelRect.y + 4,
                         addBtnW, addBtnH };

    SDL_Color addCol = addButtonHovered ? addBtnHoverColor : addBtnColor;
    SDL_SetRenderDrawColor(renderer, addCol.r, addCol.g, addCol.b, addCol.a);
    SDL_RenderFillRect(renderer, &addBtn);

    SDL_SetRenderDrawColor(renderer, 30, 60, 30, 255);
    SDL_RenderDrawRect(renderer, &addBtn);

    renderText(renderer, "+ADD",
               addBtn.x + 4,
               addBtn.y + (addBtn.h - 14) / 2,
               textColor);

    // Header bottom border
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawLine(renderer,
                       panelRect.x,
                       panelRect.y + HEADER_HEIGHT,
                       panelRect.x + panelRect.w,
                       panelRect.y + HEADER_HEIGHT);

    // ---- Entry list ----
    if (entries.empty())
    {
        renderText(renderer, "No active components.",
                   panelRect.x + 8,
                   panelRect.y + HEADER_HEIGHT + 10,
                   textDimColor);
        renderText(renderer, "Click +ADD to add one.",
                   panelRect.x + 8,
                   panelRect.y + HEADER_HEIGHT + 28,
                   textDimColor);
    }
    else
    {
        int visCount = visibleCount();
        int startIdx = scrollOffset;
        int endIdx   = startIdx + visCount;

        if (endIdx > (int)entries.size())
        {
            endIdx = (int)entries.size();
        }

        int y = panelRect.y + HEADER_HEIGHT + 2;

        for (int i = startIdx; i < endIdx; i++)
        {
            SDL_Rect rowRect = { panelRect.x, y, panelRect.w, LINE_HEIGHT };

            bool isSelected = (selectedIndex == i);
            bool isHovered  = (hoveredIndex  == i);

            SDL_Color rowBg = isSelected ? selectedColor
                            : isHovered  ? hoveredColor
                            : bgColor;

            SDL_SetRenderDrawColor(renderer,
                                   rowBg.r, rowBg.g, rowBg.b, rowBg.a);
            SDL_RenderFillRect(renderer, &rowRect);

            // Row separator line
            SDL_SetRenderDrawColor(renderer,
                                   borderColor.r, borderColor.g,
                                   borderColor.b, borderColor.a);
            SDL_RenderDrawLine(renderer,
                               panelRect.x,
                               y + LINE_HEIGHT - 1,
                               panelRect.x + panelRect.w,
                               y + LINE_HEIGHT - 1);

            // Component display name
            renderText(renderer, entries[i].displayName,
                       panelRect.x + 8,
                       y + (LINE_HEIGHT - 14) / 2,
                       isSelected ? textColor : textColor);

            // [X] remove button on the right
            SDL_Rect removeBtn = { panelRect.x + panelRect.w - REMOVE_BTN_W - 2,
                                    y + 3,
                                    REMOVE_BTN_W - 2,
                                    LINE_HEIGHT - 6 };

            bool removeHovered = (hoveredRemoveIndex == i);

            SDL_Color removeCol = removeHovered ? removeBtnHoverColor
                                                : removeBtnColor;
            SDL_SetRenderDrawColor(renderer,
                                   removeCol.r, removeCol.g,
                                   removeCol.b, removeCol.a);
            SDL_RenderFillRect(renderer, &removeBtn);

            renderText(renderer, "X",
                       removeBtn.x + (removeBtn.w - 8) / 2,
                       removeBtn.y + (removeBtn.h - 14) / 2,
                       textColor);

            y += LINE_HEIGHT;
        }

        // Scroll indicator
        if ((int)entries.size() > visCount)
        {
            int listH   = panelRect.h - HEADER_HEIGHT;
            float ratio = (float)visCount / (float)entries.size();
            int barH    = (int)(listH * ratio);
            if (barH < 10) barH = 10;

            float scrollR = (float)scrollOffset /
                            (float)(entries.size() - visCount);
            int barY = panelRect.y + HEADER_HEIGHT +
                       (int)((listH - barH) * scrollR);

            SDL_Rect scrollBar = { panelRect.x + panelRect.w - 5,
                                    barY, 4, barH };
            SDL_SetRenderDrawColor(renderer, 80, 80, 88, 255);
            SDL_RenderFillRect(renderer, &scrollBar);
        }
    }

    // Right border
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawLine(renderer,
                       panelRect.x + panelRect.w - 1, panelRect.y,
                       panelRect.x + panelRect.w - 1,
                       panelRect.y + panelRect.h);
}


// ---- Result access ----

string ActiveComponentsPanel::getSelectedComponentType() const
{
    if (selectedIndex < 0 || selectedIndex >= (int)entries.size())
    {
        return "";
    }
    return entries[selectedIndex].type;
}

void ActiveComponentsPanel::clearSelection()
{
    selectedIndex = -1;
}

bool ActiveComponentsPanel::wantsAddFromLibrary() const
{
    return addRequested;
}

void ActiveComponentsPanel::clearAddRequest()
{
    addRequested = false;
}


// ---- Private helpers ----

int ActiveComponentsPanel::visibleCount() const
{
    int usableH = panelRect.h - HEADER_HEIGHT - 4;
    return usableH / LINE_HEIGHT;
}

int ActiveComponentsPanel::entryIndexAt(int mouseX, int mouseY) const
{
    if (mouseX < panelRect.x || mouseX >= panelRect.x + panelRect.w)
    {
        return -1;
    }

    int y        = panelRect.y + HEADER_HEIGHT + 2;
    int visCount = visibleCount();
    int endIdx   = scrollOffset + visCount;

    if (endIdx > (int)entries.size())
    {
        endIdx = (int)entries.size();
    }

    for (int i = scrollOffset; i < endIdx; i++)
    {
        if (mouseY >= y && mouseY < y + LINE_HEIGHT)
        {
            return i;
        }
        y += LINE_HEIGHT;
    }

    return -1;
}

int ActiveComponentsPanel::removeButtonIndexAt(int mouseX, int mouseY) const
{
    int removeX  = panelRect.x + panelRect.w - REMOVE_BTN_W - 2;
    int y        = panelRect.y + HEADER_HEIGHT + 2;
    int visCount = visibleCount();
    int endIdx   = scrollOffset + visCount;

    if (endIdx > (int)entries.size())
    {
        endIdx = (int)entries.size();
    }

    for (int i = scrollOffset; i < endIdx; i++)
    {
        if (mouseX >= removeX &&
            mouseX <  removeX + REMOVE_BTN_W &&
            mouseY >= y + 3 &&
            mouseY <  y + LINE_HEIGHT - 3)
        {
            return i;
        }
        y += LINE_HEIGHT;
    }

    return -1;
}

bool ActiveComponentsPanel::isOverAddButton(int mouseX, int mouseY) const
{
    int addBtnW = 40;
    int addBtnH = HEADER_HEIGHT - 8;

    SDL_Rect addBtn = { panelRect.x + panelRect.w - addBtnW - 4,
                         panelRect.y + 4,
                         addBtnW, addBtnH };

    SDL_Point pt = { mouseX, mouseY };
    return (SDL_PointInRect(&pt, &addBtn) == SDL_TRUE);
}


void ActiveComponentsPanel::renderText(SDL_Renderer* renderer,
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

    int maxW = panelRect.w - (x - panelRect.x) - REMOVE_BTN_W - 6;
    if (texW > maxW) texW = maxW;

    SDL_Rect destRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
}