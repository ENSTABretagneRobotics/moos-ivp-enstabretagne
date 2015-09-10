#--------------------------------------------------------
# The CMakeLists.txt for: iModem
# ENSTA Bretagne
# Author(s): Clément Aubry
#--------------------------------------------------------

SET(SRC
  seanetmsg.cpp
  Modem.cpp
  main.cpp
)

ADD_EXECUTABLE(iModem ${SRC})

TARGET_LINK_LIBRARIES(iModem
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
