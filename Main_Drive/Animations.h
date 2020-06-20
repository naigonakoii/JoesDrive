// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive Powered by Naigon
//                         19 June 2020
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// This is part of the add-on library for Joe's Drive created by Naigon.
// ====================================================================================================================

#include "Arduino.h"

#define NOT_RUNNING -1024

struct AnimationState
{
    bool hasResult = false;
    int ch1 = NOT_RUNNING;                //right joystick up/down
    int ch2 = NOT_RUNNING;                //right joystick left/right
    int ch3 = NOT_RUNNING;                //left joystick up/down
    int ch4 = NOT_RUNNING;                //left joystick left/right
    int ch5 = NOT_RUNNING;                //flywheel
    // TODO - sound
};

using UpdateFunc = void (*)(AnimationState&);


static void Down(AnimationState& state)
{
    state.ch3 = 0;
}

static void Up(AnimationState& state)
{
    state.ch3 = 512 / 2;
}
    
class AnimateYes
{
public:
    bool GetIsRunning()
    {
        return this->isRunning;
    }

    AnimateYes()
    {
        this->isRunning = false;
        this->numberOfAnimationSteps = 5;
        this->millisCounts = new int[5];
        for (int i = 0; i < 5; i++)
        {
            this->millisCounts[i] = 400;
        }

        this->updateFunctions = new UpdateFunc[5];
        
        for (int i = 0; i < 5; i++)
        {
            if (i % 2 == 0)
            {
                this->updateFunctions[i] = Down;
            }
            else
            {
                this->updateFunctions[i] = Up;
            }
        }
    }
    
    void Start()
    {
        this->isRunning = true;
        this->currentSubAnimation = -1;
        this->currentMillis = 0;
    }

    AnimationState RunIteration()
    {
        AnimationState result;
    
        if (!this->isRunning)
        {
            result.hasResult = false;
            return result;
        }
        
        // -------------------------------------------------------------------------------------
        // Animate YES - this will do two head nods, and a sound
        // Nod will go down for one second, back up half second, then again for another sec
        // -------------------------------------------------------------------------------------

        for (int i = 0; i < this->numberOfAnimationSteps; i++)
        {
            if ((this->currentSubAnimation == i && millis() - this->currentMillis > this->millisCounts[i])
                || this->currentSubAnimation == -1)
            {
                this->currentMillis = millis();
                this->currentSubAnimation++;
                break;
            }
        }

        if (this->currentSubAnimation >= this->numberOfAnimationSteps)
        {
            result.hasResult = false;
        }
        else
        {
            result.hasResult = true;
            this->updateFunctions[this->currentSubAnimation](result);
        }

        return result;
    }
  
private:
  bool isRunning;
  int currentSubAnimation;
  unsigned long currentMillis;
  int numberOfAnimationSteps;
  int* millisCounts;
  UpdateFunc* updateFunctions;
};

