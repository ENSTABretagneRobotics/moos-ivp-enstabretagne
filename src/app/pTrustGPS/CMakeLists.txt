#--------------------------------------------------------
# The CMakeLists.txt for:                       pTrustGPS
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  TrustGPS.cpp
  TrustGPS_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pTrustGPS ${SRC})

TARGET_LINK_LIBRARIES(pTrustGPS
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread)

