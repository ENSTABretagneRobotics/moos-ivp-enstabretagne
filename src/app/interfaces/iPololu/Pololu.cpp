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
    //perror(device);
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
}

Pololu::~Pololu()
{
  close(m_pololu);
}

bool Pololu::isReady()
{
  return !m_error;
}

bool Pololu::isReady(string &error_message)
{
  error_message = m_error_message;
  return isReady();
}

bool Pololu::setTarget(unsigned char channel, unsigned short target)
{
  if(!isReady())
    return false;

  target *= 4; // quarter microseconds -> microseconds

  // Sets the target of a Maestro channel.
  // See the "Serial Servo Commands" section of the user's guide.
  // The units of 'target' are microseconds.
  unsigned char command[] = {0x84, channel, target & 0x7F, target >> 7 & 0x7F};

  if(write(m_pololu, command, sizeof(command)) == -1)
  {
    setErrorMessage("error (writing)");
    //perror(m_error_message.c_str());
    return false;
  }

  return true;
}

bool Pololu::getValue(unsigned char channel, int &returned_value)
{
  if(!isReady())
    return false;

  // Gets the position of a Maestro channel.
  // See the "Serial Servo Commands" section of the user's guide.
  unsigned char command[] = {0x90, channel};
  if(write(m_pololu, command, sizeof(command)) == -1)
  {
    setErrorMessage("error (writing)");
    //perror("error (writing)");
    return false;
  }
 
  unsigned char response[2];
  if(read(m_pololu, response,2) != 2)
  {
    setErrorMessage("error (reading)");
    //perror("error (reading)");
    return false;
  }
 
  returned_value = response[0] + 256*response[1];

  return true;
}

void Pololu::setErrorMessage(string message)
{
  if(m_error_message == "")
    m_error_message = message;
  m_error = true;
}