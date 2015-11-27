#--------------------------------------------------------
# The CMakeLists.txt for: pMixThrusters
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): 
#--------------------------------------------------------

SET(SRC
  MixThrusters.cpp
  main.cpp
)

INCLUDE_DIRECTORIES("Eigen/")

ADD_EXECUTABLE(pMixThrusters ${SRC})

TARGET_LINK_LIBRARIES(pMixThrusters
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
