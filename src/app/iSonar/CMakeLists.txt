#--------------------------------------------------------
# The CMakeLists.txt for:                       iSonar
# Author(s):                              Équipe Ciscrea - Veni Vidi Vici
#--------------------------------------------------------

SET(SRC
  seanetmsg.cpp
  Sonar.cpp
  main.cpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")

ADD_EXECUTABLE(iSonar ${SRC})

TARGET_LINK_LIBRARIES(iSonar
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
  TARGET_LINK_LIBRARIES(iSonar rt)
endif()
