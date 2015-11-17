/************************************************************/
/*    FILE: PololuPinOut.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef PololuPinOut_HEADER
#define PololuPinOut_HEADER

#include <QObject>
#include "PololuPin.h"

class PololuPinOut : public PololuPin
{
  public:
    PololuPinOut(int pin_number);
    ~PololuPinOut() {};

    int getPwmValue();
    int getPwmMini();
    int getPwmZero();
    int getPwmMaxi();
    bool isBilateral();
    bool isReversed();

    void setValue(double value);
    void setPwmMini(int value);
    void setPwmZero(int value);
    void setPwmMaxi(int value);
    void setBilaterality(bool mode);
    void reverse(bool mode);

  protected:
    int m_pwm, m_pwm_mini, m_pwm_zero, m_pwm_maxi;
    bool m_bilaterality, m_reversed;
};

#endif 