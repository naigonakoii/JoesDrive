// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//             ButtonHandler
//             Library for wrapping Arduino digital inputs into buttons that have pres and held states.
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             Scott DeBoer
//             28 June 2020
// ====================================================================================================================


#ifndef __ButtonHandler_h_
#define __ButtonHandler_h_

#include "Arduino.h"

enum ButtonState
{
    NotPressed = 0,
    Pressed = 1,
    Held = 2,
};

class ButtonHandler
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs an instance of the ButtonHandler class.
    //
    //          The Button handler is responsible for remapping digital button I/O
    //          over time into the three possible button states:
    //
    //              1. NotPressed - No button input
    //              2. Pressed - Button was pressed and then released in less than
    //                 'heldDuration' milliseconds.
    //              3. Held - Button was held for at least 'heldDuration' milliseconds.
    //
    // @param   onVal
    //          Indicates whether 0 or 1 is considered to be the pressed state.
    //
    // @param   heldDuration
    //          Time in milliseconds that is considered to be the held state.
    ///////////////////////////////////////////////////////////////////////////////////
    ButtonHandler(int onVal, unsigned long heldDuration);

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current button state for this instance.
    //
    // @ret     Whether the button is in the held, pressed, or not pressed state.
    ///////////////////////////////////////////////////////////////////////////////////
    ButtonState GetState() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Updates this button with the current digital I/O value.
    //
    //          This method should be called once per 'loop()'
    //
    // @param   value
    //          Digital I/O value for the current loop.
    ///////////////////////////////////////////////////////////////////////////////////
    void UpdateState(int value);
    
private:
    bool isTrackingPress;
    bool isHeld;
    unsigned long firstPressMillis;
    ButtonState state;
    int onValue;
    unsigned long longPressMillis;
};

#endif //__ButtonHandler_h_
