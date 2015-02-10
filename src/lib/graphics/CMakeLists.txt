#--------------------------------------------------------
# The CMakeLists.txt for:                    lib/graphics
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

SET(SRC
  vibes/vibes.cpp
)

SET(HEADERS 
  vibes/vibes.h
) 

ADD_LIBRARY(ensta-graphics ${SRC})

TARGET_LINK_LIBRARIES(ensta-graphics
  geometry
  )