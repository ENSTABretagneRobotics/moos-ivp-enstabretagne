#!/bin/bash
nsplug viability_vehicle.moos targ_viability_vehicle.moos --path=../common -f
pAntler targ_viability_vehicle.moos >& /dev/null &

printf "Mission started.\n"

ANSWER="0"
while [ "${ANSWER}" != "q" ]; do
  printf "Hit (q) to Exit and Kill:\n"
  printf "> "
  read ANSWER
done

mykill >& /dev/null &
ktm >& /dev/null &
printf "Done killing processes...\n"
rm -f targ_*