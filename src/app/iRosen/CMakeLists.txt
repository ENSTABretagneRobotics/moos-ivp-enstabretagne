#--------------------------------------------------------
# The CMakeLists.txt for: iRosen
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Rosen.cpp
  main.cpp
)

ADD_EXECUTABLE(iRosen ${SRC})

TARGET_LINK_LIBRARIES(iRosen
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
