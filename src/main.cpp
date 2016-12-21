#include "Arduino.h"
#include <EEPROM.h>
#include <Wire.h>
#include <Time.h>
//#include <TimeAlarms.h>
#include <DS1307RTC.h>

#include "ToggleButton.h"
//#include <LiquidCrystal.h>
//LiquidCrystal lcd(7,8,9,10,11,12);
//#include "Clock.h"
#include "Alarm.h"
#include "Timer.h"

//#define SERIAL_DEBUG

const int _valve[9] = {A2, A3, A4, A5, 8, 9, 10, 11, 12};
const int _mainValve = A1;
const int _button = 6;
const int _buttonLED = 7;

ToggleButton _forceWateringBtn(_button);
ToggleButton _changeAlarmBtn(_button);

bool _stringComplete = false;
char _dataRec[30];
bool _set = false;
bool _get = false;
bool _time = false;
bool _alarm = false;
bool _valveOnTime = false;
bool _valveTransTime = false;
bool _clockOK = false;  //clock have been set
bool _alarmOK = true;  //alarm have been set
bool _blinkButtonLED = false;

Alarm alarm(0, 0);

tmElements_t clock;
Timer _clockTimer(1, 0);
Timer _valveOnTimer(6, 0);
Timer _valveTransTimer(2, 0);
Timer _doubleTapTimer(1, 0);
bool _buttonDoubleTapped = false;
int _tapCounter = 0;
bool _wateringStarted = false;
int _wateringState = 0;

// EEPROM
int _alarmSaveHour = 0;
int _alarmSaveMinute = 1;
int _valveOnTimeSave = 2;
int _valveTransTimeSave = 3;

void RestoreValveOnTimeEEPROM();
void RestoreValvTransTimeEEPROM();
void RestoreAlarmEEPROM();

void setup()
{
    Serial.begin(9600);
    pinMode(_button, INPUT);
    pinMode(_buttonLED, OUTPUT);
    _forceWateringBtn.setButtonPressedTime(3000);

    for(int i = 0; i < 9; i++)
    {
        pinMode(_valve[i], OUTPUT);
    }
    pinMode(_mainValve, OUTPUT);
    _clockTimer.Start();
    RestoreValveOnTimeEEPROM();
    RestoreValvTransTimeEEPROM();
    RestoreAlarmEEPROM();
}


void RestoreValveOnTimeEEPROM()
{
    int seconds = EEPROM.read(_valveOnTimeSave);
    _valveOnTimer.ChangeTimeout(seconds,0);
}


void RestoreValvTransTimeEEPROM()
{
    int seconds = EEPROM.read(_valveTransTimeSave);
    _valveTransTimer.ChangeTimeout(seconds,0);
}

void SaveValveOnTimeEEPROM(int seconds)
{
    EEPROM.update(_valveOnTimeSave,seconds & 0xFF);
}

void SaveValvTransTimeEEPROM(int seconds)
{
    EEPROM.update(_valveTransTimeSave,seconds & 0xFF);
}

void RestoreAlarmEEPROM()
{
    int hour = EEPROM.read(_alarmSaveHour);
    int minute = EEPROM.read(_alarmSaveMinute);
    alarm.setAlarm(hour,minute);
}

void SaveAlarmEEPROM(int hour, int minute)
{
    EEPROM.update(_alarmSaveHour,hour & 0xFF);
    EEPROM.update(_alarmSaveMinute,minute & 0xFF);
}

String zeroPad(int val, int length)
{
    int valLength = 1;
    int valTemp = (val - val%10);
    String addZeros = "";

    while(true)
    {
        if(valLength >= length)
            break;
        if(valTemp <= 1)
            addZeros += "0";
        valTemp /= 10;
        valLength++;
    }
    char valCharArr[33];
    itoa(val,valCharArr,10);
    String valString = String(valCharArr);
    return addZeros+valString;
}

void openValve(int valve)
{
    digitalWrite(valve, true);
#ifdef SERIAL_DEBUG
    Serial.print("Open valve: ");
    Serial.print(valve);
    Serial.println();
#endif
}

void closeValve(int valve)
{
    digitalWrite(valve, false);
#ifdef SERIAL_DEBUG
    Serial.print("Close valve: ");
    Serial.print(valve);
    Serial.println();
#endif
}

void handleDoubleTapp()
{
    _buttonDoubleTapped = false;
    if(_changeAlarmBtn.isButtonPressed())
    {
        _tapCounter++;
    }
    if((_tapCounter == 1) && !_doubleTapTimer.IsTimerStarted())
    {
        _doubleTapTimer.Start();
    }
    if(_tapCounter == 2)
    {
#ifdef SERIAL_DEBUG
        Serial.print("Double Tapp\n");
#endif
        _buttonDoubleTapped = true;
        _doubleTapTimer.ResetTimer();
        _tapCounter = 0;
    }
    if(_doubleTapTimer.TimeIsUp())
    {
#ifdef SERIAL_DEBUG
        Serial.println("Button TimeIsup");
#endif
        _doubleTapTimer.ResetTimer();
        _tapCounter = 0;
    }
}

void handleWateringAlarm()
{
    if(_buttonDoubleTapped && _alarmOK && _clockOK)
    {
        alarm.activateAlarm(!alarm.isAlarmOn());
#ifdef SERIAL_DEBUG
        if(alarm.isAlarmOn())
        {
            Serial.print("Alarm ON\n");
        }
        else
        {
            Serial.print("Alarm OFF\n");
        }
#endif
        //digitalWrite(_buttonLED, alarm.isAlarmOn());
    }
}

void handleWatering()
{

    if(!_wateringStarted)
    {
        if(_clockOK && _alarmOK && alarm.isAlarmActive())
        {
            _wateringStarted = true;
            alarm.shutDownAlarm();
#ifdef SERIAL_DEBUG
            Serial.println("Alarm Active");
#endif
        }
        else if(_forceWateringBtn.isButtonPressed())
        {
            _wateringStarted = true;
#ifdef SERIAL_DEBUG
            Serial.println("Forced watering started");
#endif
        }
    }
    if(_wateringStarted)
    {
        if(_wateringState == 0)
        {
            openValve(_mainValve);
            openValve(_valve[_wateringState]);
            _wateringState++;
            _valveOnTimer.ResetTimer();
            _valveOnTimer.Start();
        }
        else if((_wateringState <= 8) && (_valveOnTimer.TimeIsUp()))
        {
            openValve(_valve[_wateringState]);
            _wateringState++;
            _valveTransTimer.ResetTimer();
            _valveTransTimer.Start();
            _valveOnTimer.ResetTimer();
            _valveOnTimer.Start();
        }
        else if((_wateringState == 9) && (_valveOnTimer.TimeIsUp()))
        {
            closeValve(8);
            _wateringState++;
            _valveTransTimer.ResetTimer();
            _valveTransTimer.Start();
            _valveOnTimer.ResetTimer();
        }

        if(_valveTransTimer.TimeIsUp())
        {
            closeValve(_valve[_wateringState - 2]);
            _valveTransTimer.ResetTimer();
            if(_wateringState == 10)
            {
                closeValve(_mainValve);
                _wateringStarted = false;
                //        alarm.activateAlarm(true);
                _wateringState = 0;
            }
        }
    }
}

void handleClock()
{

    if(_clockTimer.TimeIsUp())
    {
        if(RTC.read(clock))
        {
#ifdef SERIAL_DEBUG
            Serial.println("RTC read successfully");
#endif
            if(clock.Year > (2015-1970))
                _clockOK = true;
            else
                _clockOK = false;
        }
        else
        {
            _clockOK = false;
#ifdef SERIAL_DEBUG
            if(RTC.chipPresent())
            {
                Serial.println("The DS1307 is stopped.  Please run the SetTime");
                Serial.println("example to initialize the time and begin running.");
                Serial.println();
            }
            else
            {
                Serial.println("DS1307 read error!  Please check the circuitry.");
                Serial.println();
            }
#endif
        }

        _clockTimer.ResetTimer();
        _clockTimer.Start();
        alarm.update(clock.Hour, clock.Minute);
    }
}

void blinkButtonLED()
{
    static Timer turnON(0, 500);
    static Timer turnOFF(0, 500);
    if(!alarm.isAlarmOn())
    {
        _blinkButtonLED = true;
    }
    else
    {
        _blinkButtonLED = false;
    }

    if(!turnON.IsTimerStarted() && !turnOFF.IsTimerStarted() && _blinkButtonLED)
        turnON.Start();
    if(turnON.TimeIsUp())
    {
        digitalWrite(_buttonLED, true);
#ifdef SERIAL_DEBUG
        Serial.println("Start button LED ON");
#endif
        turnON.ResetTimer();
        turnOFF.Start();
    }
    if(turnOFF.TimeIsUp())
    {
        digitalWrite(_buttonLED, false);
#ifdef SERIAL_DEBUG
        Serial.println("Start button LED OFF");
#endif
        turnOFF.ResetTimer();
    }
    if(!_blinkButtonLED)
        digitalWrite(_buttonLED, true);
}

void handleSerial()
{
    static int ii = 0;

    while(Serial.available())
    {

        char inChar = (char)Serial.read();
        _dataRec[ii] = inChar;
        ii++;

        if(inChar == '\n')
        {
            _stringComplete = true;
            break;
        }
    }
    if(_stringComplete)
    {
        if((_dataRec[0] == 'h') && (_dataRec[1] == 'e') && (_dataRec[2] == 'l') && (_dataRec[3] == 'p'))
        {
            Serial.println("Examples of valid commands:");
            Serial.println("s a 23 00 #set alarm to 23:00");
            Serial.println("g a #get alarm currently set");
            Serial.println("s t 22:01 16.02.30 #set time to 22:01 2016-02-30");
            Serial.println("g t #get time currently set");
            Serial.println("s v o 32 #set valve open time to 32 seconds");
            Serial.println("s v t 4 #set valve transition time to 4 seconds");
            Serial.println("");
        }
        else
        {
            if(_dataRec[0] == 's')
            {
                _set = true;
            }
            else if(_dataRec[0] == 'g')
            {
                _get = true;
            }
            else
            {
                Serial.print("Command error\n");
            }

            if(_dataRec[2] == 't')
            {
                _time = true;
            }
            else if(_dataRec[2] == 'a')
            {
                _alarm = true;
            }
            else if(_dataRec[2] == 'v' && _dataRec[4] == 'o')
            {
                _valveOnTime = true;
            }
            else if(_dataRec[2] == 'v' && _dataRec[4] == 't')
            {
                _valveTransTime = true;
            }
            else
            {
                Serial.print("Command error\n");
            }

            if(_set && _valveOnTime)
            {
                int intValue = 0;
                char charValue[3];
                charValue[0] = _dataRec[6];
                charValue[1] = _dataRec[7];
                intValue = atoi(charValue);
                _valveOnTimer.ChangeTimeout(intValue, 0);
                SaveValveOnTimeEEPROM(intValue);
                _set = false;
                _valveOnTime = false;
            }
            else if(_set && _valveTransTime)
            {
                int intValue = 0;
                char charValue[3];
                charValue[0] = _dataRec[6];
                charValue[1] = _dataRec[7];
                intValue = atoi(charValue);
                _valveTransTimer.ChangeTimeout(intValue, 0);
                SaveValvTransTimeEEPROM(intValue);
                _set = false;
                _valveTransTime = false;
            }
            else if(_set && (_time || _alarm))
            {
                int iHour = 0;
                int iMinute = 0;
                int iYear = 0;
                int iMonth = 0;
                int iDay = 0;

                char tmpMin[3];
                tmpMin[0] = _dataRec[7];
                tmpMin[1] = _dataRec[8];
                char tmpHour[3];
                tmpHour[0] = _dataRec[4];
                tmpHour[1] = _dataRec[5];
                char tmpYear[3];
                tmpYear[0] = _dataRec[10];
                tmpYear[1] = _dataRec[11];
                char tmpMonth[3];
                tmpMonth[0] = _dataRec[13];
                tmpMonth[1] = _dataRec[14];
                char tmpDay[3];
                tmpDay[0] = _dataRec[16];
                tmpDay[1] = _dataRec[17];

                iHour = atoi(tmpHour);
                iMinute = atoi(tmpMin);
                iYear = atoi(tmpYear);
                iMonth = atoi(tmpMonth);
                iDay = atoi(tmpDay);

                if(((iHour >= 0) && (iHour <= 23)) && ((iMinute >= 0) && (iMinute <= 59)))

                {
                    if(_time)
                    {
                        if(((iYear >= 0) && (iYear <= 99)) && ((iMonth >= 1) && (iMonth <= 12)) && ((iDay >= 1) && (iDay <= 31)))
                        {
                            clock.Hour = iHour;
                            clock.Minute = iMinute;
                            clock.Year = (iYear + 2000) - 1970;
                            clock.Month = iMonth;
                            clock.Day = iDay;

                            RTC.write(clock);
                        }
                    }
                    else if(_alarm)
                    {
                        _alarmOK = true;
                        alarm.setAlarm(iHour, iMinute);
                        SaveAlarmEEPROM(iHour,iMinute);
                    }
                }
                _set = false;
                _time = false;
                _alarm = false;
            }
            else if(_get && _time)
            {
                Serial.print("Get Time: ");
                Serial.print(zeroPad(clock.Hour,2));
                Serial.print(":");
                Serial.print(zeroPad(clock.Minute,2));
                Serial.print(":");
                Serial.print(zeroPad(clock.Second,2));

                Serial.print(" ");
                Serial.print(zeroPad(1970+clock.Year,4));
                Serial.print("-");
                Serial.print(zeroPad(clock.Month,2));
                Serial.print("-");
                Serial.print(zeroPad(clock.Day,2));
                Serial.println();
                _get = false;
                _time = false;
            }
            else if(_get && _alarm)
            {
                Serial.print("Get Alarm: ");
                Serial.print(zeroPad(alarm.getAlarm("hour"),2));
                Serial.print(":");
                Serial.print(zeroPad(alarm.getAlarm("minute"),2));
                Serial.println();
                _get = false;
                _alarm = false;
            }
            else if(_get && _valveOnTime)
            {
                Serial.print("Get ValveOnTime: ");
                Serial.print(_valveOnTimer.GetTimeout() / 1000);
                Serial.println();
                _get = false;
                _valveOnTime = false;
            }
            else if(_get && _valveTransTime)
            {
                Serial.print("Get ValveTransTime: ");
                Serial.print(_valveTransTimer.GetTimeout() / 1000);
                Serial.println();
                _get = false;
                _valveTransTime = false;
            }

        }
        _stringComplete = false;
        ii = 0;
    }
}

void loop()
{
    _forceWateringBtn.update();
    _changeAlarmBtn.update();

    blinkButtonLED();
    handleDoubleTapp();
    handleSerial();
    handleClock();
    handleWateringAlarm();
    handleWatering();
}
