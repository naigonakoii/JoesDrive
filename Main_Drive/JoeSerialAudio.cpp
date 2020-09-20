#include "JoeSerialAudio.h"
#include <Adafruit_Soundboard.h>

namespace Naigon::BB_8
{

JoeSerialAudio::JoeSerialAudio(Adafruit_Soundboard *sfx, uint16_t actPin)
    : _actPin(actPin)
    , _isPlaying(false)
    , _isMusicPlaying(false)
    , _currentSound(-1)
    , _currentMusic(-1)
    , _sfx(sfx)
{
}

bool JoeSerialAudio::IsPlaying() const
{
    return _isPlaying;
}

bool JoeSerialAudio::IsMusicPlaying() const
{
    return _isMusicPlaying;
}

void JoeSerialAudio::UpdateIteration()
{
    if (_isPlaying
        && (millis() - _startedPlayingMillis) > 600
        && digitalRead(_actPin) == HIGH)
    {
        // Because the adafruit soundboard is slow to flip its playing bit to low, a small delay is elapsed before
        // checking if the sound is done playing. This ensures that the sound will be marked as playing immediately.
        StopMusic();
    }
}

void JoeSerialAudio::PlayNextSound()
{
    _currentSound = _currentSound == numberOfVoice - 1
        ? 0
        : _currentSound + 1;
    Play(_currentSound);
}

void JoeSerialAudio::PlayRandomSound()
{
    int voiceNum = random(0, numberOfVoice);
    Play(voiceNum);
}

void JoeSerialAudio::PlayMusic()
{
    _currentMusic = _currentMusic == numberOfMusic - 1
        ? 0
        : _currentMusic + 1;
    Play(_currentMusic);
    _isMusicPlaying = true;
}

void JoeSerialAudio::StopMusic()
{
    if (_isMusicPlaying)
    {
        _sfx->stop();
    }

    // Stop playing music per button request.
    _isMusicPlaying = false;
    _isPlaying = false;
}

void JoeSerialAudio::Play(uint8_t num)
{
    if (!_sfx->playTrack(num))
    {
        // If the track failed to play, assume we need to stop an existing one.
        _sfx->stop();
        delay(2);
        _sfx->playTrack(num);
    }

    _isPlaying = true;
    _startedPlayingMillis = millis();
}

} // namespace Naigon::BB_8
