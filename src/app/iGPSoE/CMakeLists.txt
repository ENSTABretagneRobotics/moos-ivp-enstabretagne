#--------------------------------------------------------
# The CMakeLists.txt for:                       iGPSoE
# Author(s):                              Équipe Ciscrea - Veni Vidi Vici
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC

  OSUtils/OSCore.c
  OSUtils/OSTime.c
  OSUtils/OSMisc.c
  OSUtils/OSThread.c
  OSUtils/OSCriticalSection.c
  OSUtils/OSRS232Port.c
  Extensions/Devices/GPSoEUtils/GPSoECore.c
  Extensions/Devices/GPSoEUtils/GPSoECfg.c
  Extensions/Devices/GPSoEUtils/GPSoEMgr.c

  GPSoE.cpp
  main.cpp
)

INCLUDE_DIRECTORIES(OSUtils/ Extensions/FileTypes/CONFUtils/ Extensions/Devices/GPSoEUtils/)

ADD_EXECUTABLE(iGPSoE ${SRC})

TARGET_LINK_LIBRARIES(iGPSoE
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)

