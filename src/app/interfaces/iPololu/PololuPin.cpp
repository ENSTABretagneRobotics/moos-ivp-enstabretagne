/************************************************************/
/*    FILE: PololuPin.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include "PololuPin.h"

PololuPin::PololuPin(int pin_number)
{
  m_pin_number = pin_number;
}

int PololuPin::getPinNumber()
{
  return m_pin_number;
}
