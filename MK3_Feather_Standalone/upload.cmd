@echo off
setlocal

pushd ..\
set uploadCmd="%CD%\tools\UploadFirmwareToFeather.cmd"
popd

echo %uploadCmd%

call %uploadCmd% MK3_Feather_Standalone.ino.hex
