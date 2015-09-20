/************************************************************/
/*    FILE: Gps.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Thomas Le Mezo
/*    DATE: 2015
/************************************************************/

#ifndef Gps_HEADER
#define Gps_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#include <string>
#include <iostream>
#include <boost/asio.hpp>

#include <nmea/info.h>
#include <nmea/context.h>
#include <nmea/parser.h>
#include <nmea/gmath.h>

#include <math.h>

class Gps : public AppCastingMOOSApp
{
  public:
    Gps();
    ~Gps();
    void quit() {};

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
    boost::system::error_code m_error;

  private: // State variables
    char* m_trame;

    nmeaINFO    m_info;
    nmeaPARSER  m_parser;
    nmeaPOS     m_dpos;

    boost::asio::streambuf m_buffer;

    double m_lon, m_lat;
    double m_speed, m_heading;

    int m_sig, m_fix;
};

#endif
