#include <cstdlib>
#include <iostream>
#include "Sonar_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("  The pSonar application is used for               ");
  blk("                                                                ");
  blk("                                                                ");
  blk("                                                                ");
  blk("                                                                ");
}

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: pSonar file.moos [OPTIONS]                   ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch pSonar with the given process name         ");
  blk("      rather than pSonar.                           ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of pSonar.        ");
  blk("                                                                ");
  blk("Note: If argv[2] does not otherwise match a known option,       ");
  blk("      then it will be interpreted as a run alias. This is       ");
  blk("      to support pAntler launching conventions.                 ");
  blk("                                                                ");
  exit(0);
}

void showExampleConfigAndExit()
{
  /*
    AppTick   = 4
  CommsTick = 4

  SERIAL_PORT_NAME = /dev/ttyUSB4
  RANGE = 1 //IN METERS
  NBINS = 100 //NUMBER OF SAMPLES PER SCANLINE
  ANGLESTEP = 1.8 //ANGULAR RESOLUTION (MUST BE A MULTIPLE OF 0.09)
  GAIN = 45 // MUST BE BETWEEN 0 AND 100 (PERCENTAGE)
  CONTINUOUS = true //TRUE FOR 360  ROTATION. SET RIGHT AND LEFT LIMIT IF FALSE 
  //LEFTLIMIT = 70.0 //IN DEGREES. ONLY WHEN CONTINUOUS IS FALSE
  //RIGHTLIMIT = 110.0 //IN DEGREES. ONLY WHEN CONTINUOUS IS FALSE
  */
  blk("                                                                ");
  blu("=============================================================== ");
  blu("pSonar Example MOOS Configuration                   ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = pSonar                              ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  SERIAL_PORT_NAME = /dev/ttyUSB4                               ");
  blk("  RANGE = 1 //IN METERS                                         ");
  blk("  NBINS = 100 //NUMBER OF SAMPLES PER SCANLINE                  ");
  blk("  ANGLESTEP = 1.8 //ANGULAR RESOLUTION (MUST BE A MULTIPLE OF 0.09)");
  blk("  GAIN = 45 // MUST BE BETWEEN 0 AND 100 (PERCENTAGE)           ");
  blk("  CONTINUOUS = true //TRUE FOR 360  ROTATION. SET RIGHT AND LEFT LIMIT IF FALSE");
  blk("  //LEFTLIMIT = 70.0 //IN DEGREES. ONLY WHEN CONTINUOUS IS FALSE");
  blk("  //RIGHTLIMIT = 110.0 //IN DEGREES. ONLY WHEN CONTINUOUS IS FALSE");
  blk("}                                                               ");
  blk("                                                                ");
  exit(0);
}

void showInterfaceAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("pSonar INTERFACE                                    ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  SONAR_PARAMS = Range=,Gain=,Continuous=   ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  SONAR_CONNECTED = true/false    ");
  blk("  SONAR_RAW_DATA = bearing=,ad_interval=,scanline=     ");
  blk("  SONAR_DISTANCE = bearing=,distance=     ");
  blk("                                                                ");
  exit(0);
}

void showReleaseInfoAndExit()
{
  showReleaseInfo("pSonar", "lgpl");
  exit(0);
}

