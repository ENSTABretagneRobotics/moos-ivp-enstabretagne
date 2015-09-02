#--------------------------------------------------------
# The CMakeLists.txt for: iEchoSounder
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  EchoSounder.cpp
  main.cpp
)

ADD_EXECUTABLE(iEchoSounder ${SRC})

TARGET_LINK_LIBRARIES(iEchoSounder
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
