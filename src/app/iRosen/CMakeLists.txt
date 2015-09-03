#--------------------------------------------------------
# The CMakeLists.txt for: iRosen
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Rosen.cpp
  Pololu.cpp
  main.cpp
)

SET(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5Widgets)

ADD_EXECUTABLE(iRosen ${SRC})

TARGET_LINK_LIBRARIES(iRosen
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   Qt5::Widgets)
