#--------------------------------------------------------
# The CMakeLists.txt for: iXSensINS
# Toutatis AUVs - ENSTA Bretagne
# Author(s): 
#--------------------------------------------------------

FILE(GLOB LOCAL_LIBRARY_DIRS ./API_XSens/include ./API_XSens/xcommunication ./API_XSens/xstypes)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})
LINK_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

SET(SRC
  XSensINS.cpp
  main.cpp
  deviceclass.cpp
)

add_custom_target(api_xsens_build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/API_XSens
    COMMAND make
)

add_custom_target(clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/API_XSens
    COMMAND make clean
)

ADD_EXECUTABLE(iXSensINS ${SRC})
add_dependencies(iXSensINS api_xsens_build)

TARGET_LINK_LIBRARIES(iXSensINS
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   xcommunication
   xstypes)
