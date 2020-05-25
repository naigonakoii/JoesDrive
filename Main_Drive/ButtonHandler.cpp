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

