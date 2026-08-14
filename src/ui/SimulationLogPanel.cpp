#include "ui/SimulationLogPanel.h"

using namespace std;

// ============================================================
// SimulationLogPanel.cpp
// ============================================================

//const int SimulationLogPanel::LINE_HEIGHT   = 18;
//const int SimulationLogPanel::HEADER_HEIGHT = 26;


// ---- Constructor ----

SimulationLogPanel::SimulationLogPanel(const SDL_Rect& panelRect,
                                        TTF_Font* font)
    : panelRect(panelRect),
      font(font),
      scrollOffset(0),
      clearButtonHovered(false)
{
    bgColor           = { 28,  28,  30,  255 };
    headerColor       = { 38,  38,  42,  255 };
    borderColor       = { 60,  60,  65,  255 };
    textNormalColor   = { 200, 200, 200, 255 };
    textErrorColor    = { 220, 70,  70,  255 };
    textWarningColor  = { 220, 150, 50,  255 };
    textDimColor      = { 120, 120, 120, 255 };
    clearBtnColor     = { 70,  70,  75,  255 };
    clearBtnHoverColor= { 100, 100, 106, 255 };

    // Clear button sits in the top-right corner of the header
    clearButtonRect.w = 50;
    clearButtonRect.h = HEADER_HEIGHT - 8;
    clearButtonRect.x = panelRect.x + panelRect.w - clearButtonRect.w - 6;
    clearButtonRect.y = panelRect.y + 4;
}


// ---- Message sync ----

void SimulationLogPanel::syncMessages(const vector<string>& logMessages)
{
    bool hadNewMessages = (logMessages.size() != messages.size());

    messages = logMessages;

    // Auto-scroll to bottom whenever new messages arrive
    if (hadNewMessages)
    {
        scrollToBottom();
    }
}

void SimulationLogPanel::clearMessages()
{
    messages.clear();
    scrollOffset = 0;
}


// ---- Event handling ----

bool SimulationLogPanel::handleEvent(const SDL_Event& event)
{
    // ---- Mouse wheel scrolling ----
    if (event.type == SDL_MOUSEWHEEL)
    {
        SDL_Point mousePos;
        SDL_GetMouseState(&mousePos.x, &mousePos.y);

        if (SDL_PointInRect(&mousePos, &panelRect) == SDL_TRUE)
        {
            // Scroll up = show older messages, scroll down = show newer
            scrollOffset -= event.wheel.y;

            // Clamp scroll range
            int maxScroll = (int)messages.size() - visibleLineCount();
            if (maxScroll < 0) maxScroll = 0;

            if (scrollOffset < 0)         scrollOffset = 0;
            if (scrollOffset > maxScroll) scrollOffset = maxScroll;

            return true;
        }
    }

    // ---- Mouse motion for clear button hover ----
    if (event.type == SDL_MOUSEMOTION)
    {
        SDL_Point pt = { event.motion.x, event.motion.y };
        clearButtonHovered = (SDL_PointInRect(&pt, &clearButtonRect) == SDL_TRUE);
    }

    // ---- Clear button click ----
    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        SDL_Point pt = { event.button.x, event.button.y };

        if (SDL_PointInRect(&pt, &clearButtonRect) == SDL_TRUE)
        {
            clearMessages();
            return true;
        }

        // Consume clicks anywhere on the panel so they don't
        // fall through to the canvas below
        if (SDL_PointInRect(&pt, &panelRect) == SDL_TRUE)
        {
            return true;
        }
    }

    return false;
}


// ---- Draw ----

void SimulationLogPanel::draw(SDL_Renderer* renderer) const
{
    // ---- Background ----
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &panelRect);

    // ---- Header bar ----
    SDL_Rect headerRect = { panelRect.x,
                             panelRect.y,
                             panelRect.w,
                             HEADER_HEIGHT };

    SDL_SetRenderDrawColor(renderer,
                           headerColor.r, headerColor.g,
                           headerColor.b, headerColor.a);
    SDL_RenderFillRect(renderer, &headerRect);

    // Header title
    renderText(renderer, "Simulation Log",
               panelRect.x + 8,
               panelRect.y + (HEADER_HEIGHT - 14) / 2,
               textDimColor);

    // ---- Clear button ----
    SDL_Color clearCol = clearButtonHovered ? clearBtnHoverColor : clearBtnColor;
    SDL_SetRenderDrawColor(renderer,
                           clearCol.r, clearCol.g, clearCol.b, clearCol.a);
    SDL_RenderFillRect(renderer, &clearButtonRect);

    SDL_SetRenderDrawColor(renderer, 50, 50, 55, 255);
    SDL_RenderDrawRect(renderer, &clearButtonRect);

    renderText(renderer, "Clear",
               clearButtonRect.x + 6,
               clearButtonRect.y + (clearButtonRect.h - 14) / 2,
               textNormalColor);

    // ---- Border ----
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &panelRect);

    // Top border line below header
    SDL_RenderDrawLine(renderer,
                       panelRect.x,
                       panelRect.y + HEADER_HEIGHT,
                       panelRect.x + panelRect.w,
                       panelRect.y + HEADER_HEIGHT);

    // ---- Message lines ----
    if (messages.empty())
    {
        renderText(renderer, "No messages.",
                   panelRect.x + 8,
                   panelRect.y + HEADER_HEIGHT + 6,
                   textDimColor);
        return;
    }

    int visibleCount = visibleLineCount();
    int startIndex   = scrollOffset;
    int endIndex     = startIndex + visibleCount;

    if (endIndex > (int)messages.size())
    {
        endIndex = (int)messages.size();
    }

    int textAreaY = panelRect.y + HEADER_HEIGHT + 4;

    for (int i = startIndex; i < endIndex; i++)
    {
        int lineY = textAreaY + (i - startIndex) * LINE_HEIGHT;

        // Line number prefix in dim color
        string prefix = to_string(i + 1) + ". ";
        renderText(renderer, prefix,
                   panelRect.x + 6,
                   lineY,
                   textDimColor);

        // Message text in color based on content
        SDL_Color msgColor = colorForMessage(messages[i]);
        renderText(renderer, messages[i],
                   panelRect.x + 30,
                   lineY,
                   msgColor);
    }

    // ---- Scroll indicator (thin bar on right edge) ----
    if ((int)messages.size() > visibleLineCount())
    {
        int textAreaHeight = panelRect.h - HEADER_HEIGHT;
        float ratio        = (float)visibleLineCount() / (float)messages.size();
        int   barHeight    = (int)(textAreaHeight * ratio);
        if (barHeight < 10) barHeight = 10;

        float scrollRatio  = (float)scrollOffset /
                             (float)(messages.size() - visibleLineCount());
        int   barY         = panelRect.y + HEADER_HEIGHT +
                             (int)((textAreaHeight - barHeight) * scrollRatio);

        SDL_Rect scrollBar = { panelRect.x + panelRect.w - 5,
                                barY,
                                4,
                                barHeight };

        SDL_SetRenderDrawColor(renderer, 80, 80, 88, 255);
        SDL_RenderFillRect(renderer, &scrollBar);
    }
}


// ---- Private helpers ----

int SimulationLogPanel::visibleLineCount() const
{
    int textAreaHeight = panelRect.h - HEADER_HEIGHT - 8;
    return textAreaHeight / LINE_HEIGHT;
}

SDL_Color SimulationLogPanel::colorForMessage(const string& message) const
{
    // Search for keywords to determine color
    if (message.find("ERROR")   != string::npos ||
        message.find("Short")   != string::npos ||
        message.find("circuit") != string::npos ||
        message.find("failed")  != string::npos)
    {
        return textErrorColor;
    }

    if (message.find("WARNING") != string::npos ||
        message.find("Floating")!= string::npos ||
        message.find("Warning") != string::npos)
    {
        return textWarningColor;
    }

    return textNormalColor;
}

void SimulationLogPanel::renderText(SDL_Renderer* renderer,
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

    // Clip text to the panel width so it does not overflow
    int maxW = panelRect.w - (x - panelRect.x) - 10;
    if (texW > maxW) texW = maxW;

    SDL_Rect destRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
}

void SimulationLogPanel::scrollToBottom()
{
    int maxScroll = (int)messages.size() - visibleLineCount();
    if (maxScroll < 0) maxScroll = 0;
    scrollOffset = maxScroll;
}