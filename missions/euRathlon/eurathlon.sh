#!/bin/bash
nsplug eurathlon.moos targ_eurathlon.moos --path=../common -f

printf "\n\n\t"

ANSWER="0"
while [ "${ANSWER}" != "2" -a "${ANSWER}" != "q" ]; do
  printf "Hit (q) to Exit and Kill \n"
  printf "> "
  read ANSWER
done

toutatiskill >& /dev/null &
printf "Done killing processes.\n"
rm targ_*