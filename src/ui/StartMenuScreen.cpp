#include "ui/StartMenuScreen.h"
#include <sstream>

using namespace std;

// ============================================================
// StartMenuScreen.cpp
// ============================================================

const int StartMenuScreen::BUTTON_W      = 250;
const int StartMenuScreen::BUTTON_H      = 54;
const int StartMenuScreen::BUTTON_GAP    = 16;
const int StartMenuScreen::RECENT_LINE_H = 34;
const int StartMenuScreen::FIELD_H       = 32;


// ---- Constructor ----

StartMenuScreen::StartMenuScreen(int windowW,
                                  int windowH,
                                  ProjectManager& projectManager,
                                  TTF_Font* titleFont,
                                  TTF_Font* font)
    : windowW(windowW),
      windowH(windowH),
      projectManager(projectManager),
      titleFont(titleFont),
      font(font),
      state(ScreenState::MAIN_MENU),
      action(MenuAction::NONE),
      hoveredRecentIndex(-1),
      recentScrollOffset(0),
      selectedProjectPath(""),
      canvasWidth(1188.0f),
      canvasHeight(840.0f),
      customWidthText("1188"),
      customHeightText("840"),
      customWidthFocused(false),
      customHeightFocused(false)
{
    bgColor                 = { 30,  32,  38,  255 };
    panelColor              = { 40,  42,  50,  255 };
    borderColor             = { 62,  64,  74,  255 };
    titleColor              = { 235, 235, 240, 255 };
    subtitleColor           = { 130, 134, 146, 255 };
    textColor               = { 214, 216, 222, 255 };
    textDimColor            = { 128, 132, 144, 255 };
    buttonColor             = { 56,  59,  70,  255 };
    buttonHoverColor        = { 74,  78,  92,  255 };
    buttonPrimaryColor      = { 0,   110, 200, 255 };
    buttonPrimaryHoverColor = { 0,   135, 235, 255 };
    buttonActiveColor       = { 0,   90,  165, 255 };
    fieldBgColor            = { 24,  25,  30,  255 };
    fieldBorderColor        = { 68,  70,  80,  255 };
    fieldActiveBorderColor  = { 0,   130, 220, 255 };

    computeAreas();
    buildMainButtons();
    buildConfigButtons();

    refresh();
}


// ---- Setup ----

void StartMenuScreen::computeAreas()
{
    // Recent projects panel on the right half of the main menu
    recentPanelRect.x = windowW / 2 + 20;
    recentPanelRect.y = 200;
    recentPanelRect.w = windowW / 2 - 80;
    recentPanelRect.h = 380;

    // Open-project list occupies a wide central area
    openListRect.x = 60;
    openListRect.y = 180;
    openListRect.w = windowW - 120;
    openListRect.h = windowH - 300;
}


void StartMenuScreen::buildMainButtons()
{
    mainButtons.clear();

    int x = 60;
    int y = 210;

    Button newBtn;
    newBtn.rect      = { x, y, BUTTON_W, BUTTON_H };
    newBtn.label     = "New Project";
    newBtn.id        = ButtonId::NEW_PROJECT;
    newBtn.isPrimary = true;
    mainButtons.push_back(newBtn);
    y += BUTTON_H + BUTTON_GAP;

    Button openBtn;
    openBtn.rect  = { x, y, BUTTON_W, BUTTON_H };
    openBtn.label = "Open Project";
    openBtn.id    = ButtonId::OPEN_PROJECT;
    mainButtons.push_back(openBtn);
    y += BUTTON_H + BUTTON_GAP;

    Button exitBtn;
    exitBtn.rect  = { x, y, BUTTON_W, BUTTON_H };
    exitBtn.label = "Exit";
    exitBtn.id    = ButtonId::EXIT;
    mainButtons.push_back(exitBtn);
}


void StartMenuScreen::buildConfigButtons()
{
    configButtons.clear();

    int x = 60;
    int y = 190;
    int presetW = 260;
    int presetH = 44;

    Button a4p;
    a4p.rect  = { x, y, presetW, presetH };
    a4p.label = "A4 Portrait   (840 x 1188)";
    a4p.id    = ButtonId::PRESET_A4_PORTRAIT;
    configButtons.push_back(a4p);
    y += presetH + 10;

    Button a4l;
    a4l.rect     = { x, y, presetW, presetH };
    a4l.label    = "A4 Landscape  (1188 x 840)";
    a4l.id       = ButtonId::PRESET_A4_LANDSCAPE;
    a4l.isActive = true;                 // default selection
    configButtons.push_back(a4l);
    y += presetH + 10;

    Button a3l;
    a3l.rect  = { x, y, presetW, presetH };
    a3l.label = "A3 Landscape  (1680 x 1188)";
    a3l.id    = ButtonId::PRESET_A3_LANDSCAPE;
    configButtons.push_back(a3l);
    y += presetH + 10;

    Button a2l;
    a2l.rect  = { x, y, presetW, presetH };
    a2l.label = "A2 Landscape  (2376 x 1680)";
    a2l.id    = ButtonId::PRESET_A2_LANDSCAPE;
    configButtons.push_back(a2l);
    y += presetH + 30;

    // Custom size input fields sit under the presets
    customWidthRect  = { x + 90,  y, 110, FIELD_H };
    customHeightRect = { x + 90,  y + FIELD_H + 12, 110, FIELD_H };

    // Create and Back buttons at the bottom
    int actionY = y + (FIELD_H + 12) * 2 + 30;

    Button createBtn;
    createBtn.rect      = { x, actionY, 140, BUTTON_H };
    createBtn.label     = "Create";
    createBtn.id        = ButtonId::CREATE;
    createBtn.isPrimary = true;
    configButtons.push_back(createBtn);

    Button backBtn;
    backBtn.rect  = { x + 140 + BUTTON_GAP, actionY, 140, BUTTON_H };
    backBtn.label = "Back";
    backBtn.id    = ButtonId::BACK;
    configButtons.push_back(backBtn);
}


void StartMenuScreen::refresh()
{
    projectManager.loadRecentProjectsList();
    recentProjects     = projectManager.getRecentProjects();
    recentScrollOffset = 0;
    hoveredRecentIndex = -1;
}


// ---- Preset helpers ----

void StartMenuScreen::applyPreset(ButtonId presetId)
{
    if (presetId == ButtonId::PRESET_A4_PORTRAIT)
    {
        canvasWidth  = 840.0f;
        canvasHeight = 1188.0f;
    }
    else if (presetId == ButtonId::PRESET_A4_LANDSCAPE)
    {
        canvasWidth  = 1188.0f;
        canvasHeight = 840.0f;
    }
    else if (presetId == ButtonId::PRESET_A3_LANDSCAPE)
    {
        canvasWidth  = 1680.0f;
        canvasHeight = 1188.0f;
    }
    else if (presetId == ButtonId::PRESET_A2_LANDSCAPE)
    {
        canvasWidth  = 2376.0f;
        canvasHeight = 1680.0f;
    }
    else
    {
        return;
    }

    // Mirror the preset into the custom fields so they stay in sync
    customWidthText  = to_string((int)canvasWidth);
    customHeightText = to_string((int)canvasHeight);

    // Highlight only the chosen preset
    for (int i = 0; i < (int)configButtons.size(); i++)
    {
        if (configButtons[i].id == ButtonId::PRESET_A4_PORTRAIT  ||
            configButtons[i].id == ButtonId::PRESET_A4_LANDSCAPE ||
            configButtons[i].id == ButtonId::PRESET_A3_LANDSCAPE ||
            configButtons[i].id == ButtonId::PRESET_A2_LANDSCAPE)
        {
            configButtons[i].isActive = (configButtons[i].id == presetId);
        }
    }
}


bool StartMenuScreen::applyCustomSize()
{
    float w = 0.0f;
    float h = 0.0f;

    // stof throws on invalid text, so guard it
    try
    {
        w = stof(customWidthText);
        h = stof(customHeightText);
    }
    catch (...)
    {
        return false;
    }

    if (w < 200.0f || h < 200.0f)
    {
        return false;
    }

    if (w > 10000.0f || h > 10000.0f)
    {
        return false;
    }

    canvasWidth  = w;
    canvasHeight = h;
    return true;
}


// ---- Event handling ----

bool StartMenuScreen::handleEvent(const SDL_Event& event)
{
    if (state == ScreenState::MAIN_MENU)
    {
        return handleMainMenuEvent(event);
    }

    if (state == ScreenState::NEW_PROJECT_CONFIG)
    {
        return handleConfigEvent(event);
    }

    return handleOpenListEvent(event);
}


bool StartMenuScreen::handleMainMenuEvent(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        int mx = event.motion.x;
        int my = event.motion.y;

        for (int i = 0; i < (int)mainButtons.size(); i++)
        {
            SDL_Point pt = { mx, my };
            mainButtons[i].isHovered =
                (SDL_PointInRect(&pt, &mainButtons[i].rect) == SDL_TRUE);
        }

        hoveredRecentIndex = recentIndexAt(recentPanelRect,
                                           recentScrollOffset, mx, my);
        return true;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        int mx = event.button.x;
        int my = event.button.y;

        int idx = buttonIndexAt(mainButtons, mx, my);

        if (idx >= 0)
        {
            ButtonId id = mainButtons[idx].id;

            if (id == ButtonId::NEW_PROJECT)
            {
                state = ScreenState::NEW_PROJECT_CONFIG;
                return true;
            }

            if (id == ButtonId::OPEN_PROJECT)
            {
                refresh();
                state = ScreenState::OPEN_PROJECT_LIST;
                return true;
            }

            if (id == ButtonId::EXIT)
            {
                action = MenuAction::EXIT;
                return true;
            }
        }

        // Clicking an entry in the recent list opens it directly
        int recentIdx = recentIndexAt(recentPanelRect,
                                      recentScrollOffset, mx, my);

        if (recentIdx >= 0 && recentIdx < (int)recentProjects.size())
        {
            selectedProjectPath = recentProjects[recentIdx];
            action              = MenuAction::OPEN_PROJECT;
            return true;
        }

        return true;
    }

    if (event.type == SDL_MOUSEWHEEL)
    {
        int mx = 0;
        int my = 0;
        SDL_GetMouseState(&mx, &my);

        SDL_Point pt = { mx, my };
        if (SDL_PointInRect(&pt, &recentPanelRect) == SDL_TRUE)
        {
            recentScrollOffset -= event.wheel.y;

            int maxScroll = (int)recentProjects.size() -
                            visibleRecentCount(recentPanelRect);
            if (maxScroll < 0)                maxScroll = 0;
            if (recentScrollOffset < 0)         recentScrollOffset = 0;
            if (recentScrollOffset > maxScroll) recentScrollOffset = maxScroll;
        }

        return true;
    }

    return false;
}


bool StartMenuScreen::handleConfigEvent(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        int mx = event.motion.x;
        int my = event.motion.y;

        for (int i = 0; i < (int)configButtons.size(); i++)
        {
            SDL_Point pt = { mx, my };
            configButtons[i].isHovered =
                (SDL_PointInRect(&pt, &configButtons[i].rect) == SDL_TRUE);
        }

        return true;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        int mx = event.button.x;
        int my = event.button.y;
        SDL_Point pt = { mx, my };

        // Text field focus
        if (SDL_PointInRect(&pt, &customWidthRect) == SDL_TRUE)
        {
            customWidthFocused  = true;
            customHeightFocused = false;
            SDL_StartTextInput();
            return true;
        }

        if (SDL_PointInRect(&pt, &customHeightRect) == SDL_TRUE)
        {
            customWidthFocused  = false;
            customHeightFocused = true;
            SDL_StartTextInput();
            return true;
        }

        customWidthFocused  = false;
        customHeightFocused = false;

        int idx = buttonIndexAt(configButtons, mx, my);

        if (idx >= 0)
        {
            ButtonId id = configButtons[idx].id;

            if (id == ButtonId::BACK)
            {
                state = ScreenState::MAIN_MENU;
                return true;
            }

            if (id == ButtonId::CREATE)
            {
                // Custom text always wins — presets write into it too
                if (applyCustomSize())
                {
                    action = MenuAction::NEW_PROJECT;
                }
                return true;
            }

            applyPreset(id);
            return true;
        }

        return true;
    }

    if (event.type == SDL_TEXTINPUT)
    {
        if (customWidthFocused)
        {
            customWidthText += event.text.text;
            return true;
        }
        if (customHeightFocused)
        {
            customHeightText += event.text.text;
            return true;
        }
    }

    if (event.type == SDL_KEYDOWN)
    {
        SDL_Keycode key = event.key.keysym.sym;

        if (key == SDLK_ESCAPE)
        {
            state = ScreenState::MAIN_MENU;
            return true;
        }

        if (key == SDLK_BACKSPACE)
        {
            if (customWidthFocused && !customWidthText.empty())
            {
                customWidthText.pop_back();
                return true;
            }
            if (customHeightFocused && !customHeightText.empty())
            {
                customHeightText.pop_back();
                return true;
            }
        }

        if (key == SDLK_TAB)
        {
            if (customWidthFocused)
            {
                customWidthFocused  = false;
                customHeightFocused = true;
            }
            else
            {
                customWidthFocused  = true;
                customHeightFocused = false;
            }
            return true;
        }

        if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
        {
            if (applyCustomSize())
            {
                action = MenuAction::NEW_PROJECT;
            }
            return true;
        }
    }

    return false;
}


bool StartMenuScreen::handleOpenListEvent(const SDL_Event& event)
{
    if (event.type == SDL_MOUSEMOTION)
    {
        int mx = event.motion.x;
        int my = event.motion.y;

        hoveredRecentIndex = recentIndexAt(openListRect,
                                           recentScrollOffset, mx, my);

        for (int i = 0; i < (int)configButtons.size(); i++)
        {
            if (configButtons[i].id != ButtonId::BACK)
            {
                continue;
            }
            SDL_Point pt = { mx, my };
            configButtons[i].isHovered =
                (SDL_PointInRect(&pt, &configButtons[i].rect) == SDL_TRUE);
        }

        return true;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        int mx = event.button.x;
        int my = event.button.y;

        int recentIdx = recentIndexAt(openListRect,
                                      recentScrollOffset, mx, my);

        if (recentIdx >= 0 && recentIdx < (int)recentProjects.size())
        {
            selectedProjectPath = recentProjects[recentIdx];
            action              = MenuAction::OPEN_PROJECT;
            return true;
        }

        // Back button (reused from the config button list)
        for (int i = 0; i < (int)configButtons.size(); i++)
        {
            if (configButtons[i].id != ButtonId::BACK)
            {
                continue;
            }

            SDL_Point pt = { mx, my };
            if (SDL_PointInRect(&pt, &configButtons[i].rect) == SDL_TRUE)
            {
                state = ScreenState::MAIN_MENU;
                return true;
            }
        }

        return true;
    }

    if (event.type == SDL_MOUSEWHEEL)
    {
        recentScrollOffset -= event.wheel.y;

        int maxScroll = (int)recentProjects.size() -
                        visibleRecentCount(openListRect);
        if (maxScroll < 0)                maxScroll = 0;
        if (recentScrollOffset < 0)         recentScrollOffset = 0;
        if (recentScrollOffset > maxScroll) recentScrollOffset = maxScroll;

        return true;
    }

    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.sym == SDLK_ESCAPE)
    {
        state = ScreenState::MAIN_MENU;
        return true;
    }

    return false;
}


// ---- Draw ----

void StartMenuScreen::draw(SDL_Renderer* renderer) const
{
    // Full-screen background
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);

    SDL_Rect full = { 0, 0, windowW, windowH };
    SDL_RenderFillRect(renderer, &full);

    if (state == ScreenState::MAIN_MENU)
    {
        drawMainMenu(renderer);
    }
    else if (state == ScreenState::NEW_PROJECT_CONFIG)
    {
        drawConfig(renderer);
    }
    else
    {
        drawOpenList(renderer);
    }
}


void StartMenuScreen::drawMainMenu(SDL_Renderer* renderer) const
{
    // ---- Title ----
    renderText(renderer, "Proteus Simulator", 60, 70, titleColor, true);
    renderText(renderer, "Circuit design and simulation environment",
               60, 125, subtitleColor);

    // Accent underline beneath the title
    SDL_SetRenderDrawColor(renderer,
                           buttonPrimaryColor.r, buttonPrimaryColor.g,
                           buttonPrimaryColor.b, 255);

    SDL_Rect underline = { 60, 160, 120, 3 };
    SDL_RenderFillRect(renderer, &underline);

    // ---- Buttons ----
    for (int i = 0; i < (int)mainButtons.size(); i++)
    {
        drawButton(renderer, mainButtons[i]);
    }

    // ---- Recent projects panel ----
    SDL_SetRenderDrawColor(renderer,
                           panelColor.r, panelColor.g,
                           panelColor.b, panelColor.a);
    SDL_RenderFillRect(renderer, &recentPanelRect);

    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &recentPanelRect);

    renderText(renderer, "Recent Projects",
               recentPanelRect.x + 14,
               recentPanelRect.y + 12,
               textDimColor);

    SDL_RenderDrawLine(renderer,
                       recentPanelRect.x + 1,
                       recentPanelRect.y + 40,
                       recentPanelRect.x + recentPanelRect.w - 1,
                       recentPanelRect.y + 40);

    if (recentProjects.empty())
    {
        renderText(renderer, "No recent projects yet.",
                   recentPanelRect.x + 14,
                   recentPanelRect.y + 58,
                   textDimColor);
        return;
    }

    int visCount = visibleRecentCount(recentPanelRect);
    int endIdx   = recentScrollOffset + visCount;

    if (endIdx > (int)recentProjects.size())
    {
        endIdx = (int)recentProjects.size();
    }

    int y = recentPanelRect.y + 50;

    for (int i = recentScrollOffset; i < endIdx; i++)
    {
        SDL_Rect row = { recentPanelRect.x + 1,
                          y,
                          recentPanelRect.w - 2,
                          RECENT_LINE_H };

        if (hoveredRecentIndex == i)
        {
            SDL_SetRenderDrawColor(renderer,
                                   buttonHoverColor.r, buttonHoverColor.g,
                                   buttonHoverColor.b, buttonHoverColor.a);
            SDL_RenderFillRect(renderer, &row);
        }

        renderText(renderer, prettyProjectName(recentProjects[i]),
                   recentPanelRect.x + 14,
                   y + (RECENT_LINE_H - 16) / 2,
                   textColor);

        y += RECENT_LINE_H;
    }
}


void StartMenuScreen::drawConfig(SDL_Renderer* renderer) const
{
    renderText(renderer, "New Project", 60, 70, titleColor, true);
    renderText(renderer, "Choose the canvas size for your circuit",
               60, 125, subtitleColor);

    SDL_SetRenderDrawColor(renderer,
                           buttonPrimaryColor.r, buttonPrimaryColor.g,
                           buttonPrimaryColor.b, 255);
    SDL_Rect underline = { 60, 160, 120, 3 };
    SDL_RenderFillRect(renderer, &underline);

    // Preset and action buttons
    for (int i = 0; i < (int)configButtons.size(); i++)
    {
        drawButton(renderer, configButtons[i]);
    }

    // ---- Custom size fields ----
    renderText(renderer, "Width:",
               customWidthRect.x - 80,
               customWidthRect.y + (FIELD_H - 16) / 2,
               textDimColor);

    drawTextField(renderer, customWidthRect,
                  customWidthText, customWidthFocused);

    renderText(renderer, "Height:",
               customHeightRect.x - 80,
               customHeightRect.y + (FIELD_H - 16) / 2,
               textDimColor);

    drawTextField(renderer, customHeightRect,
                  customHeightText, customHeightFocused);

    renderText(renderer, "Values are in canvas units (min 200, max 10000).",
               customWidthRect.x + 130,
               customWidthRect.y + (FIELD_H - 16) / 2,
               textDimColor);
}


void StartMenuScreen::drawOpenList(SDL_Renderer* renderer) const
{
    renderText(renderer, "Open Project", 60, 70, titleColor, true);
    renderText(renderer, "Select a saved project to load",
               60, 125, subtitleColor);

    SDL_SetRenderDrawColor(renderer,
                           buttonPrimaryColor.r, buttonPrimaryColor.g,
                           buttonPrimaryColor.b, 255);
    SDL_Rect underline = { 60, 160, 120, 3 };
    SDL_RenderFillRect(renderer, &underline);

    // List panel
    SDL_SetRenderDrawColor(renderer,
                           panelColor.r, panelColor.g,
                           panelColor.b, panelColor.a);
    SDL_RenderFillRect(renderer, &openListRect);

    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &openListRect);

    if (recentProjects.empty())
    {
        renderText(renderer, "No saved projects found.",
                   openListRect.x + 16,
                   openListRect.y + 20,
                   textDimColor);

        renderText(renderer,
                   "Create a new project and save it to see it listed here.",
                   openListRect.x + 16,
                   openListRect.y + 48,
                   textDimColor);
    }
    else
    {
        int visCount = visibleRecentCount(openListRect);
        int endIdx   = recentScrollOffset + visCount;

        if (endIdx > (int)recentProjects.size())
        {
            endIdx = (int)recentProjects.size();
        }

        int y = openListRect.y + 10;

        for (int i = recentScrollOffset; i < endIdx; i++)
        {
            SDL_Rect row = { openListRect.x + 1,
                              y,
                              openListRect.w - 2,
                              RECENT_LINE_H };

            if (hoveredRecentIndex == i)
            {
                SDL_SetRenderDrawColor(renderer,
                                       buttonHoverColor.r, buttonHoverColor.g,
                                       buttonHoverColor.b, buttonHoverColor.a);
                SDL_RenderFillRect(renderer, &row);
            }

            renderText(renderer, prettyProjectName(recentProjects[i]),
                       openListRect.x + 16,
                       y + (RECENT_LINE_H - 16) / 2,
                       textColor);

            // Full path shown dimly on the right
            renderText(renderer, recentProjects[i],
                       openListRect.x + openListRect.w / 2,
                       y + (RECENT_LINE_H - 16) / 2,
                       textDimColor);

            y += RECENT_LINE_H;
        }
    }

    // Back button — reuse the one built for the config screen,
    // repositioned under the list.
    for (int i = 0; i < (int)configButtons.size(); i++)
    {
        if (configButtons[i].id != ButtonId::BACK)
        {
            continue;
        }

        Button back = configButtons[i];
        back.rect.x = 60;
        back.rect.y = openListRect.y + openListRect.h + 20;

        drawButton(renderer, back);
    }
}


void StartMenuScreen::drawButton(SDL_Renderer* renderer,
                                  const Button& btn) const
{
    SDL_Color fill;

    if (btn.isActive)
    {
        fill = buttonActiveColor;
    }
    else if (btn.isPrimary)
    {
        fill = btn.isHovered ? buttonPrimaryHoverColor : buttonPrimaryColor;
    }
    else
    {
        fill = btn.isHovered ? buttonHoverColor : buttonColor;
    }

    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer, &btn.rect);

    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &btn.rect);

    renderText(renderer, btn.label,
               btn.rect.x + 16,
               btn.rect.y + (btn.rect.h - 16) / 2,
               textColor);
}


void StartMenuScreen::drawTextField(SDL_Renderer* renderer,
                                     const SDL_Rect& rect,
                                     const string& value,
                                     bool focused) const
{
    SDL_SetRenderDrawColor(renderer,
                           fieldBgColor.r, fieldBgColor.g,
                           fieldBgColor.b, fieldBgColor.a);
    SDL_RenderFillRect(renderer, &rect);

    SDL_Color borderCol = focused ? fieldActiveBorderColor : fieldBorderColor;
    SDL_SetRenderDrawColor(renderer,
                           borderCol.r, borderCol.g,
                           borderCol.b, borderCol.a);
    SDL_RenderDrawRect(renderer, &rect);

    renderText(renderer, value,
               rect.x + 8,
               rect.y + (rect.h - 16) / 2,
               textColor);

    if (focused)
    {
        int cursorX = rect.x + 8 + (int)value.size() * 8;

        SDL_SetRenderDrawColor(renderer, 210, 210, 210, 255);
        SDL_RenderDrawLine(renderer,
                           cursorX, rect.y + 5,
                           cursorX, rect.y + rect.h - 5);
    }
}


void StartMenuScreen::renderText(SDL_Renderer* renderer,
                                  const string& text,
                                  int x, int y,
                                  SDL_Color color,
                                  bool useTitleFont) const
{
    TTF_Font* chosen = useTitleFont ? titleFont : font;

    if (chosen == nullptr || text.empty())
    {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(chosen, text.c_str(), color);
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


// ---- Helpers ----

string StartMenuScreen::prettyProjectName(const string& path) const
{
    size_t lastSlash = path.find_last_of("/\\");

    string filename = (lastSlash == string::npos)
                       ? path
                       : path.substr(lastSlash + 1);

    size_t dotPos = filename.find_last_of('.');

    if (dotPos == string::npos)
    {
        return filename;
    }

    return filename.substr(0, dotPos);
}


int StartMenuScreen::buttonIndexAt(const vector<Button>& list,
                                    int mouseX, int mouseY) const
{
    SDL_Point pt = { mouseX, mouseY };

    for (int i = 0; i < (int)list.size(); i++)
    {
        if (SDL_PointInRect(&pt, &list[i].rect) == SDL_TRUE)
        {
            return i;
        }
    }

    return -1;
}


int StartMenuScreen::recentIndexAt(const SDL_Rect& area,
                                    int scrollOffset,
                                    int mouseX, int mouseY) const
{
    if (mouseX < area.x || mouseX >= area.x + area.w)
    {
        return -1;
    }

    // The main-menu panel has a header, the open list does not
    int startY = (area.x == recentPanelRect.x &&
                  area.y == recentPanelRect.y)
                  ? area.y + 50
                  : area.y + 10;

    int visCount = visibleRecentCount(area);
    int endIdx   = scrollOffset + visCount;

    if (endIdx > (int)recentProjects.size())
    {
        endIdx = (int)recentProjects.size();
    }

    int y = startY;

    for (int i = scrollOffset; i < endIdx; i++)
    {
        if (mouseY >= y && mouseY < y + RECENT_LINE_H)
        {
            return i;
        }
        y += RECENT_LINE_H;
    }

    return -1;
}


int StartMenuScreen::visibleRecentCount(const SDL_Rect& area) const
{
    int usable = area.h - 60;

    if (usable < RECENT_LINE_H)
    {
        return 1;
    }

    return usable / RECENT_LINE_H;
}


// ---- Action polling ----

StartMenuScreen::MenuAction StartMenuScreen::getAction() const
{
    return action;
}

void StartMenuScreen::clearAction()
{
    action = MenuAction::NONE;
}

float StartMenuScreen::getCanvasWidth() const
{
    return canvasWidth;
}

float StartMenuScreen::getCanvasHeight() const
{
    return canvasHeight;
}

string StartMenuScreen::getSelectedProjectPath() const
{
    return selectedProjectPath;
}