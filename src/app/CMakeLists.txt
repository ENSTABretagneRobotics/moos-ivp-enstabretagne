##############################################################################
# FILE:        moos-ivp-enstabretagne/src/CMakeLists.txt
# DATE:        2015
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-enstabretagne source directory
##############################################################################


#=============================================================================
# Options
#=============================================================================

  option(WITH_RAZOR "Add Razor IMU support (Causes problem on OSX)" ON)
  option(WITH_JOYSTICK "Add joystick support (Unavailable on Windows and OSX)" ON)
  option(WITH_XSENS "Add XSens INS support (Precompiled lib is not available ATM for OSX)" ON)
  option(WITH_GPSOE "Add GPSoE support (Clock config not available on a Mac.)" ON)
  option(WITH_INTERVAL_FILTER "Add Interval filter support (recquires Ibex library)")

#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================

  FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
  INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

#============================================================================
# List the subdirectories to build...
#============================================================================

# 'p' apps

  ADD_SUBDIRECTORY(processes/pController)
  ADD_SUBDIRECTORY(processes/pWallDetector)
  ADD_SUBDIRECTORY(processes/pSimplePID)
  ADD_SUBDIRECTORY(processes/pOpiDetection)
  ADD_SUBDIRECTORY(processes/pRad2Degree)
  ADD_SUBDIRECTORY(processes/pDegree2Rad)
  ADD_SUBDIRECTORY(processes/pTrustGPS)
  ADD_SUBDIRECTORY(processes/pEstimSpeed)
  ADD_SUBDIRECTORY(processes/pMixThrusters)
  ADD_SUBDIRECTORY(processes/pLatLon2LocalGrid)
  ADD_SUBDIRECTORY(processes/pSonarCSVPlayer)

  if (WITH_INTERVAL_FILTER)
    MESSAGE("Building with pMapLocalizer*")
    ADD_SUBDIRECTORY(processes/pMapLocalizerIntervals)
    ADD_SUBDIRECTORY(processes/pMapLocalizerParticleFilter)
  endif(WITH_INTERVAL_FILTER)

# 'u' apps

  ADD_SUBDIRECTORY(utilities/uSensorsViewer)
  ADD_SUBDIRECTORY(utilities/uNavScreen)

  if (WITH_JOYSTICK)
    MESSAGE("Building with uJoystick")
    ADD_SUBDIRECTORY(utilities/uJoystick)
  endif (WITH_JOYSTICK)

# 'i' apps

  ADD_SUBDIRECTORY(interfaces/iSounder)
  ADD_SUBDIRECTORY(interfaces/iGPS)
  ADD_SUBDIRECTORY(interfaces/iModem)
#  ADD_SUBDIRECTORY(interfaces/iLidarMove)
  ADD_SUBDIRECTORY(interfaces/iPololu)
  ADD_SUBDIRECTORY(interfaces/iSonar)
  ADD_SUBDIRECTORY(interfaces/iKeller)
  ADD_SUBDIRECTORY(interfaces/iSaucisse)
  ADD_SUBDIRECTORY(interfaces/iCamera)

  if (WITH_RAZOR)
    MESSAGE("Building with iRazorIMU")
    ADD_SUBDIRECTORY(interfaces/iRazorIMU)
  endif (WITH_RAZOR)

  if (WITH_XSENS)
    MESSAGE("Building with iXSensINS")
    ADD_SUBDIRECTORY(interfaces/iXSensINS)
  endif (WITH_XSENS)

  if (WITH_GPSOE)
    MESSAGE("Building with iGPSoE")
    ADD_SUBDIRECTORY(interfaces/iGPSoE)
  endif (WITH_GPSOE)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################
