#--------------------------------------------------------
# The CMakeLists.txt for:                       pMapLocalizerIntervals
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  MapLocalizerIntervals.cpp
  MapLocalizerIntervals_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pMapLocalizerIntervals ${SRC})

TARGET_LINK_LIBRARIES(pMapLocalizerIntervals
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread)

