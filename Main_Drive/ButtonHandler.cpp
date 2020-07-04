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

#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(int onVal, unsigned long heldDuration)
    : onValue(onVal)
    , state(ButtonState::NotPressed)
    , isTrackingPress(false)
    , isHeld(false)
    , longPressMillis(heldDuration)
{
}

ButtonState ButtonHandler::GetState() const
{
    return state;
}

void ButtonHandler::UpdateState(int value)
{
    unsigned long currentMillis = millis();
    
    if (value == onValue && !isTrackingPress)
    {
        isTrackingPress = true;
        firstPressMillis = millis();
        state = ButtonState::NotPressed;
    }
    else if (value == onValue && isTrackingPress && currentMillis - firstPressMillis > longPressMillis)
    {
        state = ButtonState::Held;
        isHeld = true;
    }
    else if (value != onValue && isTrackingPress && !isHeld)
    {
        state = ButtonState::Pressed;   
        isTrackingPress = false;
    }
    else if (value != onValue)
    {
        isTrackingPress = false;
        isHeld = false;
        state = ButtonState::NotPressed;
    }
}

