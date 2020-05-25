# JoesDrive
Modifications to Joe's Drive, including the code changes to make the flywheel MK3 I developed work correctly

This will conttain all the code for my Joe's drive setup. I started pretty early and have some custom stuff. My layout is as follows:

# Arduino Mega
The mega is the main brains of the drive, and it is responsible for handling the remote functions and reading the IMU sub-board from the serial terminals.

It currently has 6 digital I/0 lines (with a resistor) going into my custom Igniter 3 BB-8 sound generator to play sounds and music independently. This means BB-8 can still 'talk' while music is playing.

The code for this Mega is in the folder Main_Drive in this repository (keeping with Joe's naming)

# Arduino Pro Mini - IMU reader
Between MK2 and MK3 head update, Joe split the IMU to a separate Pro Mini. I incorporated this change as it is much safer to get the IMU data via serial from the MEGA, as if the IMU locks it won't cause the drive to go wonky, it will just stop.

This code is NOT included in this repository, as it is 100% the same at this point as when Joe created it. If I need to make customizations I will add it here.

# Arduino Pro Mini - Remote
I'm using the original remote from Joe that is run off an Arduino Pro Mini. This is the single remote with two Audafruit sticks, one OLED display, and a small PSP stick on back to do the flywheel.

The code for this is under BB8_remote, to keep consistent with Joe's naming

# Customized I2 and I3 boards
I have two customized I2 boards, one running the lights in the body and one running the lights in the dome. These connect with an XBee Zigbee mesh network to the I3 in the drive. All of these are using NEC code so for now I won't share this but anyone that wants the boards can get some from me at a good price, with the custom firmware loaded.
