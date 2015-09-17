/************************************************************/
/*    FILE: Gps2.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Thomas Le Mezo
/*    DATE: 2015
/************************************************************/

#ifndef Gps2_HEADER
#define Gps2_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#include <string>
#include <iostream>
#include <boost/asio.hpp>

#include <nmea/info.h>
#include <nmea/context.h>
#include <nmea/parser.h>
#include <nmea/gmath.h>

#include <math.h>

class Gps2 : public AppCastingMOOSApp
{
  public:
    Gps2();
    ~Gps2() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  private:
    bool Notify_GNSS(float *lat, float *lon);

  private: // Configuration variables
    std::string   m_uart_port;
    int           m_uart_baud_rate;

    boost::asio::io_service   m_io;
    boost::asio::serial_port  m_serial;

    double m_depth_invalid_threshold;

  private: // State variables
    char* m_trame;

    nmeaINFO    m_info;
    nmeaPARSER  m_parser;
    nmeaPOS     m_dpos;

    boost::asio::streambuf m_buffer;

    double m_depth;
    double m_lon, m_lat;
};

#endif 
