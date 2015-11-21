/************************************************************/
/*    FILE: PololuPinIn.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef PololuPinIn_HEADER
#define PololuPinIn_HEADER

//#include <QObject>
#include "PololuPin.h"

class PololuPinIn : public PololuPin
{
  public:
    PololuPinIn(int pin_number);
    ~PololuPinIn() {};

    double getValue();
    double getCoeff();
    double getThreshold();
    std::string getUnit();
    std::string getWarningMessage();
    bool testThresholdRequired();

    void setValue(double value);
    void setCoeff(double coeff);
    void setThreshold(double threshold);
    void setUnit(std::string unit);
    void setWarningMessage(std::string message);
    void requireTestThreshold(bool test);

  protected:
  	double m_value, m_coeff, m_threshold;
  	std::string m_unit, m_warning_message;
  	bool m_test_threshold;
};

#endif 