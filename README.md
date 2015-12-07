MOOS-IvP-ENSTABretagne [![Build Status](https://travis-ci.org/ENSTABretagneRobotics/moos-ivp-enstabretagne.svg)](https://travis-ci.org/ENSTABretagneRobotics/moos-ivp-enstabretagne)
=================

Introduction
------------

The `moos-ivp-enstabretagne` repository is an extension of the [MOOS-IvP
Autonomy system](http://oceanai.mit.edu/moos-ivp). This includes MOOS applications,
IvP behaviors and mission files for autonomous vehicles of [ENSTA Bretagne](http://www.ensta-bretagne.fr).

The following vehicles are equipped with this software:
* Toutatis AUVs
* Optical AUVs
* ENSTA Bretagne's boat
* AUV Saucisse NG

[Documentation](http://rawgit.com/ENSTABretagneRobotics/moos-ivp-enstabretagne/master/doc/index.html)
-------------

MOOSApp documentation is available by command line:

```shell
Options:
  --alias=<ProcessName>
      Launch the MOOSApp with the given process name.
  --example, -e
      Display example MOOS configuration block.
  --help, -h
      Display this help message.
  --interface, -i
      Display MOOS publications and subscriptions.
  --version,-v
      Display all information about the MOOSApp.
```

Complete documentation can also be found in `doc/index.html`. [**Click to access**](http://rawgit.com/ENSTABretagneRobotics/moos-ivp-enstabretagne/master/doc/index.html).


Directory Structure
-------------------

The directory structure for the `moos-ivp-enstabretagne` is decribed below:

| Item             | Description                              |
|-----------------:|:-----------------------------------------|
| `bin`            | Directory for generated executable files |
| `build`          | Directory for build object files         |
| `build.sh`       | Script for building moos-ivp-enstabretagne    |
| `CMakeLists.txt` | CMake configuration file for the project |
| `data`           | Directory for storing data               |
| `doc`            | Directory for documentation files        |
| `lib`            | Directory for generated library files    |
| `missions`       | Directory for mission files              |
| `README`         | The file you are reading!                |
| `scripts`        | Directory for script files               |
| `src`            | Directory for source code (app, lib)     |


Installation
------------------

[MOOS-IvP](http://oceanai.mit.edu/moos-ivp) (typically v14.7.1) is supposed to be installed.
More information available on the official website.

Additional packages are required for building `moos-ivp-enstabretagne`:
```shell
sudo apt-get install libtinyxml2-dev libusb-1.0-0-dev mono-runtime libmono-winforms2.0-cil libv4l-dev libopencv-dev libboost1.55-dev libboost-system1.55-dev
```
Specific configuration is required for Pololu devices. In the project repository:
```shell
sudo cp scripts/99-pololu.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo adduser your_username dialout video
```
Be careful to change `your_username`.

Octomap is needed for the particle filter:
```shell
git clone https://github.com/OctoMap/octomap.git ; cd octomap/octomap/ ; mkdir build ; cd build ; cmake .. ; sudo make install
```

If you clone the repository, either use:
```shell
git clone --recursive
```
or if you already cloned, use:
```shell
git submodule init ; git submodule update --recursive
```

Build Instructions
------------------

#### Linux and Mac Users

To build on Linux and Apple platforms, execute the build script within this directory:
```shell
./build.sh
```
To build without using the supplied script, execute the following commands within this directory:
```shell
mkdir -p build
cd build
cmake ../
make
cd ..
```

#### Windows Users

To build on Windows platform, open CMake using your favorite shortcut. Then set the source
directory to be this directory and set the build directory to the `build` directory
inside this directory.

The source directory is typically next to the question:
   *Where is the source code?*
<br />
The build directory is typically next to the question:
   *Where to build the binaries?*

Alternatively, CMake can be invoked via the command line. However, you must
specify your generator. Use `cmake --help` for a list of generators and
additional help.


Environment variables
---------------------
The `moos-ivp-enstabretagne` binaries files should be added to your path to allow them
to be launched from `pAntler`.
Linux users can edit `~/.bashrc` configuration file by adding at the end:
```shell
export PATH=$PATH:~/moos-ivp-enstabretagne/bin
export PATH=$PATH:~/moos-ivp-enstabretagne/scripts
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/moos-ivp-enstabretagne/lib
```

In order for generated IvP Behaviors to be recognized by the IvP Helm, you
should add the library directory to the `IVP_BEHAVIOR_DIRS` environment
variable.

New XML documentation is also available by command-line (e.g. typing `uJoystick -e`).
This can work by specifying the `MOOS_IVP_ENSTABRETAGNE_PATH` environment variable (Linux users, update your `~/.bashrc`). For instance:
```shell
export MOOS_IVP_ENSTABRETAGNE_PATH="/home/your_username/moos-ivp-enstabretagne"
```
Linux users may need to run, in the current terminal:
```shell
source ~/.bashrc
```


Generate a new MOOS Application
-------------------------------

Please use `GenMOOSApp_ENSTABretagne` script to generate an empty structure for your new MOOSApp. For instance:

```shell
cd ./src/app
GenMOOSApp_ENSTABretagne SimModem u "John Doe"
```

Do not forget to add your new application to the CMake configuration file: `./src/app/CMakeLists.txt` and to update the main documentation page : `./doc/index.html`.


License
-------

To be defined.


Contributors
------------

* [Simon Rohou](https://github.com/SimonRohou) (ENSTA Bretagne)
* [Clement Aubry](https://github.com/ClementAubry) (ISEN)
* [Thomas Le Mézo](https://github.com/ThomasLeMezo) (ENSTA Bretagne)
* [Mohamed Saad Ibn Seddik](https://github.com/msis) (ENSTA Bretagne / CGG)
* [Vincent Drevelle](https://github.com/dvinc) (INRIA Rennes / IRISA)
* [Jérémy Nicola](https://github.com/nicolaje) (ENSTA Bretagne / iXBlue)
* [Benoit Desrochers](https://github.com/benEnsta) (ENSTA Bretagne / GESMA)
* [Guilherme Schvarcz Franco](https://github.com/schvarcz) (UFRGS)
* [Murilo Santos](https://github.com/muriloksantos) (École Navale)
* [Elouan Autret](https://github.com/Elessog) (ENSTA Bretagne)