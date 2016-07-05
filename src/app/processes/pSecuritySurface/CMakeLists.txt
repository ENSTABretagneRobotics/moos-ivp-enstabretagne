#--------------------------------------------------------
# The CMakeLists.txt for: pSecuritySurface
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): 
#--------------------------------------------------------

SET(SRC
  SecuritySurface.cpp
  main.cpp
)

ADD_EXECUTABLE(pSecuritySurface ${SRC})

TARGET_LINK_LIBRARIES(pSecuritySurface
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
