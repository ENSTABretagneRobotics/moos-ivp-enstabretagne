#!/bin/bash

#-------------------------------------------------------------------
#  Part 1: Building moos-ivp
#-------------------------------------------------------------------

cd
svn co https://oceanai.mit.edu/svn/moos-ivp-aro/releases/moos-ivp-14.7.1 moos-ivp
cd moos-ivp
sudo apt-get update -q
sudo apt-get install -y --force-yes g++ subversion xterm cmake libfltk1.3-dev freeglut3-dev libpng12-dev libjpeg-dev libxft-dev libxinerama-dev libtiff5-dev
./build-moos.sh
./build-ivp.sh

echo 'export PATH=$PATH:~/moos-ivp/bin' >> ~/.bashrc
echo 'export PATH=$PATH:~/moos-ivp/lib' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/moos-ivp/lib' >> ~/.bashrc

source ~/.bashrc

#-------------------------------------------------------------------
#  Part 2: Building moos-ivp-enstabretagne
#-------------------------------------------------------------------

cd ../moos-ivp-enstabretagne
sudo apt-get install -y --force-yes libtinyxml2-dev libusb-1.0-0-dev mono-runtime libmono-winforms2.0-cil libv4l-dev libopencv-dev libboost1.55-dev libboost-system1.55-dev
./build.sh

echo 'export PATH=$PATH:~/moos-ivp-enstabretagne/bin' >> ~/.bashrc
echo 'export PATH=$PATH:~/moos-ivp-enstabretagne/lib' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/moos-ivp-enstabretagne/lib' >> ~/.bashrc
echo 'export IVP_BEHAVIOR_DIRS=$IVP_BEHAVIOR_DIRS:~/moos-ivp-enstabretagne/lib' >> ~/.bashrc
echo 'export MOOS_IVP_ENSTABRETAGNE_PATH="/home/'$USER'/moos-ivp-enstabretagne"' >> ~/.bashrc

source ~/.bashrc