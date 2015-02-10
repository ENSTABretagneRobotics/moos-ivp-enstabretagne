/****************************************************************/
/*    FILE: Joystick_Info.cpp
/*    ORGN: ENSTA Bretagne
/*    AUTH: Vincent Drevelle, Simon Rohou
/*    DATE: 2015
/****************************************************************/

#include <cstdlib>
#include <iostream>
#include "Joystick_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("  The uJoystick application is used to update MOOS variables by ");
  blk("  handling of a joystick.                                       ");
  blk("  Each updated MOOS Variable is linked to an axis or a button   ");
  blk("  of the joystick. The corresponding MOOS Variable name has to  ");
  blk("  be set in the .moos configuration file.                       ");
}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: uJoystick file.moos [OPTIONS]                            ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch uJoystick with the given process name              ");
  blk("      rather than uJoystick.                                    ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of uJoystick.                 ");
  blk("                                                                ");
  blk("Note: If argv[2] does not otherwise match a known option,       ");
  blk("      then it will be interpreted as a run alias. This is       ");
  blk("      to support pAntler launching conventions.                 ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showExampleConfigAndExit

void showExampleConfigAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uJoystick Example MOOS Configuration                            ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = uJoystick                                       ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  DEVICE_NAME = /dev/input/js0                                  ");
  blk("                                                                ");
  blk("  INCREMENT = false                                             ");
  blk("  SCALE = 100                                                   ");
  blk("                                                                ");
  blk("  AXIS = 0                                                      ");
  blk("  SCALE_RATIO = 0.5                                             ");
  blk("  MOOS_DEST_VAR = DESIRED_SLIDE                                 ");
  blk("                                                                ");
  blk("  AXIS = 1                                                      ");
  blk("  SCALE_RATIO = 0.5                                             ");
  blk("  MOOS_DEST_VAR = DESIRED_THRUST                                ");
  blk("                                                                ");
  blk("  AXIS = 2                                                      ");
  blk("  SCALE_RATIO = 0.5                                             ");
  blk("  MOOS_DEST_VAR = DESIRED_RUDDER                                ");
  blk("                                                                ");
  blk("  AXIS = 3                                                      ");
  blk("  SCALE_RATIO = 0.5                                             ");
  blk("  MOOS_DEST_VAR = DESIRED_ELEVATOR                              ");
  blk("                                                                ");
  blk("  BUTTON = 1                                                    ");
  blk("  SCALE_RATIO = 0.5                                             ");
  blk("  MOOS_DEST_VAR = PUSHED_BUTTON_1                               ");
  blk("                                                                ");
  blk("  BUTTON = 1                                                    ");
  blk("  SCALE_RATIO = 0.5                                             ");
  blk("  MOOS_DEST_VAR = PUSHED_BUTTON_2                               ");
  blk("}                                                               ");
  blk("                                                                ");
  exit(0);
}


//----------------------------------------------------------------
// Procedure: showInterfaceAndExit

void showInterfaceAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uJoystick INTERFACE                                             ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  No subscriptions.                                             ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  No publications.                                              ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("uJoystick", "gpl");
  exit(0);
}
