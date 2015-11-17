/************************************************************/
/*    FILE: PololuPinOut.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <string>
#include <iostream>
#include <stdio.h>
#include "PololuPinOut.h"

using namespace std;

PololuPinOut::PololuPinOut(int pin_number) : PololuPin(pin_number)
{
  m_pwm_mini = 1200;
  m_pwm_zero = 1500;
  m_pwm_maxi = 1800;
  m_bilaterality = true;
  m_reversed = false;
}

int PololuPinOut::getPwmValue()
{
  return m_pwm;
}

int PololuPinOut::getPwmMini()
{
  return m_pwm_mini;
}

int PololuPinOut::getPwmZero()
{
  return m_pwm_zero;
}

int PololuPinOut::getPwmMaxi()
{
  return m_pwm_maxi;
}

bool PololuPinOut::isBilateral()
{
  return m_bilaterality;
}

bool PololuPinOut::isReversed()
{
  return m_reversed;
}

void PololuPinOut::setValue(double value)
{
  // value \in [-100;100]
  value /= 100.;

  if(m_bilaterality) // from -1 to 1
  {
    value = max(-1., min(1., value));

    if(m_reversed)
      value *= -1.;

    if(value < 0)
      m_pwm = (int)(m_pwm_zero + value * (m_pwm_zero - m_pwm_mini));

    else
      m_pwm = (int)(m_pwm_zero + value * (m_pwm_maxi - m_pwm_zero));
  }

  else // from 0 to 1
  {
    value = max(0., min(1., value));

    if(m_reversed)
      value = 1. - value;

    m_pwm = (int)(m_pwm_mini + value * (m_pwm_maxi - m_pwm_mini));
  }
}

void PololuPinOut::setPwmMini(int value)
{
  m_pwm_mini = value;
}

void PololuPinOut::setPwmZero(int value)
{
  m_pwm_zero = value;
}

void PololuPinOut::setPwmMaxi(int value)
{
  m_pwm_maxi = value;
}

void PololuPinOut::setBilaterality(bool mode)
{
  m_bilaterality = mode;
}

void PololuPinOut::reverse(bool mode)
{
  m_reversed = mode;
}