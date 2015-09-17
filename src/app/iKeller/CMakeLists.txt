#--------------------------------------------------------
# The CMakeLists.txt for: iKeller
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Clement Aubry
#--------------------------------------------------------

SET(SRC
  kellermsg.cpp
  Keller.cpp
  main.cpp
)

ADD_EXECUTABLE(iKeller ${SRC})

TARGET_LINK_LIBRARIES(iKeller
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
   TARGET_LINK_LIBRARIES(iKeller rt)
endif()
