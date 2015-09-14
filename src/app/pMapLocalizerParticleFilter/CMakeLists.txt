#--------------------------------------------------------
# The CMakeLists.txt for:                       pMapLocalizerParticleFilter
# Author(s):
#--------------------------------------------------------

SET(SRC
  MapLocalizerParticleFilter.cpp
  MapLocalizerParticleFilter_Info.cpp
  main.cpp
)

ADD_SUBDIRECTORY(particle-filter-localization)

INCLUDE_DIRECTORIES(particle-filter-localization/src/)

ADD_EXECUTABLE(pMapLocalizerParticleFilter ${SRC})

TARGET_LINK_LIBRARIES(pMapLocalizerParticleFilter
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread
   particle_filter)
