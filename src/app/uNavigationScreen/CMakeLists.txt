#--------------------------------------------------------
# The CMakeLists.txt for: uNavigationScreen
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  NavigationScreen.cpp
  main.cpp
)

ADD_EXECUTABLE(uNavigationScreen ${SRC})

TARGET_LINK_LIBRARIES(uNavigationScreen
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
