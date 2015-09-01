#--------------------------------------------------------
# The CMakeLists.txt for: iXSensINS
# Toutatis AUVs - ENSTA Bretagne
# Author(s): 
#--------------------------------------------------------

FILE(GLOB LOCAL_LIBRARY_DIRS ./lib ./include)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})
LINK_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

SET(SRC
  XSensINS.cpp
  main.cpp
  deviceclass.cpp
)

ADD_EXECUTABLE(iXSensINS ${SRC})

TARGET_LINK_LIBRARIES(iXSensINS
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   xcommunication
   xstypes)
