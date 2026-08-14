#pragma once

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "core/Component.h"

using namespace std;

// ============================================================
// PropertiesDialog — a modal popup dialog for editing a
// component's properties.
//
// Triggered by: double-clicking any component on the canvas.
//
// Behavior:
//   - Inspects the component type and builds the correct set
//     of editable fields (label, resistance, voltage, etc.)
//   - User edits the fields, then clicks OK or Cancel
//   - OK applies all changes back to the component
//   - Cancel discards all changes
//   - Escape key = Cancel
//   - Enter key  = OK
//
// The dialog is modal — while it is open, MainEditorScreen
// should not pass events to the Canvas.
//
// Fields are dynamic based on component type:
//   All components : Label
//   RESISTOR       : Label, Resistance (Ohms)
//   CAPACITOR      : Label, Capacitance (Farads)
//   INDUCTOR       : Label, Inductance (Henries)
//   DCVOLTAGE      : Label, Voltage (V)
//   BATTERY        : Label, EMF (V), Internal Resistance (Ohms)
//   CLOCK          : Label, Half Period (ticks)
//   LED            : Label, Forward Voltage (V)
//   AND/OR/NAND/XOR: Label, Num Inputs, Propagation Delay (ms)
//   NOT / DFF      : Label, Propagation Delay (ms)
// ============================================================

class PropertiesDialog
{
public:

    PropertiesDialog(int windowW, int windowH, TTF_Font* font);

    // ---- Lifecycle ----

    // Opens the dialog for the given component.
    // Inspects its type and populates the field list.
    void open(Component* component);

    // Closes and resets the dialog without applying changes.
    void close();

    // Returns true while the dialog is visible and active.
    bool isOpen() const;

    // ---- Event handling ----

    // Returns true if event was consumed.
    // Always consume all events while the dialog is open.
    bool handleEvent(const SDL_Event& event);

    // ---- Draw ----

    void draw(SDL_Renderer* renderer) const;

    // ---- Result ----

    // Returns true if the user clicked OK since last check.
    // MainEditorScreen calls this to know when to save an undo snapshot.
    bool wasAccepted() const;
    void clearAccepted();

private:

    // ---- Field definition ----

    enum class FieldType
    {
        TEXT,      
        NUMBER   
    };

    struct Field
    {
        string    fieldLabel;     // shown to the left of the input box
        string    value;          // current text in the input box
        FieldType type;
        bool      focused;        // true when this field is receiving input

        Field(const string& label, const string& value, FieldType type)
            : fieldLabel(label), value(value), type(type), focused(false)
        {}
    };

    // ---- Layout constants ----
    static const int DIALOG_W      = 320;
    static const int HEADER_H      = 36;
    static const int FIELD_H       = 30;
    static const int FIELD_PADDING = 8;
    static const int BUTTON_H      = 32;
    static const int BUTTON_W      = 90;
    static const int FOOTER_H      = 48;

    int windowW;
    int windowH;
    TTF_Font* font;              

    Component*   targetComponent; // the component being edited
    bool         dialogOpen;
    bool         accepted;

    vector<Field> fields;

    // Computed dialog rect (centered on screen)
    SDL_Rect dialogRect;

    // OK and Cancel button rects
    SDL_Rect okRect;
    SDL_Rect cancelRect;

    bool okHovered;
    bool cancelHovered;

    // Colors
    SDL_Color bgColor;
    SDL_Color headerColor;
    SDL_Color borderColor;
    SDL_Color fieldBgColor;
    SDL_Color fieldActiveBorderColor;
    SDL_Color fieldNormalBorderColor;
    SDL_Color textColor;
    SDL_Color textDimColor;
    SDL_Color overlayColor;
    SDL_Color okBtnColor;
    SDL_Color okBtnHoverColor;
    SDL_Color cancelBtnColor;
    SDL_Color cancelBtnHoverColor;

    // ---- Setup ----

    // Builds the field list based on the component type
    void buildFields(Component* component);

    // Computes dialog rect and button rects based on field count
    void computeLayout();

    // ---- Apply ----

    // Parses all fields and writes values back to the component.
    // Returns false if any numeric field has invalid input.
    bool applyToComponent();

    // ---- Drawing helpers ----

    void drawOverlay(SDL_Renderer* renderer)  const;
    void drawField(SDL_Renderer* renderer,
                   const Field& field,
                   int x, int y, int w)      const;

    void renderText(SDL_Renderer* renderer,
                    const string& text,
                    int x, int y,
                    SDL_Color color)          const;

    // ---- Hit testing ----

    int fieldIndexAt(int mouseX, int mouseY) const;

    // Returns the Y position of a field by its index
    int fieldY(int index) const;
    int fieldInputX()     const;
    int fieldInputW()     const;
};