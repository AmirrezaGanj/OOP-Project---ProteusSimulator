#include "ui/Toolbar.h"

using namespace std;

// ============================================================
// Toolbar.cpp
// ============================================================

// Button layout constants
static const int BTN_W   = 68;
static const int BTN_GAP = 4;
static const int DIV_W   = 2;
static const int DIV_GAP = 8;


// ---- Constructor ----

Toolbar::Toolbar(const SDL_Rect& barRect, TTF_Font* font)
    : barRect(barRect),
      font(font),
      lastAction(ToolbarAction::NONE),
      currentToolMode(ToolMode::SELECT)
{
    bgColor             = { 45, 45, 48,  255 };
    buttonNormalColor   = { 62, 62, 66,  255 };
    buttonHoverColor    = { 80, 80, 85,  255 };
    buttonActiveColor   = { 0,  120, 215, 255 };
    buttonDisabledColor = { 50, 50, 53,  255 };
    textColor           = { 220, 220, 220, 255 };
    textDisabledColor   = { 100, 100, 100, 255 };
    dividerColor        = { 70,  70,  75,  255 };

    buildButtons();
}


// ---- Build button layout ----

void Toolbar::appendButton(const string& label,
                            ToolbarAction action,
                            bool isToggle,
                            int& xCursor)
{
    int y    = barRect.y + 5;
    int btnH = barRect.h - 10;

    Button btn;
    btn.rect      = { xCursor, y, BTN_W, btnH };
    btn.label     = label;
    btn.action    = action;
    btn.isToggle  = isToggle;
    btn.isActive  = false;
    btn.isHovered = false;
    btn.isEnabled = true;
    btn.isDivider = false;

    buttons.push_back(btn);
    xCursor += BTN_W + BTN_GAP;
}

void Toolbar::appendDivider(int& xCursor)
{
    int y    = barRect.y + 5;
    int btnH = barRect.h - 10;

    Button div;
    div.rect      = { xCursor + DIV_GAP / 2, y + 4, DIV_W, btnH - 8 };
    div.isDivider = true;
    div.isEnabled = true;
    div.action    = ToolbarAction::NONE;

    buttons.push_back(div);
    xCursor += DIV_W + DIV_GAP;
}

void Toolbar::buildButtons()
{
    buttons.clear();

    int xCursor = barRect.x + 8;

    // ---- Tool group ----
    appendButton("SELECT", ToolbarAction::SELECT_TOOL, true,  xCursor);
    appendButton("WIRE",   ToolbarAction::WIRE_TOOL,   true,  xCursor);

    appendDivider(xCursor);

    // ---- Simulation group ----
    appendButton("RUN",   ToolbarAction::RUN,   false, xCursor);
    appendButton("PAUSE", ToolbarAction::PAUSE, false, xCursor);
    appendButton("STOP",  ToolbarAction::STOP,  false, xCursor);
    appendButton("STEP",  ToolbarAction::STEP,  false, xCursor);

    appendDivider(xCursor);

    // ---- Edit group ----
    appendButton("UNDO", ToolbarAction::UNDO, false, xCursor);
    appendButton("REDO", ToolbarAction::REDO, false, xCursor);

    appendDivider(xCursor);

    // ---- File group ----
    appendButton("SAVE",   ToolbarAction::SAVE,         false, xCursor);
    appendButton("EXPORT", ToolbarAction::EXPORT_IMAGE, false, xCursor);

    // SELECT is active by default
    for (int i = 0; i < (int)buttons.size(); i++)
    {
        if (buttons[i].action == ToolbarAction::SELECT_TOOL)
        {
            buttons[i].isActive = true;
        }
    }
}


// ---- Event handling ----

bool Toolbar::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        int mx = event.motion.x;
        int my = event.motion.y;

        for (int i = 0; i < (int)buttons.size(); i++)
        {
            if (buttons[i].isDivider)
            {
                continue;
            }

            SDL_Point pt = { mx, my };
            buttons[i].isHovered = (SDL_PointInRect(&pt, &buttons[i].rect) == SDL_TRUE);
        }

        SDL_Point pt  = { mx, my };
        SDL_Rect  bar = barRect;
        return (SDL_PointInRect(&pt, &bar) == SDL_TRUE);
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        int mx  = event.button.x;
        int my  = event.button.y;
        int idx = findButtonAt(mx, my);

        if (idx < 0)
        {
            return false;
        }

        Button& btn = buttons[idx];

        if (!btn.isEnabled || btn.isDivider)
        {
            return true;
        }

        lastAction = btn.action;

        if (btn.isToggle)
        {
            // Deactivate all toggle buttons first
            for (int i = 0; i < (int)buttons.size(); i++)
            {
                if (buttons[i].isToggle)
                {
                    buttons[i].isActive = false;
                }
            }
            btn.isActive = true;

            if (btn.action == ToolbarAction::SELECT_TOOL)
            {
                currentToolMode = ToolMode::SELECT;
            }
            else if (btn.action == ToolbarAction::WIRE_TOOL)
            {
                currentToolMode = ToolMode::WIRE;
            }
        }

        return true;
    }

    return false;
}


// ---- Draw ----

void Toolbar::draw(SDL_Renderer* renderer) const
{
    // Background
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &barRect);

    // Bottom border
    SDL_SetRenderDrawColor(renderer, 25, 25, 27, 255);
    SDL_RenderDrawLine(renderer,
                       barRect.x,
                       barRect.y + barRect.h - 1,
                       barRect.x + barRect.w,
                       barRect.y + barRect.h - 1);

    for (int i = 0; i < (int)buttons.size(); i++)
    {
        drawButton(renderer, buttons[i]);
    }
}

void Toolbar::drawButton(SDL_Renderer* renderer, const Button& btn) const
{
    if (btn.isDivider)
    {
        SDL_SetRenderDrawColor(renderer,
                               dividerColor.r, dividerColor.g,
                               dividerColor.b, dividerColor.a);
        SDL_RenderFillRect(renderer, &btn.rect);
        return;
    }

    SDL_Color bgCol;

    if (!btn.isEnabled)
    {
        bgCol = buttonDisabledColor;
    }
    else if (btn.isActive)
    {
        bgCol = buttonActiveColor;
    }
    else if (btn.isHovered)
    {
        bgCol = buttonHoverColor;
    }
    else
    {
        bgCol = buttonNormalColor;
    }

    SDL_SetRenderDrawColor(renderer, bgCol.r, bgCol.g, bgCol.b, bgCol.a);
    SDL_RenderFillRect(renderer, &btn.rect);

    SDL_SetRenderDrawColor(renderer, 30, 30, 32, 255);
    SDL_RenderDrawRect(renderer, &btn.rect);

    SDL_Color col = btn.isEnabled ? textColor : textDisabledColor;
    renderText(renderer, btn.label,
               btn.rect.x + 6,
               btn.rect.y + (btn.rect.h - 14) / 2,
               col);
}

void Toolbar::renderText(SDL_Renderer* renderer,
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

    SDL_Rect destRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
}

int Toolbar::findButtonAt(int mouseX, int mouseY) const
{
    SDL_Point pt = { mouseX, mouseY };

    for (int i = 0; i < (int)buttons.size(); i++)
    {
        if (buttons[i].isDivider)
        {
            continue;
        }
        if (SDL_PointInRect(&pt, &buttons[i].rect) == SDL_TRUE)
        {
            return i;
        }
    }

    return -1;
}


// ---- Action polling ----

Toolbar::ToolbarAction Toolbar::getLastAction() const
{
    return lastAction;
}

void Toolbar::clearLastAction()
{
    lastAction = ToolbarAction::NONE;
}


// ---- State setters ----

void Toolbar::setToolMode(ToolMode mode)
{
    currentToolMode = mode;

    for (int i = 0; i < (int)buttons.size(); i++)
    {
        if (!buttons[i].isToggle)
        {
            continue;
        }
        if (buttons[i].action == ToolbarAction::SELECT_TOOL)
        {
            buttons[i].isActive = (mode == ToolMode::SELECT);
        }
        else if (buttons[i].action == ToolbarAction::WIRE_TOOL)
        {
            buttons[i].isActive = (mode == ToolMode::WIRE);
        }
    }
}

void Toolbar::setCanUndo(bool canUndo)
{
    for (int i = 0; i < (int)buttons.size(); i++)
    {
        if (buttons[i].action == ToolbarAction::UNDO)
        {
            buttons[i].isEnabled = canUndo;
        }
    }
}

void Toolbar::setCanRedo(bool canRedo)
{
    for (int i = 0; i < (int)buttons.size(); i++)
    {
        if (buttons[i].action == ToolbarAction::REDO)
        {
            buttons[i].isEnabled = canRedo;
        }
    }
}

void Toolbar::setSimRunning(bool running)
{
    for (int i = 0; i < (int)buttons.size(); i++)
    {
        if (buttons[i].action == ToolbarAction::RUN)
        {
            buttons[i].isEnabled = !running;
        }
        if (buttons[i].action == ToolbarAction::PAUSE ||
            buttons[i].action == ToolbarAction::STOP  ||
            buttons[i].action == ToolbarAction::STEP)
        {
            buttons[i].isEnabled = running;
        }
    }
}

void Toolbar::setSimPaused(bool paused)
{
    for (int i = 0; i < (int)buttons.size(); i++)
    {
        if (buttons[i].action == ToolbarAction::STEP)
        {
            buttons[i].isEnabled = paused;
        }
    }
}

Toolbar::ToolMode Toolbar::getCurrentToolMode() const
{
    return currentToolMode;
}