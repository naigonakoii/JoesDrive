// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive - Main drive mechanism using Ardino Mega with secondary Arduino Pro Mini IMU
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                         Joe's Drive powered by Naigon
//                         27 May 2020
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             ***         You are free to use, and share this code as long as it is not sold. There is no warranty,
//                         guarantee, or other tomfoolery.
//                         This entire project was masterminded by an average Joe, your mileage may vary.
// ====================================================================================================================
//                         Written by Joe Latiola - https://www.facebook.com/groups/JoesDrive/
//                         You will need libraries: EepromEX: https://github.com/thijse/Arduino-EEPROMEx
//                                                  PIDLibrary: http://playground.arduino.cc/Code/PIDLibrary
//                                                  EasyTransfer: https://github.com/madsci1016/Arduino-EasyTransfer
//
// ====================================================================================================================
// ====================================================================================================================


void debugRoutines()
{
    // Uncomment " #Define " above
#ifdef debugDrive
    Serial.print(F(" joystickDrive: "));
    Serial.print(joystickDrive);
    Serial.print(F(" SetDrive: "));
    Serial.print(Setpoint3);
    Serial.print(F(" InDrive: "));
    Serial.print(Input3);
    Serial.print(F(" OutDrive: "));
    Serial.print(Output3);
#endif

#ifdef debugS2S
    Serial.print(F(" joystickS2S: "));
    Serial.print(joystickS2S);
    Serial.print(F(" Roll: "));
    Serial.print(roll);
    Serial.print(F(" RollOffset: "));
    Serial.print(rollOffset);
    Serial.print(F(" S2SPot: "));
    Serial.print(S2Spot);
    Serial.print(F(" In2: "));
    Serial.print(Input2);
    Serial.print(F(" Set2: "));
    Serial.print(Setpoint2);
    Serial.print(F(" Out2/Set1: "));
    Serial.print(Output2);
    Serial.print(F(" In1: "));
    Serial.print(Input1);
    Serial.print(F(" Out1: "));
    Serial.println(Output1);
#endif

#ifdef debugDomeTilt
    Serial.print(F(" joystickDome: "));
    Serial.print(joystickDome);
    Serial.print(F(" In4 :"));
    Serial.print(Input4);
    Serial.print(F(" Set4 :"));
    Serial.print(Setpoint4);
    Serial.print(F(" Out4 :"));
    Serial.println(Output4);
#endif

#ifdef debugdomeRotation
    Serial.print(F(" domeRotation: "));
    Serial.print(domeRotation);
    Serial.print(F(" currentDomeSpeed: "));
    Serial.print(currentDomeSpeed);
    Serial.print(F(" ch4Servo: "));
    Serial.print(ch4Servo);
    Serial.print(F(" In5: "));
    Serial.print(Input5);
    Serial.print(F(" Set5: "));
    Serial.print(Setpoint5);
    Serial.print(F(" Out5: "));
    Serial.print(Output5);
    //Serial.print(F(" yawOffset: "));
    //Serial.print(yawOffset);
    Serial.print(F(" domeServo: "));
    Serial.print(domeServo);
    //Serial.print(F(" domeYaw: "));
    //Serial.print(recFromDome.domeYaw);
    //Serial.print(F(" yaw: "));
    //Serial.print(yaw);
    Serial.print(F(" pot: "));
    Serial.println(analogRead(domeSpinPot));
#endif

#ifdef debugSound
    Serial.print(F(" playing sound: "));
    Serial.println(soundPlayer->SoundTypeCurrentlyPlaying());
#endif

#ifdef debugPSI
    Serial.print(F(" readPinState: "));
    Serial.print(readPinState);
    Serial.print(F(" fadeVal: "));
    Serial.print(fadeVal);
#endif

#ifdef printbodyBatt
    Serial.print(F(" Vin: "));
    Serial.print(sendToRemote.bodyBatt);
#endif

#ifdef printYPR
    //Serial.print(F(" Yaw: "));
    //Serial.print(yaw);
    Serial.print(F(" Roll: "));
    Serial.print(roll);
    Serial.print(F("  Pitch: "));
    Serial.println(pitch);
#endif

#ifdef printDome
    //Serial.print(F(" Dome Yaw: "));
    //Serial.print(recFromDome.domeYaw);
    Serial.print(F(" Dome Batt: "));
    Serial.print(recFromDome.domeBatt);
#endif

#ifdef printRemote
    Serial.print(F("  Remote: "));
    Serial.print(recFromRemote.ch1);
    Serial.print(" , ");
    Serial.print(recFromRemote.ch2);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.ch3);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.ch4);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.ch5);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but1);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but2);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but3);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but4);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but5);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but6);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but7);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but8);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.motorEnable);
    Serial.print('\n');
#endif

#ifdef printOffsets
    Serial.print(" pitchOffset: ");
    Serial.print(pitchOffset);
    Serial.print(" rollOffset: ");
    Serial.print(rollOffset);
    Serial.print(" potOffsetS2S: ");
    Serial.print(potOffsetS2S);
    Serial.print("domeTiltPotOffset: ");
    Serial.println(domeTiltPotOffset);
#endif

#ifdef debugRSelectMillis
    //Serial.print(" currentMillisBodyCalib: ");
    //Serial.print(currentMillisBodyCalib);
    Serial.print(" setCalibMillis: ");
    Serial.print(setCalibMillis);
    Serial.print(" motorEnable: ");
    Serial.print(recFromRemote.motorEnable);
    Serial.print(" bodyCalibState: ");
    Serial.print(bodyCalibState);
    Serial.print(" bodyStatus: ");
    Serial.print(sendToRemote.bodyStatus);
    Serial.print(" countdown: ");
    Serial.print(countdown);
#endif

#ifdef printOutputs
    Serial.print(F(" Out1: "));
    Serial.print(abs(Output1));
    Serial.print(F(" Out2: "));
    Serial.print(abs(Output2));
    Serial.print(F(" Out3: "));
    Serial.print(abs(Output3));
    Serial.print(F(" Out4: "));
    Serial.println(abs(Output4));
#endif

#ifdef printSoundPin
    Serial.print(F(" Pin1: "));
    Serial.print(digitalRead(soundpin1));
    Serial.print(F(" Pin2: "));
    Serial.print(digitalRead(soundpin2));
    Serial.print(F(" Pin3: "));
    Serial.print(digitalRead(soundpin3));
    Serial.print(F(" Pin4: "));
    Serial.print(digitalRead(soundpin4));
    Serial.print(F(" Pin5: "));
    Serial.print(digitalRead(soundpin5));
    Serial.print(F(" Pin6: "));
    Serial.print(digitalRead(soundpin6));
    Serial.print(F(" soundState: "));
    Serial.print(soundState);
    Serial.print(F(" readPinState: "));
    Serial.print(digitalRead(readpin));
    Serial.print(F(" randSoundPin: "));
    Serial.println(randSoundPin);
#endif

#ifdef debugFlywheelSpin
    Serial.print(F(" ch5: "));
    Serial.print(recFromRemote.ch5);
    Serial.print(F(" ch5PWM: "));
    Serial.print(ch5PWM);
    Serial.print(F(" flywheelRotation: "));
    Serial.println(flywheelRotation);
#endif
}