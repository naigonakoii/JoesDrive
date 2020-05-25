#include "SoundPlayer.h"

// =====================================================================================================================================================================================================
// SoundMapper class
// =====================================================================================================================================================================================================
SoundMapper::SoundMapper(int happyPin, int sadPin, int excitedPin, int scaredPin, int chattyPin, int agitatedPin, int playTrackPin, int stopTrackPin)
{
    m_mappings[(int)SoundTypes::Happy] = happyPin;
    m_mappings[(int)SoundTypes::Sad] = sadPin;
    m_mappings[(int)SoundTypes::Excited] = excitedPin;
    m_mappings[(int)SoundTypes::Scared] = scaredPin;
    m_mappings[(int)SoundTypes::Chatty] = chattyPin;
    m_mappings[(int)SoundTypes::Agitated] = agitatedPin;
    m_mappings[(int)SoundTypes::PlayTrack] = playTrackPin;
    m_mappings[(int)SoundTypes::StopTrack] = stopTrackPin;
}

int SoundMapper::Pin(SoundTypes type) const
{
    return m_mappings[(int)type];
}
// =====================================================================================================================================================================================================

// =====================================================================================================================================================================================================
// WiredSoundPlayer class
// =====================================================================================================================================================================================================
WiredSoundPlayer::WiredSoundPlayer(const SoundMapper& mapper, unsigned long pinHighMs)
    : m_mapper(mapper)
    , playingIndex(-1)
    , trackIndex(-1)
    , playMillis(0)
    , trackMillis(0)
    , totalMillis(pinHighMs)
{
}

void WiredSoundPlayer::PlaySound(SoundTypes type)
{
    if ((type == SoundTypes::PlayTrack || type == SoundTypes::StopTrack)
        && trackIndex != (int)type)
    {
        trackMillis = millis();
        trackIndex = (int)type;
    }
    else if (type != SoundTypes::NotPlaying && playingIndex != (int)type)
    {
        // A new type is playing.
        playMillis = millis();
        playingIndex = (int)type;
    }

    if (trackIndex != -1 && millis() - trackMillis > totalMillis)
    {
        trackIndex = -1;
    }

    if (playingIndex != -1 && millis() - playMillis > totalMillis)
    {
        playingIndex = -1;
    }
    
    for (int i = 0; i < SoundTypes::COUNT; i++)
    {
        if (i == playingIndex || i == trackIndex)
        {
            digitalWrite(m_mapper.Pin(i), HIGH);
        }
        else
        {
            digitalWrite(m_mapper.Pin(i), LOW);
        }
    }
}

void WiredSoundPlayer::ClearSounds()
{
    trackMillis = playMillis = 0;
    trackIndex = playingIndex = -1;
    
    for (int i = 0; i < SoundTypes::COUNT; i++)
    {
        digitalWrite(m_mapper.Pin(i), LOW);
    }
}

SoundTypes WiredSoundPlayer::SoundTypeCurrentlyPlaying() const
{
    return (SoundTypes)playingIndex;
}

SoundTypes WiredSoundPlayer::TrackTypeCurrentlyPlaying() const
{
    return (SoundTypes)trackIndex;
}

