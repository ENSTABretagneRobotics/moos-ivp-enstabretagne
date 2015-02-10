/************************************************************/
/*    FILE: main.cpp
/*    ORGN: ENSTA Bretagne
/*    AUTH: Vincent Drevelle, Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <string>
#include "MBUtils.h"
#include "documentation/MOOSAppDoc.h"
#include "ColorParse.h"
#include "Joystick.h"
#include "Joystick_Info.h"

using namespace std;

int main(int argc, char *argv[])
{
  string mission_file;
  string run_command = argv[0];

  for(int i=1; i<argc; i++) {
    string argi = argv[i];
    if((argi=="-v") || (argi=="--version") || (argi=="-version"))
      ensta::showReleaseInfoAndExit(argv[0]);
    else if((argi=="-e") || (argi=="--example") || (argi=="-example"))
      ensta::showExampleConfigAndExit(argv[0]);
    else if((argi == "-h") || (argi == "--help") || (argi=="-help"))
      ensta::showHelpAndExit(argv[0]);
    else if((argi == "-i") || (argi == "--interface"))
      ensta::showInterfaceAndExit(argv[0]);
    else if(strEnds(argi, ".moos") || strEnds(argi, ".moos++"))
      mission_file = argv[i];
    else if(strBegins(argi, "--alias="))
      run_command = argi.substr(8);
    else if(i==2)
      run_command = argi;
  }

  if(mission_file == "")
    ensta::showHelpAndExit(argv[0]);

  cout << termColor("green");
  cout << "uJoystick launching as " << run_command << endl;
  cout << termColor() << endl;

  Joystick Joystick;

  Joystick.Run(run_command.c_str(), mission_file.c_str());

  return(0);
}
