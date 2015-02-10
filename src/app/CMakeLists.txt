##############################################################################
# FILE:        moos-ivp-enstabzh/src/CMakeLists.txt
# DATE:        2015
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-enstabzh source directory
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
  # ADD_SUBDIRECTORY(uSimModem)

# 'i' apps
  # ADD_SUBDIRECTORY(iModem)


##############################################################################
#                           END of CMakeLists.txt
##############################################################################