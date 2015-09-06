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
    
    int setTarget(unsigned char channel, unsigned short target);
    bool isReady(std::string &error_message);
    void setErrorMessage(std::string message);

  public slots:
    int turnOnRelay(int id, bool turned_on = true);
    int turnOnBistableRelay(int id_on, int id_off, bool turned_on = true);
    int setThrustersValue(int id, double value);
    void buzzOn();
    void buzzOff();
    void bipOnStartUp();
    void bipError();

  protected:
    int m_pololu;
    bool m_error;
    std::string m_error_message;
};

#endif 