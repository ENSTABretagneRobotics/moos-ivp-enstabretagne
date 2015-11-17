#--------------------------------------------------------
# The CMakeLists.txt for: pMixThrusters
# Toutatis AUVs - ENSTA Bretagne
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
