#--------------------------------------------------------
# The CMakeLists.txt for: iModem
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Clément Aubry (from V.Drevelle iSonar App (url?))
#--------------------------------------------------------

SET(SRC
  seanetmsg.cpp
  Modem.cpp
  main.cpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")

ADD_EXECUTABLE(iModem ${SRC})

TARGET_LINK_LIBRARIES(iModem
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   )
if(APPLE)
   MESSAGE("Building on OSX, avoid linking with realtime support")
elseif(UNIX)
   TARGET_LINK_LIBRARIES(iModem rt)
endif()
