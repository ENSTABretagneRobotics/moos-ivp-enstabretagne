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

ADD_LIBRARY(xml-documentation ${SRC})

TARGET_LINK_LIBRARIES(xml-documentation
  tinyxml2
  mbutil
  )