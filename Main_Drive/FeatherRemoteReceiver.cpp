// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

#include "FeatherRemoteReceiver.h"

#include <EasyTransfer.h>

namespace Naigon::BB_8
{

FeatherRemoteReceiver::FeatherRemoteReceiver(uint16_t delay)
    : _receiveDelay(delay)
    , _receivedData(false)
    , _isControllerConnected(false)
    , _lastReceivedMillis(0)
{
}

bool FeatherRemoteReceiver::ReceivedData() const
{
    return _receivedData;
}

bool FeatherRemoteReceiver::IsControllerConnected() const
{
    return _isControllerConnected;
}

void FeatherRemoteReceiver::UpdateIteration(EasyTransfer *remoteData)
{
    _receivedData = false;
    if(millis() - _lastReceivedMillis < _receiveDelay) { return; }

    if(Serial1.available() > 0)
    {
        remoteData->receiveData();
        _lastReceivedMillis = millis();
        _receivedData = true;
    }

    _isControllerConnected = millis() - _lastReceivedMillis >= 600
        ? false
        : true;
}

} // namespace Naigon::BB_8
