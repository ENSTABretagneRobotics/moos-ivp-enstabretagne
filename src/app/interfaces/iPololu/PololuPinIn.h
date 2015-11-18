/************************************************************/
/*    FILE: PololuPinIn.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef PololuPinIn_HEADER
#define PololuPinIn_HEADER

#include <QObject>
#include "PololuPin.h"

class PololuPinIn : public PololuPin
{
  public:
    PololuPinIn(int pin_number);
    ~PololuPinIn() {};

    double getValue();
    void setValue(double value);

  protected:
  	double m_value;
};

#endif 