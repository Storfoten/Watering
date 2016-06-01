#ifndef Alarm_h
#define Alarm_h
class Alarm
{
 public:
    Alarm(int hour, int minute) :
            _alarmOn(false),
            _hour(hour),
            _minute(minute),
            _soundAlarm(false)
    {
    }
    ;
    void update(int currHour, int currMin)
    {
        if((currHour == _hour) && (currMin == _minute) && _alarmOn)
            _soundAlarm = true;
        else
            _soundAlarm = false;

    }
    ;
    void setAlarm(int hour, int minute)
    {
        if(hour > 23) hour = 0;
        if(minute > 59) minute = 0;
        _hour = hour;
        _minute = minute;
    }
    ;
    int getAlarm(String val)
    {
        if(val == "hour")
            return _hour;
        else if(val == "minute")
            return _minute;
        else
            return -1;
    }
    ;
    boolean isAlarmOn()
    {
        return _alarmOn;
    }
    ;
    boolean isAlarmActive()
    {
        return _soundAlarm;
    }
    ;
    void activateAlarm(boolean bol)
    {
        _alarmOn = bol;
    }
    ;
    void shutDownAlarm()
    {
        //     _alarmOn = false;
        _soundAlarm = false;
    }
    ;

 private:
    boolean _alarmOn;
    int _hour;
    int _minute;
    boolean _soundAlarm;

};
#endif
