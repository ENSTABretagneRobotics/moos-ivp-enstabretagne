#--------------------------------------------------------
# The CMakeLists.txt for: pController
# Toutatis AUVs - ENSTA Bretagne
# Author(s): MDS
#--------------------------------------------------------

SET(SRC
  Controller.cpp
  main.cpp
)

ADD_EXECUTABLE(pController ${SRC})

TARGET_LINK_LIBRARIES(pController
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
