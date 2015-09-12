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

# 'u' apps
if (WITH_JOYSTICK)
  MESSAGE("Building with uJoystick")
  ADD_SUBDIRECTORY(uJoystick)
endif (WITH_JOYSTICK)

# 'i' apps
  #ADD_SUBDIRECTORY(iEchoSounder)
  ADD_SUBDIRECTORY(iRazorIMU)
  ADD_SUBDIRECTORY(iModem)
  ADD_SUBDIRECTORY(iXSensINS)
  ADD_SUBDIRECTORY(iGPS)
  ADD_SUBDIRECTORY(iSonar)


##############################################################################
#                           END of CMakeLists.txt
##############################################################################
