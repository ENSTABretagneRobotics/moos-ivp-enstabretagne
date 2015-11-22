/************************************************************/
/*    FILE: Nuc.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
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