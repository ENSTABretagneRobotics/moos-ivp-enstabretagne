/************************************************************/
/*    FILE: PololuPin.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <string>
#include <iostream>
#include <stdio.h>
#include "PololuPin.h"

using namespace std;

PololuPin::PololuPin(int pin_number)
{
  m_pin_number = pin_number;
}

int PololuPin::getPinNumber()
{
  return m_pin_number;
}