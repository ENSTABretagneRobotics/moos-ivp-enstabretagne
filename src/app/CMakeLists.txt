##############################################################################
# FILE:        moos-ivp-toutatis/src/CMakeLists.txt
# DATE:        2015
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-toutatis source directory
##############################################################################


#=============================================================================
# Options
#=============================================================================

option(WITH_RAZOR "Add Razor IMU support (Causes problem on OSX)" ON)
option(WITH_JOYSTICK "Add joystick support (Unavailable on Windows and OSX)" ON)
option(WITH_XSENS "Add XSens INS support (Precompiled lib is not available ATM for OSX)" ON)

#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

#============================================================================
# List the subdirectories to build...
#============================================================================

# 'p' apps
  # ADD_SUBDIRECTORY(pLocalization)
  ADD_SUBDIRECTORY(pController)

# 'u' apps
if (WITH_JOYSTICK)
  MESSAGE("Building with uJoystick")
  ADD_SUBDIRECTORY(uJoystick)
endif (WITH_JOYSTICK)

# 'i' apps
  #ADD_SUBDIRECTORY(iEchoSounder)
if (WITH_RAZOR)
  MESSAGE("Building with iRazor IMU")
  ADD_SUBDIRECTORY(uSonarCSVPlayer)

# 'i' apps
  ADD_SUBDIRECTORY(iEchoSounder)
  ADD_SUBDIRECTORY(iRazorIMU)
endif (WITH_RAZOR)
  ADD_SUBDIRECTORY(iModem)
if (WITH_XSENS)
  MESSAGE("Building with XSens INS")
  ADD_SUBDIRECTORY(iXSensINS)
endif (WITH_XSENS)
  ADD_SUBDIRECTORY(iGPS)
  ADD_SUBDIRECTORY(iSonar)
  ADD_SUBDIRECTORY(iKeller)
  ADD_SUBDIRECTORY(iSaucisse)
  ADD_SUBDIRECTORY(iGPSoE)
  ADD_SUBDIRECTORY(pSensorViewer)
  ADD_SUBDIRECTORY(iCamera)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################
