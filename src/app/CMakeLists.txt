##############################################################################
# FILE:        moos-ivp-toutatis/src/CMakeLists.txt
# DATE:        2015
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-toutatis source directory
##############################################################################


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
  ADD_SUBDIRECTORY(uJoystick)

# 'i' apps
  ADD_SUBDIRECTORY(iEchoSounder)
  ADD_SUBDIRECTORY(iRazorIMU)
  ADD_SUBDIRECTORY(iModem)
  ADD_SUBDIRECTORY(iXSensINS)
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
