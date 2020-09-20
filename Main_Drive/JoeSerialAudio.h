// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//             JoeSerialAudio
//             Wrapper for Joe's serial audio playing via the Adafruit soundboard.
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             Scott DeBoer
//             19 Sept 2020
// ====================================================================================================================

#ifndef __JoeSerialAudio_H_
#define __JoeSerialAudio_H_

#include "Arduino.h"
#include "AudioConstants.h"

class Adafruit_Soundboard;

namespace Naigon::BB_8
{

class JoeSerialAudio
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs a new instance of the JoeSerialAudio class.
    //
    // @param   sfx
    //          Pointer to an initialized Adafruit soundboard object.
    //
    // @param   actPin
    //          Pin of the arduino that connects to the ACT pin on the soundboard.
    ///////////////////////////////////////////////////////////////////////////////////
    JoeSerialAudio(Adafruit_Soundboard *sfx, uint16_t actPin);

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Call once per loop iteration.
    ///////////////////////////////////////////////////////////////////////////////////
    void UpdateIteration();

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Determins if any sound is currently playing.
    //
    // @ret     true if a sound is playing, otherwise false.
    ///////////////////////////////////////////////////////////////////////////////////
    bool IsPlaying() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Determins if a music sound is currently playing.
    //
    // @ret     true if a music sound is currently being played, otherwise false;
    ///////////////////////////////////////////////////////////////////////////////////
    bool IsMusicPlaying() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Play a sound in order serial soundboard.
    ///////////////////////////////////////////////////////////////////////////////////
    void PlayNextSound();

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Play a random sound.
    ///////////////////////////////////////////////////////////////////////////////////
    void PlayRandomSound();

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Play the next music track.
    ///////////////////////////////////////////////////////////////////////////////////
    void PlayMusic();

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Stop a currently playing music. No op if no music is being played.
    ///////////////////////////////////////////////////////////////////////////////////
    void StopMusic();

private:
    void Play(uint8_t num);

    Adafruit_Soundboard *_sfx;
    uint16_t _actPin;
    int _currentSound;
    int _currentMusic;
    bool _isPlaying;
    bool _isMusicPlaying;
    unsigned long _startedPlayingMillis;

};

} // namespace Naigon::BB_8

#endif // __JoeSerialAudio_H_
