#--------------------------------------------------------
# The CMakeLists.txt for:                       iLidarMove
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  LidarMove.cpp
  LidarMove_Info.cpp
  main.cpp
)


add_custom_target(api_dynam_build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/API_DYNAMIXEL/src
    COMMAND make
)

add_custom_target(api_dynam_clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/API_DYNAMIXEL/src
    COMMAND make clean
)

add_custom_target(api_servo_build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/API_DYNAMIXEL/DriverServo
    COMMAND make
)

add_custom_target(api_servo_clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/API_DYNAMIXEL/DriverServo
    COMMAND make clean
)

add_custom_target(api_lidar_build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/API_LIDAR_LITE/src
    COMMAND make
)

add_custom_target(api_lidar_clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/API_LIDAR_LITE/src
    COMMAND make clean
)


ADD_EXECUTABLE(iLidarMove ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/API_DYNAMIXEL/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/API_LIDAR_LITE/include
 )


TARGET_LINK_LIBRARIES(iLidarMove
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread
   ${CMAKE_CURRENT_SOURCE_DIR}/API_DYNAMIXEL/lib/libr_28_servo.a  
   ${CMAKE_CURRENT_SOURCE_DIR}/API_DYNAMIXEL/lib/libdxl.a
   
   ${CMAKE_CURRENT_SOURCE_DIR}/API_LIDAR_LITE/lib/libserial_lidar.a
)

