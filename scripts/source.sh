export MOOS_IVP_ENSTABRETAGNE_PATH=$(dirname `dirname $BASH_SOURCE`)
export PATH=$PATH:$MOOS_IVP_ENSTABRETAGNE_PATH/bin
export PATH=$PATH:$MOOS_IVP_ENSTABRETAGNE_PATH/lib
export PATH=$PATH:$MOOS_IVP_ENSTABRETAGNE_PATH/scripts
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MOOS_IVP_ENSTABRETAGNE_PATH/lib
export IVP_BEHAVIOR_DIRS=$IVP_BEHAVIOR_DIRS:$MOOS_IVP_ENSTABRETAGNE_PATH/lib


alias createdevelopbranch='git checkout master && git branch -D develop | git checkout -b develop && git merge uJoystick missions scripts pSimplePID'

alias cdEnstaMissions='cd $MOOS_IVP_ENSTABRETAGNE_PATH/missions'
alias cdSaucisseMissions='cd $MOOS_IVP_ENSTABRETAGNE_PATH/missions/saucisse/rov'

alias connectSaucisseW='ssh ciscrea@192.168.0.101 -x'
alias connectSaucisseE='ssh ciscrea@192.168.0.100 -x'
