/************************************************************/
/*    FILE: PololuPinIn.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef PololuPinOut_HEADER
#define PololuPinOut_HEADER

#include <QObject>
#include "PololuPin.h"

class PololuPinIn : public PololuPin
{
  public:
    PololuPinIn(int pin_number);
    ~PololuPinIn() {};

  protected:
    
};

#endif 