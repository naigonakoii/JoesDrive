// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//             Offsets
//             Loads the current offsets from disk
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             Scott DeBoer
//             07 July 2020
// ====================================================================================================================

#include "Offsets.h"

//
// External library includes. These are included in the /ext folder, but you'll need to install them into the Arduino
// Library folder (Documents/Arduino/Libraries in Windows).
//
#include <EEPROMex.h> // https://github.com/thijse/Arduino-EEPROMEx

namespace Naigon::BB_8
{

enum OffsetIndicies : uint8_t
{
    Pitch = 0,
    Roll = 1,
    SideToSide = 2,
    DomeTilt = 3,
    DomeSpin = 4,
};

Offsets::Offsets()
    // Pitch and Roll are float; remaining are int.
    : _offsetsNeedWrite(false)
    , _domeOffsetNeedsWrite(false)
    , _isValid(false)
{
    for (uint8_t i = 0; i < 5; i++)
    {
        _offsetIsFloat[i] = i < 2 ? true : false;
    }
}

bool Offsets::NeedsWrite() const
{
    return _offsetsNeedWrite || _domeOffsetNeedsWrite;
}

bool Offsets::AreValuesLoaded() const
{
    // It can be valid from initial load, or by manually setting values for both dome and normal.
    return _isValid || (_isDomeValid && _areOffsetsValid);
}

float Offsets::PitchOffset() const
{
    return _offsets[OffsetIndicies::Pitch];
}

float Offsets::RollOffset() const
{
    return _offsets[OffsetIndicies::Roll];
}

int Offsets::SideToSidePotOffset() const
{
    return _offsets[OffsetIndicies::SideToSide];
}

int Offsets::DomeTiltPotOffset() const
{
    return _offsets[OffsetIndicies::DomeTilt];
}

int Offsets::DomeSpinPotOffset() const
{
    return _offsets[OffsetIndicies::DomeSpin];
}

bool Offsets::LoadOffsetsFromMemory()
{
    float sum = 0.0f;
    for (uint8_t i = 0; i < 5; i++)
    {
        _offsets[i] = _offsetIsFloat[i]
            ? EEPROM.readFloat(i * 4)
            : EEPROM.readInt(i * 4);
        sum += _offsets[i];
    }

    // If the sum is non-zero(ish) then we can assume values were loaded. Zero means this is the first time the user
    // ran the drive and values need to be stored.
    _isValid = abs(sum) > 0.05f;
    return _isValid;
}

void Offsets::UpdateOffsets(float pitchOffset, float rollOffset, int sideToSideOffset, int domeTiltOffset)
{
    _offsets[OffsetIndicies::Pitch] = pitchOffset * -1;
    _offsets[OffsetIndicies::Roll] = rollOffset * -1;
    _offsets[OffsetIndicies::SideToSide] = 0 - sideToSideOffset;
    _offsets[OffsetIndicies::DomeTilt] = 0 - domeTiltOffset;

    _offsetsNeedWrite = true;
    _areOffsetsValid = true;
}

void Offsets::UpdateDomeOffset(int domeSpinOffset, bool isDomeSpinReversed)
{
    _offsets[OffsetIndicies::DomeTilt] = isDomeSpinReversed
        ? 180 - domeSpinOffset
        : 0 - domeSpinOffset;

    _domeOffsetNeedsWrite = true;
    _writeStep = 0;
    _isDomeValid = true;
}

void Offsets::WriteOffsets()
{
    // No-op if nothing needs to be written.
    if (!NeedsWrite()) { return; }

    if (_domeOffsetNeedsWrite)
    {
        EEPROM.writeInt(OffsetIndicies::DomeSpin * 4, _offsets[OffsetIndicies::DomeSpin]);
        _domeOffsetNeedsWrite = false;
    }

    if (_offsetsNeedWrite)
    {
        if (_offsetIsFloat[_writeStep])
        {
            EEPROM.writeFloat(_writeStep * 4, _offsets[_writeStep]);
        }
        else
        {
            EEPROM.writeInt(_writeStep * 4, _offsets[_writeStep]);
        }
        
        _writeStep++;

        if (_writeStep > 3)
        {
            _writeStep = 0;
            _offsetsNeedWrite = false;
        }
    }
}

} // namespace Naigon::BB_8
