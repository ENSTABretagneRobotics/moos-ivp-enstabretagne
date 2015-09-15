/************************************************************/
/*    FILE: Nuc.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <string>
#include <iostream>
#include <stdio.h>
#include <string>
#include "Nuc.h"

using namespace std;

Nuc::Nuc()
{

}

double Nuc::getTemperature()
{
  string str_t = exec("cat /sys/class/thermal/thermal_zone0/temp");
  return atof(str_t.c_str()) / 1000.;
}

string Nuc::exec(string cmd)
{
  FILE* pipe = popen(cmd.c_str(), "r");
  if(!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while(!feof(pipe))
  {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}