/************************************************************/
/*    FILE: Pololu.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "Pololu.h"

#ifdef _WIN32
  #define O_NOCTTY 0
#else
  #include <termios.h>
#endif

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
  #include <windows.h>
  inline void delay(unsigned long ms)
  {
    Sleep(ms);
  }
#else /* POSIX */
  #include <unistd.h>
  inline void delay(unsigned long ms)
  {
    usleep(ms * 1000);
  }
#endif 

#define HIGH_LEVEL 7000
#define LOW_LEVEL  5000

using namespace std;

Pololu::Pololu(string device_name)
{
  bool init_success = true;
  m_error = false;
  m_error_message = "";

  const char * device = device_name.c_str();

  // Open the Maestro's virtual COM port
  m_pololu = open(device, O_RDWR | O_NOCTTY);
  if(m_pololu == -1)
  {
    perror(device);
    setErrorMessage("error (loading " + string(device) + ")");
    cout << "Pololu: " << m_error_message << endl;
    init_success = false;
  }

  #ifdef _WIN32
    _setmode(m_pololu, _O_BINARY);
  #else
    struct termios options;
    tcgetattr(m_pololu, &options);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_oflag &= ~(ONLCR | OCRNL);
    tcsetattr(m_pololu, TCSANOW, &options);
  #endif

  init_success &= setAllThrustersValue(0.);

  if(!init_success)
    bipError();

  else
    bipOnStartUp();
}

Pololu::~Pololu()
{
  close(m_pololu);
}

bool Pololu::isReady(string &error_message)
{
  error_message = m_error_message;
  return !m_error;
}

bool Pololu::turnOnRelay(int id, bool turned_on)
{
  return setTarget(id, turned_on ? HIGH_LEVEL : LOW_LEVEL);
}

bool Pololu::turnOnBistableRelay(int id_on, int id_off, bool turned_on)
{
  bool success_on, success;

  if(!turnOnRelay(id_on, turned_on) || !turnOnRelay(id_off, !turned_on))
  {
    bipError();
    return false;
  }

  delay(50);

  if(!turnOnRelay(id_on, false) || !turnOnRelay(id_off, false))
  {
    bipError();
    return false;
  }

  return true;
}

bool Pololu::setThrusterValue(int id, double value)
{
  // value in [-1.0;1.0]
  value = min(1., value);
  value = max(-1., value);
  double mean = (LOW_LEVEL + HIGH_LEVEL) / 2;
  double radius = (HIGH_LEVEL - LOW_LEVEL) / 2;
  return setTarget(id, mean + radius * value);
}

bool Pololu::setLeftThrusterValue(double value)
{
  return setThrusterValue(22, value);
}

bool Pololu::setRightThrusterValue(double value)
{
  return setThrusterValue(23, value);
}

bool Pololu::setVerticalThrusterValue(double value)
{
  return setThrusterValue(21, value);
}

bool Pololu::setAllThrustersValue(double value)
{
  return setLeftThrusterValue(value)
         && setRightThrusterValue(value)
         && setVerticalThrusterValue(value);
}

void Pololu::buzzOn()
{
  setTarget(13, HIGH_LEVEL);
}

void Pololu::buzzOff()
{
  setTarget(13, LOW_LEVEL);
}

void Pololu::bipOnStartUp()
{
  for(int i = 0 ; i < 3 ; i++)
  {
    buzzOn();
    delay(80);
    buzzOff();
    delay(50);
  }
}

void Pololu::bipOnExit()
{
  for(int i = 0 ; i < 2 ; i++)
  {
    buzzOn();
    delay(80);
    buzzOff();
    delay(50);
  }
}

void Pololu::bipError()
{
  for(int i = 0 ; i < 2 ; i++)
  {
    buzzOn();
    delay(2000);
    buzzOff();
    delay(1000);
  }
}

bool Pololu::emitBips(int bip_number)
{
  for(int i = 0 ; i < bip_number ; i++)
  {
    buzzOn();
    delay(80);
    buzzOff();
    delay(50);
  }
}

bool Pololu::setTarget(unsigned char channel, unsigned short target)
{
  // Sets the target of a Maestro channel.
  // See the "Serial Servo Commands" section of the user's guide.
  // The units of 'target' are quarter-microseconds.
  unsigned char command[] = {0x84, channel, target & 0x7F, target >> 7 & 0x7F};

  if(write(m_pololu, command, sizeof(command)) == -1)
  {
    setErrorMessage("error (writing)");
    perror(m_error_message.c_str());
    bipError();
    return false;
  }

  return true;
}

void Pololu::setErrorMessage(string message)
{
  if(m_error_message == "")
    m_error_message = message;
  m_error = true;
}