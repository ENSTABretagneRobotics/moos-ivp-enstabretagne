#--------------------------------------------------------
# The CMakeLists.txt for: pDegree2Rad
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Degree2Rad.cpp
  main.cpp
)

ADD_EXECUTABLE(pDegree2Rad ${SRC})

TARGET_LINK_LIBRARIES(pDegree2Rad
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   geometry
   m
   pthread)
