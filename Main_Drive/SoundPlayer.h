// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive Powered by Naigon
//                         19 June 2020
//                         Scott DeBoer
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// This is part of the add-on library for Joe's Drive created by Naigon.
// ====================================================================================================================

namespace NaigonBB8
{

#include "Arduino.h"

// Naigon sound pins
enum SoundTypes : int8_t
{
    NotPlaying = -1,
    Happy = 0,
    Sad = 1,
    Excited = 2,
    Scared = 3,
    Chatty = 4,
    Agitated = 5,
    PlayTrack = 6,
    StopTrack = 7,
    COUNT = 8,
};

class SoundMapper
{
public:
    SoundMapper(int happyPin, int sadPin, int excitedPin, int scaredPin, int chattyPin, int agitatedPin, int playTrackPin, int stopTrackPin);

    int Pin(SoundTypes soundType) const;

private:
    int m_mappings[8];
};

struct ISoundPlayer
{
    // Call this method on each iteration of the main loop to play the specified sound. Use the special NotPlaying enum to indicate no sounds
    // will be played for this iteration.
    virtual void PlaySound(SoundTypes type) = 0;

    // Forcabily clear all sounds, and reset counters.
    virtual void ClearSounds() = 0;

    virtual SoundTypes SoundTypeCurrentlyPlaying() const = 0;
    virtual SoundTypes TrackTypeCurrentlyPlaying() const = 0;
};

//
// Class that plays sounds by writing high to pins that are connected to the player.
//
// Pins will remaine high for the specified number of execution cycles; calls to PlaySound will reset the pins to low after the count has expired.
// 
// This class assumes that only one sound at a time can play of the main six sounds, but allows tracks and a sound to occur.
//
class WiredSoundPlayer : public ISoundPlayer
{
public:
    WiredSoundPlayer(const SoundMapper& mapper, unsigned long pinHighMs);

    // ISoundPlayer interface
    virtual void PlaySound(SoundTypes type) override;
    virtual void ClearSounds() override;
    virtual SoundTypes SoundTypeCurrentlyPlaying() const override;
    virtual SoundTypes TrackTypeCurrentlyPlaying() const override;
    
private:
    const SoundMapper& m_mapper;
    int playingIndex;
    int trackIndex;
    unsigned long playMillis;
    unsigned long trackMillis;
    int totalMillis;
};

} // namespace NaigonBB8
