#!/bin/bash

INVOCATION_ABS_DIR=`pwd`
BUILD_TYPE="None"
CMD_LINE_ARGS=""
JOYSTICK="ON"
RAZOR="ON"
XSENS="ON"
GPSOE="ON"
INTERVAL_FILTER="OFF"

#-------------------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	printf "%s [SWITCHES]                       \n" $0
	printf "Switches:                           \n"
	printf "  --help, -h                        \n"
        printf "  --debug,   -d                     \n"
        printf "  --release, -r                     \n"
	printf "Notes:                              \n"
	printf " (1) All other command line args will be passed as args    \n"
	printf "     to \"make\" when it is eventually invoked.            \n"
	printf " (2) For example -k will continue making when/if a failure \n"
	printf "     is encountered in building one of the subdirectories. \n"
	printf " (3) For example -j2 will utilize a 2nd core in the build  \n"
	printf "     if your machine has two cores. -j4 etc for quad core. \n"
	exit 0;
    elif [ "${ARGI}" = "--debug" -o "${ARGI}" = "-d" ] ; then
        BUILD_TYPE="Debug"
    elif [ "${ARGI}" = "--release" -o "${ARGI}" = "-r" ] ; then
        BUILD_TYPE="Release"
    elif [ "${ARGI}" = "--no-joystick" ] ; then
        JOYSTICK="OFF"
    elif [ "${ARGI}" = "--no-razor" ] ; then
        RAZOR="OFF"
    elif [ "${ARGI}" = "--no-xsens" ] ; then
        XSENS="OFF"
    elif [ "${ARGI}" = "--no-gpsoe" ] ; then
        GPSOE="OFF"
    elif [ "${ARGI}" = "--no-intervals" ] ; then
        INTERVAL_FILTER="OFF"
    else
	CMD_LINE_ARGS=$CMD_LINE_ARGS" "$ARGI
    fi
done

#-------------------------------------------------------------------
#  Part 2: Invoke the call to make in the build directory
#-------------------------------------------------------------------
mkdir -p build
mkdir -p bin
mkdir -p lib
cd build

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_JOYSTICK=${JOYSTICK} -DWITH_RAZOR=${RAZOR} -DWITH_XSENS=${XSENS} -DWITH_GPSOE=${GPSOE} -DWITH_INTERVAL_FILTER=${INTERVAL_FILTER} ../

make ${CMD_LINE_ARGS} || exit 1
cd "${INVOCATION_ABS_DIR}"