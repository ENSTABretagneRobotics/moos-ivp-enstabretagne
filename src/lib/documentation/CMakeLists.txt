#--------------------------------------------------------
# The CMakeLists.txt for:               lib/documentation
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

SET(SRC
  MOOSAppDocumentation.cpp
)

SET(HEADERS 
  MOOSAppDocumentation.h
) 

ADD_LIBRARY(ensta-documentation ${SRC})

TARGET_LINK_LIBRARIES(ensta-documentation
  tinyxml2
  mbutil
  )