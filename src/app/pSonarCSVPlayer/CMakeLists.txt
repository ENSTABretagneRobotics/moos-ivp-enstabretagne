#--------------------------------------------------------
# The CMakeLists.txt for: pSonarCSVPlayer
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Clément Aubry
#--------------------------------------------------------

SET(SRC
  SonarCSVPlayer.cpp
  main.cpp
)

ADD_EXECUTABLE(pSonarCSVPlayer ${SRC})

TARGET_LINK_LIBRARIES(pSonarCSVPlayer
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
