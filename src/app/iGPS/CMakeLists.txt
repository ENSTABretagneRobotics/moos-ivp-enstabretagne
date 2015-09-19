#--------------------------------------------------------
# The CMakeLists.txt for: iGPS
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Thomas Le Mezo
#--------------------------------------------------------

FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_nmea/include ./lib_nmea/lib)

INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})
LINK_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

SET(SRC
  Gps.cpp
  main.cpp
)

add_custom_target(lib_nmea_build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib_nmea
    COMMAND make
)

add_custom_target(lib_nmea_clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib_nmea
    COMMAND make clean
)

ADD_EXECUTABLE(iGPS ${SRC})
add_dependencies(iGPS lib_nmea_build)

TARGET_LINK_LIBRARIES(iGPS
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   nmea
   boost_system
   proj)
