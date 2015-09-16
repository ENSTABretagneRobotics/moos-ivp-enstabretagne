cmake_minimum_required(VERSION 2.6 FATAL_ERROR)


project(pMapLocalizer)

# C++11 for GCC 4.6
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(CMAKE_BUILD_TYPE Debug)

# Find Ibex libs
find_package(PkgConfig)
pkg_check_modules(PC_IBEX REQUIRED ibex)
add_definitions(${PC_IBEX_CFLAGS_OTHER})
include_directories(${PC_IBEX_INCLUDEDIR} ${PC_IBEX_INCLUDE_DIRS} ${PC_IBEX_INCLUDES})
set(LIBS ${LIBS} -L${PC_IBEX_LIBDIR} ${PC_IBEX_LIB_DIRS} ${PC_IBEX_LIBRARIES})
message(STATUS ${LIBS})

#file(GLOB SRCS "*.cpp")

set(SRCS maplocalizer.cpp
	vibes.cpp
        simulator.cpp
        example.cpp
)

set(HEADERS maplocalizer.h
	vibes.h
        simulator.h
)

ADD_LIBRARY(interval_filter SHARED maplocalizer.cpp)
TARGET_LINK_LIBRARY(interval_filter ${LIBS}) 

add_executable (Example ${SRCS} ${HEADERS})
TARGET_LINK_LIBRARIES(Example interval_filter) #${LIBS})

