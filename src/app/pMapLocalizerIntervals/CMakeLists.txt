#--------------------------------------------------------
# The CMakeLists.txt for:                       pMapLocalizerIntervals
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  MapLocalizerIntervals.cpp
  MapLocalizerIntervals_Info.cpp
  main.cpp
)

ADD_SUBDIRECTORY(interval-filter-localization)

INCLUDE_DIRECTORIES(interval-filter-localization)

ADD_EXECUTABLE(pMapLocalizerIntervals ${SRC})

TARGET_LINK_LIBRARIES(pMapLocalizerIntervals
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread
   interval_filter)

