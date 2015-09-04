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
   rt
   )
