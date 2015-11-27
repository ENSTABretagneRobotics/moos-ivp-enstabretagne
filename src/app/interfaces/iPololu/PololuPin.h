/************************************************************/
/*    FILE: PololuPin.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef PololuPin_HEADER
#define PololuPin_HEADER

//#include <QObject>

class PololuPin
{
  public:
    PololuPin(int pin_number);
    int getPinNumber();

  protected:
    int m_pin_number;
};

#endif 