#!/bin/bash
nsplug ground_regulation.moos targ_ground_regulation.moos --path=../../common -f
pAntler targ_ground_regulation.moos >& /dev/null &

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