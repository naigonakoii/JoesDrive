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

#ifndef __Naigon_Offsets_h_
#define __Naigon_Offsets_h_

#include "Arduino.h"

namespace Naigon::BB_8
{

///////////////////////////////////////////////////////////////////////////////////////
// @summary Wraps the imu and pot offsets and EEPROM loading/saving functionality.
//
//          NOTE: this class should be treated as a singleton, and only one instance
//          per main program should be instanciated.
///////////////////////////////////////////////////////////////////////////////////////
class Offsets
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs a new instance of the Offsets class. Only ONE (1) class
    //          should be active for the entire program.
    ///////////////////////////////////////////////////////////////////////////////////
    Offsets();

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Indicates if the current instance needs to be written.
    //
    // @ret     true if this instance needs to be written to EEPROM; otherwise, false.
    ///////////////////////////////////////////////////////////////////////////////////
    bool NeedsWrite() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Indicates whether values have been successfully loaded from EEPROM.
    //          This will also be true if all offsets have been manually updated.
    //
    // @ret     true if all vales are loaded from EEPROM or manually set; otherwise,
    //          false.
    ///////////////////////////////////////////////////////////////////////////////////
    bool AreValuesLoaded() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the pitch offset from this instance.
    ///////////////////////////////////////////////////////////////////////////////////
    float PitchOffset() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the roll offset from this instance.
    ///////////////////////////////////////////////////////////////////////////////////
    float RollOffset() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the side to side pot offset from this instance.
    ///////////////////////////////////////////////////////////////////////////////////
    int SideToSidePotOffset() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the dome tilt pot offset from this instance.
    ///////////////////////////////////////////////////////////////////////////////////
    int DomeTiltPotOffset() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the dome spin continuous pot offset from this instance.
    ///////////////////////////////////////////////////////////////////////////////////
    int DomeSpinPotOffset() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Loads the offsets from EEPROM. If successful, this method returns true
    //          as well as internally storing that load was successful.
    //
    // @ret     true if the load was successful; otherwise, false.
    ///////////////////////////////////////////////////////////////////////////////////
    bool LoadOffsetsFromMemory();

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Update the offsets for this instance.
    //
    // @param   pitchOffset
    //          Current pitch offset from the imu.
    //
    // @param   rollOffset
    //          Current roll offset from the imu.
    //
    // @param   sideToSideOffset
    //          Side to side pot offset from the side to side pot handler.
    //
    // @param   domeTiltOffset
    //          Dome tilt offset from the dome tilt pot handler.
    ///////////////////////////////////////////////////////////////////////////////////
    void UpdateOffsets(
        float pitchOffset,
        float rollOffset,
        int sideToSideOffset,
        int domeTiltOffset);

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Updates the dome offset for this instance.
    //
    // @param   domeSpinOffset
    //          Updates the dome spin offset from the dome spin pot handler.
    //
    // @param   isDomeSpinReversed
    //          Set to true if the drive is in reverse mode.
    ///////////////////////////////////////////////////////////////////////////////////
    void UpdateDomeOffset(int domeSpinOffset, bool isDomeSpinReversed);

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Perform the write to EEPROM. This method is a no-op if all values have
    //          already been written.
    ///////////////////////////////////////////////////////////////////////////////////
    void WriteOffsets();

private:
    float _offsets[5];
    bool _offsetIsFloat[5];
    bool _offsetsNeedWrite, _domeOffsetNeedsWrite;
    bool _isValid, _isDomeValid, _areOffsetsValid;
    int _writeStep;
};

} // namespace Naigon::BB_8

#endif // __Naigon_Offsets_h_
