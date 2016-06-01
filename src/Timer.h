#ifndef TIMER_H_
#define TIMER_H_

class Timer {
public:
  Timer(unsigned long seconds, unsigned long milliseconds)
	: MilliSecondTimer(seconds*1000+milliseconds)
	, StartTime(0)
	, StopTime(0)
	, TimerStarted(false)
  {};

void Start()
{
	StartTime = millis();
	TimerStarted = true;
};
void Stop()
{
	if(TimerStarted)
	{
		StopTime = millis();
		TimerStarted = false;
	}
};
bool TimeIsUp()
{
	if(!TimerStarted)
		return false;
	if(MillisecondsElapsed()>MilliSecondTimer)
		return true;
	else
		return false;
};
unsigned long MillisecondsElapsed()
{
	if(!TimerStarted)
		return 0;
	unsigned long timeElap = millis() - StartTime;
	return timeElap;
};
unsigned long MillisecondsLeft()
{
	if(!TimerStarted)
		return MilliSecondTimer;
	unsigned long timeElap = MillisecondsElapsed();
	if(timeElap>MilliSecondTimer)
		return 0;
	else
		return MilliSecondTimer-timeElap;
};

void ResetTimer()
{
	StartTime = 0;
	StopTime = 0;
	TimerStarted = false;
};

bool IsTimerStarted()
{
	return TimerStarted;
};

void ChangeTimeout(unsigned long seconds, unsigned long milliseconds)
{
  MilliSecondTimer = seconds*1000+milliseconds;
}
unsigned long GetTimeout()
{
   return MilliSecondTimer; 
}
private:
	unsigned long MilliSecondTimer;
	unsigned long StartTime;
	unsigned long StopTime;
	bool TimerStarted;
};

#endif /* TIMER_H_ */



