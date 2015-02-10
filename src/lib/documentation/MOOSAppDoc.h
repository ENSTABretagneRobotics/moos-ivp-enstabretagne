/****************************************************************/
/*    FILE: MOOSAppDoc.h
/*    ORGN: ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/****************************************************************/

#ifndef MOOSAPPDOC_INFO_HEADER
#define MOOSAPPDOC_INFO_HEADER

#include <string>

namespace ensta
{
  void showSynopsis(std::string moosapp_name);
  void showHelpAndExit(std::string moosapp_name);
  void showExampleConfigAndExit(std::string moosapp_name);
  void showInterfaceAndExit(std::string moosapp_name);
  void showReleaseInfoAndExit(std::string moosapp_name);
}

#endif