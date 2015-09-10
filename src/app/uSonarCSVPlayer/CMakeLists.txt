#--------------------------------------------------------
# The CMakeLists.txt for: uSonarCSVPlayer
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Clément Aubry
#--------------------------------------------------------

SET(SRC
  SonarCSVPlayer.cpp
  main.cpp
)

ADD_EXECUTABLE(uSonarCSVPlayer ${SRC})

TARGET_LINK_LIBRARIES(uSonarCSVPlayer
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
