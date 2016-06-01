#ifndef ToggleButton_h
#define ToggleButton_h
class ToggleButton
{
 public:
    ToggleButton(int pin) :
            _pin(pin),
            _isDown(false),
            _isChanged(false),
            _isPressed(false),
            _isReleased(false),
            _inverted(false),
            _pause(false),
            _delayUpdate(0),
            _lastMillis(0)
    {
//      pinMode(_pin, INPUT);
    }
    ;
    void update()
    {
        unsigned long currMillis = millis();
        bool currState = digitalRead(_pin) ^ _inverted;
        if(((currState && _isDown) || (!currState && !_isDown)))
        {
            _isChanged = false;
            _isPressed = false;
            _isReleased = false;
        }
        else if(currState && !_isDown)
        {
            _isPressed = true;
            _isReleased = false;
            _isChanged = true;
        }
        else if(!currState && _isDown)
        {
            _isPressed = false;
            _isReleased = true;
            _isChanged = true;
        }

        if((_delayUpdate > 0) && _isPressed)
        {
            if((currMillis - _lastMillis) < _delayUpdate)
            {
                _pause = true;
                _isPressed = false;
                _isChanged = false;
            }
            else
            {
                _pause = false;
            }
        }
        else
        {
            _pause = false;
        }

        if(!_pause)
        {
            _lastMillis = currMillis;
            _isDown = currState;
        }

    }
    ;
    bool isButtonDown()
    {
        return _isDown;
    }
    ;
    void setButtonPressedTime(unsigned long millis_)
    {
        _delayUpdate = millis_;
    }
    ;
    void setInvertedButton()
    {
        _inverted = true;
    }
    ;
    bool isButtonChanged()
    {
        return _isChanged;
    }
    ;
    bool isButtonPressed()
    {
        return _isPressed;
    }
    ;
    bool isButtonReleased()
    {
        return _isReleased;
    }
    ;

 private:
    int _pin;
    bool _isDown;
    bool _isChanged;
    bool _isPressed;
    bool _isReleased;
    bool _inverted;
    bool _pause;
    unsigned long _delayUpdate;
    unsigned long _lastMillis;
};
#endif
