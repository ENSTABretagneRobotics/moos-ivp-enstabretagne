#!/bin/bash
source "../../../scripts/source.sh"
nsplug robot.moos targ_robot.moos --path=../common:../../../data/maps -f
pAntler targ_robot.moos >& /dev/null &

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
