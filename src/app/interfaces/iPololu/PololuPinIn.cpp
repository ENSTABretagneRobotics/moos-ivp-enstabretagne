/************************************************************/
/*    FILE: PololuPinIn.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <string>
#include <iostream>
#include <stdio.h>
#include "PololuPinIn.h"

using namespace std;

PololuPinIn::PololuPinIn(int pin_number) : PololuPin(pin_number)
{
  m_value = 0.;
}

void PololuPinIn::setValue(double value)
{
  m_value = value;
}

double PololuPinIn::getValue()
{
  return m_value;
}