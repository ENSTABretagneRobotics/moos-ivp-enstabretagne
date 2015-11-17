#--------------------------------------------------------
# The CMakeLists.txt for: pRad2Degree
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Rad2Degree.cpp
  main.cpp
)

ADD_EXECUTABLE(pRad2Degree ${SRC})

TARGET_LINK_LIBRARIES(pRad2Degree
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   geometry
   m
   pthread)
