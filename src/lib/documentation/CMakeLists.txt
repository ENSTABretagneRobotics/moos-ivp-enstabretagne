#--------------------------------------------------------
# The CMakeLists.txt for:               lib/documentation
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

SET(SRC
  MOOSAppDoc.cpp
)

SET(HEADERS 
  MOOSAppDoc.h
) 

ADD_LIBRARY(ensta-documentation ${SRC})