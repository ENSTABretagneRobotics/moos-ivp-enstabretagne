#--------------------------------------------------------
# The CMakeLists.txt for:                    lib/graphics
# Author(s):
#--------------------------------------------------------

SET(SRC
  vibes/vibes.cpp
)

SET(HEADERS 
  vibes/vibes.h
) 

ADD_LIBRARY(graphics ${SRC})

TARGET_LINK_LIBRARIES(graphics
  geometry
  )