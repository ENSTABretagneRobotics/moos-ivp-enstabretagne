/************************************************************/
/*    FILE: Pololu.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

class Pololu;

#ifndef Pololu_HEADER
#define Pololu_HEADER


#include <iostream>
#include <fcntl.h>

using namespace std;

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

class Pololu
{
  public:
    Pololu(std::string device_name);
    ~Pololu();
    
    bool isReady();
    bool isReady(std::string &error_message);
    bool setTarget(unsigned char channel, unsigned short target);
    bool getValue(unsigned char channel, int &returned_value);

  protected:
    void setErrorMessage(std::string message);
    int m_pololu;
    bool m_error;
    std::string m_error_message;
};

#endif
