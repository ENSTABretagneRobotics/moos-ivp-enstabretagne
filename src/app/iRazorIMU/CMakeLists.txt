#--------------------------------------------------------
# The CMakeLists.txt for: iRazorIMU
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

# Needed for RazorAHRS compilation:
SET(CMAKE_CXX_FLAGS "-D_REENTRANT=1 -std=c++11")

SET(SRC
  razor-9dof-ahrs/RazorAHRS.cpp
  RazorIMU.cpp
  main.cpp
)

ADD_EXECUTABLE(iRazorIMU ${SRC})

TARGET_LINK_LIBRARIES(iRazorIMU
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
