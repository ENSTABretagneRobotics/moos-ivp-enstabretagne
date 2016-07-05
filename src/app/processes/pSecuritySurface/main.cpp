/************************************************************/
/*    FILE: main.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#include <string>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "MBUtils.h"
#include "documentation/MOOSAppDocumentation.h"
#include "ColorParse.h"
#include "SecuritySurface.h"

using namespace std;

SecuritySurface *objSecuritySurface;

void kill_handler(int s);

int main(int argc, char *argv[])
{
  objSecuritySurface = new SecuritySurface;

  string mission_file;
  string run_command = argv[0];
  xmldoc::MOOSAppDocumentation documentation(argv[0]);

  for(int i=1; i<argc; i++)
  {
    string argi = argv[i];
    if((argi=="-v") || (argi=="--version") || (argi=="-version"))
      documentation.showReleaseInfoAndExit();
    else if((argi=="-e") || (argi=="--example") || (argi=="-example"))
      documentation.showExampleConfigAndExit();
    else if((argi == "-h") || (argi == "--help") || (argi=="-help"))
      documentation.showHelpAndExit();
    else if((argi == "-i") || (argi == "--interface"))
      documentation.showInterfaceAndExit();
    else if(strEnds(argi, ".moos") || strEnds(argi, ".moos++"))
      mission_file = argv[i];
    else if(strBegins(argi, "--alias="))
      run_command = argi.substr(8);
    else if(i==2)
      run_command = argi;
  }

  if(mission_file == "")
    documentation.showHelpAndExit();

  cout << termColor("green");
  cout << "pSecuritySurface launching as " << run_command << endl;
  cout << termColor() << endl;

  // To catch the kill event
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = kill_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGTERM, &sigIntHandler, NULL);
  sigaction(SIGKILL, &sigIntHandler, NULL);
  sigaction(SIGQUIT, &sigIntHandler, NULL);
  sigaction(SIGINT, &sigIntHandler, NULL);

  objSecuritySurface->Run(run_command.c_str(), mission_file.c_str());

  return(0);
}

void kill_handler(int s)
{
  delete objSecuritySurface;
  exit(0);
}
