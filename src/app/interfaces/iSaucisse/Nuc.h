/************************************************************/
/*    FILE: Nuc.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef Nuc_HEADER
#define Nuc_HEADER

//#include <QObject>

class Nuc
{
  public:
    Nuc();
    ~Nuc() {};
    double getTemperature();

  protected:
    std::string exec(std::string cmd);
};

#endif 