
export PATH=$PATH:~/moos-ivp-enstabretagne/bin
export PATH=$PATH:~/moos-ivp-enstabretagne/lib
export PATH=$PATH:~/moos-ivp-enstabretagne/scripts
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/moos-ivp-enstabretagne/lib
export IVP_BEHAVIOR_DIRS=$IVP_BEHAVIOR_DIRS:~/moos-ivp-enstabretagne/lib
export MOOS_IVP_ENSTABRETAGNE_PATH="/home/ciscrea/moos-ivp-enstabretagne"


alias createdevelopbranch='git checkout master && git branch -D develop | git checkout -b develop && git merge uJoystick missions scripts'

alias cdEnstaMissions='cd $MOOS_IVP_ENSTABRETAGNE_PATH/missions'
alias cdEnstaMissions='cd $MOOS_IVP_ENSTABRETAGNE_PATH/missions/saucisse/rov'

alias connectSaucisseE='ssh ciscrea@192.168.0.100:$MOOS_IVP_ENSTABRETAGNE_PATH/missions/saucisse/rov -X'
alias connectSaucisseW='ssh ciscrea@192.168.0.101:$MOOS_IVP_ENSTABRETAGNE_PATH/missions/saucisse/rov -X'
