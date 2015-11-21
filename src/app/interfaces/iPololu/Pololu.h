/************************************************************/
/*    FILE: Pololu.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

class Pololu;

#ifndef Pololu_HEADER
#define Pololu_HEADER

//#include <QObject>

class Pololu/* : public QObject*/
{
  /*Q_OBJECT*/

  public:
    Pololu(std::string device_name);
    ~Pololu();
    
    bool isReady();
    bool isReady(std::string &error_message);

  /*public slots:*/
    bool setTarget(unsigned char channel, unsigned short target);
    bool getValue(unsigned char channel, int &returned_value);

  protected:
    void setErrorMessage(std::string message);
    
  protected:
    int m_pololu;
    bool m_error;
    std::string m_error_message;
};

#endif 