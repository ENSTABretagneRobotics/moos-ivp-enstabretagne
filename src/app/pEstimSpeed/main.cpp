/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: main.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <string>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "MBUtils.h"
#include "documentation/MOOSAppDocumentation.h"
#include "ColorParse.h"
#include "EstimSpeed.h"
#include "EstimSpeed_Info.h"

using namespace std;

EstimSpeed *objEstimSpeed;

void kill_handler(int s);

int main(int argc, char *argv[]) {
  objEstimSpeed = new EstimSpeed;

  string mission_file;
  string run_command = argv[0];
  xmldoc::MOOSAppDocumentation documentation(argv[0]);

  for (int i = 1; i < argc; i++) {
    string argi = argv[i];
    if ((argi == "-v") || (argi == "--version") || (argi == "-version"))
        showReleaseInfoAndExit();
    else if ((argi == "-e") || (argi == "--example") || (argi == "-example"))
        showExampleConfigAndExit();
    else if ((argi == "-h") || (argi == "--help") || (argi == "-help"))
        showHelpAndExit();
    else if ((argi == "-i") || (argi == "--interface"))
        showInterfaceAndExit();
    else if (strEnds(argi, ".moos") || strEnds(argi, ".moos++"))
        mission_file = argv[i];
    else if (strBegins(argi, "--alias="))
        run_command = argi.substr(8);
    else if (i == 2)
        run_command = argi;
  }

  if (mission_file == "")
    showHelpAndExit();

  cout << termColor("green");
  cout << "pEstimSpeed launching as " << run_command << endl;
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

  objEstimSpeed->Run(run_command.c_str(), mission_file.c_str());

  return (0);
}

void kill_handler(int s) {
    delete objEstimSpeed;
    exit(0);
}