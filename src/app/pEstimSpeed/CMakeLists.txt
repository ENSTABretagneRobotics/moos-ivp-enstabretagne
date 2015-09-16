#--------------------------------------------------------
# The CMakeLists.txt for:                       pEstimSpeed
# Author(s): Jeremy NICOLA                             
#--------------------------------------------------------

# This is a big overkill to include 
# Eigen for a 3x3 matrix inversion
INCLUDE_DIRECTORIES("Eigen/")

SET(SRC
  EstimSpeed.cpp
  EstimSpeed_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pEstimSpeed ${SRC})

TARGET_LINK_LIBRARIES(pEstimSpeed
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread)

