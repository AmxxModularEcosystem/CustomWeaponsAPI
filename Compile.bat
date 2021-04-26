@echo off

ECHO Compiling...
copy /y .\include\cwapi.inc C:\AmxModX\1.9.0\include
copy /y .\include\cwapi.inc C:\AmxModX\1.10.0\include

amxx190 .\CustomWeaponsAPI.sma

ECHO .
ECHO .
ECHO .
set /p set q=Done...