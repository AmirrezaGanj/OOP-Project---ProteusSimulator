#include "ui/PropertiesDialog.h"

// Component type headers needed for dynamic_cast in applyToComponent()
#include "components/passive/Resistor.h"
#include "components/passive/Capacitor.h"
#include "components/passive/Inductor.h"
#include "components/sources/DCVoltageSource.h"
#include "components/sources/Battery.h"
#include "components/sources/ClockGenerator.h"
#include "components/interactive/LED.h"
#include "components/gates/ANDGate.h"
#include "components/gates/ORGate.h"
#include "components/gates/NOTGate.h"
#include "components/gates/NANDGate.h"
#include "components/gates/XORGate.h"
#include "components/gates/DFlipFlop.h"
#include "components/gates/LogicGate.h"

#include <sstream>
#include <iomanip>

using namespace std;

// ============================================================
// PropertiesDialog.cpp
// ============================================================

//const int PropertiesDialog::DIALOG_W      = 320;
//const int PropertiesDialog::HEADER_H      = 36;
//const int PropertiesDialog::FIELD_H       = 30;
//const int PropertiesDialog::FIELD_PADDING = 8;
//const int PropertiesDialog::BUTTON_H      = 32;
//const int PropertiesDialog::BUTTON_W      = 90;
//const int PropertiesDialog::FOOTER_H      = 48;


// ---- Constructor ----

PropertiesDialog::PropertiesDialog(int windowW, int windowH, TTF_Font* font)
    : windowW(windowW),
      windowH(windowH),
      font(font),
      targetComponent(nullptr),
      dialogOpen(false),
      accepted(false),
      okHovered(false),
      cancelHovered(false)
{
    bgColor                = { 45,  45,  50,  255 };
    headerColor            = { 30,  30,  35,  255 };
    borderColor            = { 70,  70,  78,  255 };
    fieldBgColor           = { 28,  28,  32,  255 };
    fieldActiveBorderColor = { 0,   120, 215, 255 };
    fieldNormalBorderColor = { 65,  65,  72,  255 };
    textColor              = { 215, 215, 215, 255 };
    textDimColor           = { 120, 120, 128, 255 };
    overlayColor           = { 0,   0,   0,   160 };
    okBtnColor             = { 0,   100, 180, 255 };
    okBtnHoverColor        = { 0,   130, 220, 255 };
    cancelBtnColor         = { 65,  65,  72,  255 };
    cancelBtnHoverColor    = { 90,  90,  98,  255 };

    dialogRect = { 0, 0, 0, 0 };
    okRect     = { 0, 0, 0, 0 };
    cancelRect = { 0, 0, 0, 0 };
}


// ---- Lifecycle ----

void PropertiesDialog::open(Component* component)
{
    if (component == nullptr)
    {
        return;
    }

    targetComponent = component;
    accepted        = false;
    dialogOpen      = true;

    buildFields(component);
    computeLayout();

    // Focus the first field by default
    if (!fields.empty())
    {
        fields[0].focused = true;
        SDL_StartTextInput();
    }
}

void PropertiesDialog::close()
{
    dialogOpen      = false;
    targetComponent = nullptr;
    fields.clear();
    SDL_StopTextInput();
}

bool PropertiesDialog::isOpen() const
{
    return dialogOpen;
}


// ---- Event handling ----

bool PropertiesDialog::handleEvent(const SDL_Event& event)
{
    if (!dialogOpen)
    {
        return false;
    }

    // Always consume all events while dialog is open
    if (event.type == SDL_MOUSEMOTION)
    {
        SDL_Point pt = { event.motion.x, event.motion.y };
        okHovered     = (SDL_PointInRect(&pt, &okRect)     == SDL_TRUE);
        cancelHovered = (SDL_PointInRect(&pt, &cancelRect) == SDL_TRUE);
        return true;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        SDL_Point pt = { event.button.x, event.button.y };

        // OK button
        if (SDL_PointInRect(&pt, &okRect) == SDL_TRUE)
        {
            if (applyToComponent())
            {
                accepted = true;
                close();
            }
            return true;
        }

        // Cancel button
        if (SDL_PointInRect(&pt, &cancelRect) == SDL_TRUE)
        {
            close();
            return true;
        }

        // Click outside dialog = cancel
        if (SDL_PointInRect(&pt, &dialogRect) != SDL_TRUE)
        {
            close();
            return true;
        }

        // Field click — change focus
        int idx = fieldIndexAt(pt.x, pt.y);
        if (idx >= 0)
        {
            for (int i = 0; i < (int)fields.size(); i++)
            {
                fields[i].focused = (i == idx);
            }
        }

        return true;
    }

    // Text input
    if (event.type == SDL_TEXTINPUT)
    {
        for (int i = 0; i < (int)fields.size(); i++)
        {
            if (fields[i].focused)
            {
                fields[i].value += event.text.text;
                break;
            }
        }
        return true;
    }

    // Keyboard shortcuts
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            close();
            return true;
        }

        if (event.key.keysym.sym == SDLK_RETURN ||
            event.key.keysym.sym == SDLK_KP_ENTER)
        {
            if (applyToComponent())
            {
                accepted = true;
                close();
            }
            return true;
        }

        if (event.key.keysym.sym == SDLK_BACKSPACE)
        {
            for (int i = 0; i < (int)fields.size(); i++)
            {
                if (fields[i].focused && !fields[i].value.empty())
                {
                    fields[i].value.pop_back();
                    break;
                }
            }
            return true;
        }

        // Tab — cycle focus between fields
        if (event.key.keysym.sym == SDLK_TAB)
        {
            int currentFocused = -1;
            for (int i = 0; i < (int)fields.size(); i++)
            {
                if (fields[i].focused)
                {
                    currentFocused = i;
                    break;
                }
            }

            if (currentFocused >= 0)
            {
                fields[currentFocused].focused = false;
                int next = (currentFocused + 1) % (int)fields.size();
                fields[next].focused = true;
            }

            return true;
        }
    }

    return true;    // consume all events while open
}


// ---- Draw ----

void PropertiesDialog::draw(SDL_Renderer* renderer) const
{
    if (!dialogOpen)
    {
        return;
    }

    // Semi-transparent overlay behind the dialog
    drawOverlay(renderer);

    // Dialog background
    SDL_SetRenderDrawColor(renderer,
                           bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &dialogRect);

    // Dialog border
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &dialogRect);

    // ---- Header ----
    SDL_Rect headerRect = { dialogRect.x, dialogRect.y,
                             dialogRect.w, HEADER_H };

    SDL_SetRenderDrawColor(renderer,
                           headerColor.r, headerColor.g,
                           headerColor.b, headerColor.a);
    SDL_RenderFillRect(renderer, &headerRect);

    string title = "Properties";
    if (targetComponent != nullptr)
    {
        title = targetComponent->getType() + " Properties";
    }

    renderText(renderer, title,
               dialogRect.x + 12,
               dialogRect.y + (HEADER_H - 14) / 2,
               textColor);

    // Header bottom border
    SDL_SetRenderDrawColor(renderer,
                           borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);
    SDL_RenderDrawLine(renderer,
                       dialogRect.x,
                       dialogRect.y + HEADER_H,
                       dialogRect.x + dialogRect.w,
                       dialogRect.y + HEADER_H);

    // ---- Fields ----
    int inputX = fieldInputX();
    int inputW = fieldInputW();

    for (int i = 0; i < (int)fields.size(); i++)
    {
        drawField(renderer, fields[i], inputX, fieldY(i), inputW);
    }

    // ---- Buttons ----

    // OK button
    SDL_Color okCol = okHovered ? okBtnHoverColor : okBtnColor;
    SDL_SetRenderDrawColor(renderer, okCol.r, okCol.g, okCol.b, okCol.a);
    SDL_RenderFillRect(renderer, &okRect);
    SDL_SetRenderDrawColor(renderer, 0, 80, 150, 255);
    SDL_RenderDrawRect(renderer, &okRect);
    renderText(renderer, "OK",
               okRect.x + (okRect.w - 20) / 2,
               okRect.y + (okRect.h - 14) / 2,
               textColor);

    // Cancel button
    SDL_Color cancelCol = cancelHovered ? cancelBtnHoverColor : cancelBtnColor;
    SDL_SetRenderDrawColor(renderer,
                           cancelCol.r, cancelCol.g,
                           cancelCol.b, cancelCol.a);
    SDL_RenderFillRect(renderer, &cancelRect);
    SDL_SetRenderDrawColor(renderer, 50, 50, 58, 255);
    SDL_RenderDrawRect(renderer, &cancelRect);
    renderText(renderer, "Cancel",
               cancelRect.x + (cancelRect.w - 48) / 2,
               cancelRect.y + (cancelRect.h - 14) / 2,
               textColor);
}


// ---- Result ----

bool PropertiesDialog::wasAccepted() const
{
    return accepted;
}

void PropertiesDialog::clearAccepted()
{
    accepted = false;
}


// ---- Private: Setup ----

void PropertiesDialog::buildFields(Component* component)
{
    fields.clear();

    string type = component->getType();

    // All components have a label
    fields.push_back(Field("Label", component->getLabel(), FieldType::TEXT));

    // Type-specific fields
    if (type == "RESISTOR")
    {
        Resistor* r = dynamic_cast<Resistor*>(component);
        if (r != nullptr)
        {
            ostringstream oss;
            oss << fixed << setprecision(1) << r->getResistance();
            fields.push_back(Field("Resistance (Ohms)", oss.str(), FieldType::NUMBER));
        }
    }
    else if (type == "CAPACITOR")
    {
        Capacitor* c = dynamic_cast<Capacitor*>(component);
        if (c != nullptr)
        {
            ostringstream oss;
            oss << fixed << setprecision(6) << c->getCapacitance();
            fields.push_back(Field("Capacitance (F)", oss.str(), FieldType::NUMBER));
        }
    }
    else if (type == "INDUCTOR")
    {
        Inductor* l = dynamic_cast<Inductor*>(component);
        if (l != nullptr)
        {
            ostringstream oss;
            oss << fixed << setprecision(4) << l->getInductance();
            fields.push_back(Field("Inductance (H)", oss.str(), FieldType::NUMBER));
        }
    }
    else if (type == "DCVOLTAGE")
    {
        DCVoltageSource* v = dynamic_cast<DCVoltageSource*>(component);
        if (v != nullptr)
        {
            ostringstream oss;
            oss << fixed << setprecision(2) << v->getVoltage();
            fields.push_back(Field("Voltage (V)", oss.str(), FieldType::NUMBER));
        }
    }
    else if (type == "BATTERY")
    {
        Battery* b = dynamic_cast<Battery*>(component);
        if (b != nullptr)
        {
            ostringstream emf;
            emf << fixed << setprecision(2) << b->getEmf();
            fields.push_back(Field("EMF (V)", emf.str(), FieldType::NUMBER));

            ostringstream intR;
            intR << fixed << setprecision(2) << b->getInternalResistance();
            fields.push_back(Field("Int. Resistance (Ohms)", intR.str(), FieldType::NUMBER));
        }
    }
    else if (type == "CLOCK")
    {
        ClockGenerator* clk = dynamic_cast<ClockGenerator*>(component);
        if (clk != nullptr)
        {
            fields.push_back(Field("Half Period (ticks)",
                                   to_string(clk->getHalfPeriodTicks()),
                                   FieldType::NUMBER));
        }
    }
    else if (type == "LED")
    {
        LED* led = dynamic_cast<LED*>(component);
        if (led != nullptr)
        {
            ostringstream oss;
            oss << fixed << setprecision(2) << led->getForwardVoltage();
            fields.push_back(Field("Forward Voltage (V)", oss.str(), FieldType::NUMBER));
        }
    }
    else if (type == "AND"  || type == "OR" ||
             type == "NAND" || type == "XOR")
    {
        LogicGate* gate = dynamic_cast<LogicGate*>(component);
        if (gate != nullptr)
        {
            fields.push_back(Field("Num Inputs",
                                   to_string(gate->getNumInputs()),
                                   FieldType::NUMBER));
            ostringstream oss;
            oss << fixed << setprecision(1) << gate->getPropagationDelayMs();
            fields.push_back(Field("Delay (ms)", oss.str(), FieldType::NUMBER));
        }
    }
    else if (type == "NOT" || type == "DFF")
    {
        LogicGate* gate = dynamic_cast<LogicGate*>(component);
        if (gate != nullptr)
        {
            ostringstream oss;
            oss << fixed << setprecision(1) << gate->getPropagationDelayMs();
            fields.push_back(Field("Delay (ms)", oss.str(), FieldType::NUMBER));
        }
    }
}

void PropertiesDialog::computeLayout()
{
    int totalH = HEADER_H
               + (int)fields.size() * (FIELD_H + FIELD_PADDING)
               + FOOTER_H;

    // Center on screen
    dialogRect.w = DIALOG_W;
    dialogRect.h = totalH;
    dialogRect.x = (windowW - DIALOG_W) / 2;
    dialogRect.y = (windowH - totalH)   / 2;

    // OK button — right side of footer
    okRect.w = BUTTON_W;
    okRect.h = BUTTON_H;
    okRect.x = dialogRect.x + dialogRect.w - BUTTON_W - 12;
    okRect.y = dialogRect.y + dialogRect.h - FOOTER_H + (FOOTER_H - BUTTON_H) / 2;

    // Cancel button — left of OK
    cancelRect.w = BUTTON_W;
    cancelRect.h = BUTTON_H;
    cancelRect.x = okRect.x - BUTTON_W - 10;
    cancelRect.y = okRect.y;
}


// ---- Private: Apply ----

bool PropertiesDialog::applyToComponent()
{
    if (targetComponent == nullptr || fields.empty())
    {
        return false;
    }

    // Apply label (always first field)
    targetComponent->setLabel(fields[0].value);

    string type = targetComponent->getType();

    // Apply type-specific numeric fields
    // We use try/catch around stof/stoi to handle invalid input
    try
    {
        if (type == "RESISTOR" && fields.size() >= 2)
        {
            Resistor* r = dynamic_cast<Resistor*>(targetComponent);
            if (r != nullptr) r->setResistance(stof(fields[1].value));
        }
        else if (type == "CAPACITOR" && fields.size() >= 2)
        {
            Capacitor* c = dynamic_cast<Capacitor*>(targetComponent);
            if (c != nullptr) c->setCapacitance(stof(fields[1].value));
        }
        else if (type == "INDUCTOR" && fields.size() >= 2)
        {
            Inductor* l = dynamic_cast<Inductor*>(targetComponent);
            if (l != nullptr) l->setInductance(stof(fields[1].value));
        }
        else if (type == "DCVOLTAGE" && fields.size() >= 2)
        {
            DCVoltageSource* v = dynamic_cast<DCVoltageSource*>(targetComponent);
            if (v != nullptr) v->setVoltage(stof(fields[1].value));
        }
        else if (type == "BATTERY" && fields.size() >= 3)
        {
            Battery* b = dynamic_cast<Battery*>(targetComponent);
            if (b != nullptr)
            {
                b->setEmf(stof(fields[1].value));
                b->setInternalResistance(stof(fields[2].value));
            }
        }
        else if (type == "CLOCK" && fields.size() >= 2)
        {
            ClockGenerator* clk = dynamic_cast<ClockGenerator*>(targetComponent);
            if (clk != nullptr) clk->setHalfPeriodTicks(stoi(fields[1].value));
        }
        else if (type == "LED" && fields.size() >= 2)
        {
            LED* led = dynamic_cast<LED*>(targetComponent);
            if (led != nullptr) led->setForwardVoltage(stof(fields[1].value));
        }
        else if ((type == "AND"  || type == "OR" ||
                  type == "NAND" || type == "XOR") && fields.size() >= 3)
        {
            LogicGate* gate = dynamic_cast<LogicGate*>(targetComponent);
            if (gate != nullptr)
            {
                gate->setPropagationDelayMs(stod(fields[2].value));
                // Note: changing numInputs requires rebuilding the component —
                // for now we only update the delay, numInputs is read-only after placement.
            }
        }
        else if ((type == "NOT" || type == "DFF") && fields.size() >= 2)
        {
            LogicGate* gate = dynamic_cast<LogicGate*>(targetComponent);
            if (gate != nullptr) gate->setPropagationDelayMs(stod(fields[1].value));
        }
    }
    catch (...)
    {
        // Invalid numeric input — silently reject and keep dialog open
        return false;
    }

    return true;
}


// ---- Private: Drawing helpers ----

void PropertiesDialog::drawOverlay(SDL_Renderer* renderer) const
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,
                           overlayColor.r, overlayColor.g,
                           overlayColor.b, overlayColor.a);

    SDL_Rect fullScreen = { 0, 0, windowW, windowH };
    SDL_RenderFillRect(renderer, &fullScreen);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void PropertiesDialog::drawField(SDL_Renderer* renderer,
                                  const Field& field,
                                  int x, int y, int w) const
{
    // Field label on the left
    int labelX = dialogRect.x + 12;
    renderText(renderer, field.fieldLabel + ":",
               labelX,
               y + (FIELD_H - 14) / 2,
               textDimColor);

    // Input box
    SDL_Rect inputRect = { x, y + 4, w, FIELD_H - 8 };

    SDL_SetRenderDrawColor(renderer,
                           fieldBgColor.r, fieldBgColor.g,
                           fieldBgColor.b, fieldBgColor.a);
    SDL_RenderFillRect(renderer, &inputRect);

    SDL_Color borderCol = field.focused ? fieldActiveBorderColor
                                        : fieldNormalBorderColor;
    SDL_SetRenderDrawColor(renderer,
                           borderCol.r, borderCol.g,
                           borderCol.b, borderCol.a);
    SDL_RenderDrawRect(renderer, &inputRect);

    // Input text
    renderText(renderer, field.value,
               inputRect.x + 6,
               inputRect.y + (inputRect.h - 14) / 2,
               textColor);

    // Blinking cursor when focused
    if (field.focused)
    {
        int cursorX = inputRect.x + 6 + (int)field.value.size() * 8;
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawLine(renderer,
                           cursorX,
                           inputRect.y + 4,
                           cursorX,
                           inputRect.y + inputRect.h - 4);
    }
}

void PropertiesDialog::renderText(SDL_Renderer* renderer,
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


// ---- Private: Hit testing ----

int PropertiesDialog::fieldIndexAt(int mouseX, int mouseY) const
{
    int x = fieldInputX();
    int w = fieldInputW();

    for (int i = 0; i < (int)fields.size(); i++)
    {
        int y = fieldY(i);
        SDL_Rect inputRect = { x, y + 4, w, FIELD_H - 8 };
        SDL_Point pt = { mouseX, mouseY };

        if (SDL_PointInRect(&pt, &inputRect) == SDL_TRUE)
        {
            return i;
        }
    }

    return -1;
}

int PropertiesDialog::fieldY(int index) const
{
    return dialogRect.y + HEADER_H + FIELD_PADDING
           + index * (FIELD_H + FIELD_PADDING);
}

int PropertiesDialog::fieldInputX() const
{
    // Input box starts at 55% of dialog width from left
    return dialogRect.x + (DIALOG_W * 55 / 100);
}

int PropertiesDialog::fieldInputW() const
{
    return DIALOG_W - (DIALOG_W * 55 / 100) - 12;
}