#!/bin/bash
nsplug gate_timers.moos targ_gate_timers.moos --path=../../common -f
pAntler targ_gate_timers.moos >& /dev/null &

printf "Mission started.\n"

ANSWER="0"
while [ "${ANSWER}" != "q" ]; do
  printf "Hit (q) to Exit and Kill:\n"
  printf "> "
  read ANSWER
done

toutatiskill >& /dev/null &
printf "Done killing processes...\n"
rm targ_*