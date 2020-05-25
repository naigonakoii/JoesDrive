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
    ButtonHandler(int onVal, unsigned long heldDuration);

    ButtonState GetState() const;
    
    void UpdateState(int value);
    
private:
    bool isTrackingPress;
    bool isHeld;
    unsigned long firstPressMillis;
    ButtonState state;
    int onValue;
    unsigned long longPressMillis;
};

