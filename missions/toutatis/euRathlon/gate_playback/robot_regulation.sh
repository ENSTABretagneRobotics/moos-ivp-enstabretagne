#!/bin/bash
nsplug robot_regulation.moos targ_robot_regulation.moos --path=../../common:../../../../data/maps -f
pAntler targ_robot_regulation.moos >& /dev/null &

printf "Mission started.\n"

ANSWER="0"
while [ "${ANSWER}" != "q" ]; do
  printf "Hit (q) to Exit and Kill:\n"
  printf "> "
  read ANSWER
done

enstabretagnekill >& /dev/null &
printf "Done killing processes...\n"
rm targ_*