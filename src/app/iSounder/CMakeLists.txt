#--------------------------------------------------------
# The CMakeLists.txt for: iSounder
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  EchoSounder.cpp
  main.cpp
)

ADD_EXECUTABLE(iSounder ${SRC})

TARGET_LINK_LIBRARIES(iSounder
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
