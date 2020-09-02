#ifndef __JoeSerialAudio_H_
#define __JoeSerialAudio_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Joe's Serial Sound Player settings
//
// Change these values to match the audio settings that you have uploaded to your adafruit player.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define numberOfVoice   50        // This is the number of 'voice' sound files NOT INCLUDING Music and such
                                  // BB-9E has 33 of these.
#define numberOfMusic   6         // This is the number of 'music' files
// Below are used for the multipress button sounds. Pressing button 1 on the left or button 3 on the right once plays a
// speech track at random, pressing 2-6 times will play quickVoice1-5. 
#define quickVoice1     6
#define quickVoice2     8
#define quickVoice3     20
#define quickVoice4     22
#define quickVoice5     1
// Below are used for the multipress button sounds. Pressing button 2 on the left once plays a sound at random, pressing
// 2-6 times will play quickMusic1-5. 
#define quickMusic1     33
#define quickMusic2     34
#define quickMusic3     35
#define quickMusic4     36
#define quickMusic5     38
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __JoeSerialAudio_H_
