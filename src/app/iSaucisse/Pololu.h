/************************************************************/
/*    FILE: Pololu.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

class Pololu;

#ifndef Pololu_HEADER
#define Pololu_HEADER

#include <QObject>

class Pololu : public QObject
{
  Q_OBJECT

  public:
    Pololu(std::string device_name);
    ~Pololu();
    
    bool isReady(std::string &error_message);

  public slots:
    int turnOnRelay(int id, bool turned_on = true);
    int turnOnBistableRelay(int id_on, int id_off, bool turned_on = true);
    int emitBips(int bip_number = 1);
    int reset();
    int setLeftThrusterValue(double value);
    int setRightThrusterValue(double value);
    int setVerticalThrusterValue(double value);
    void bipOnStartUp();
    void bipError();

  protected:
    void buzzOn();
    void buzzOff();
    int setThrusterValue(int id, double value);
    int setTarget(unsigned char channel, unsigned short target);
    void setErrorMessage(std::string message);
    
  protected:
    int m_pololu;
    bool m_error;
    std::string m_error_message;
};

#endif 