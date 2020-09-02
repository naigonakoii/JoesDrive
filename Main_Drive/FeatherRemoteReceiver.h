// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//             FeatherRemoteReceiver
//             Receives data from an external Feather which uses its on-board wireless sensor to connect with a single
//             or pair of external remotes. Uses serial communication
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             Scott DeBoer
//             16 July 2020
// ====================================================================================================================

#ifndef __Naigon_FeatherRemoteReceiver_h_
#define __Naigon_FeatherRemoteReceiver_h_

#include "Arduino.h"

// Forward declare EasyTransfer
class EasyTransfer;

namespace Naigon::BB_8
{

class FeatherRemoteReceiver
{
public:
    FeatherRemoteReceiver(uint16_t delay);

    bool ReceivedData() const;
    bool IsControllerConnected() const;
    void UpdateIteration(EasyTransfer *remote);

private:
    unsigned long _lastReceivedMillis;
    uint16_t _receiveDelay;
    bool _receivedData, _isControllerConnected;
};

} // namespace Naigon::BB_8

#endif // __Naigon_FeatherRemoteReceiver_h_
