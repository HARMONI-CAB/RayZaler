# Installing RayZaler
At the present time, the only supported platform is GNU/Linux. We have no prebuilt binaries (yet), so you will need to build RayZaler from source.

## Requirements
In order to build RayZaler, you will need to install the following packages (along with their development files):

* A C++17 compiler (GCC 12 works)
* [Git](https://www.git-scm.com/)
* [CMake 3.20](https://cmake.org/) or later
* [Bison](https://www.gnu.org/software/bison/)
* [png++](https://www.nongnu.org/pngpp/)
* [Python 3.11](https://www.python.org/downloads/release/python-3110/) or any other still-compatible version
* [SWIG 4.0](https://swig.org/)
* [NumPy](https://numpy.org/install/)
* [Qt6 + OpenGL widgets](https://www.qt.io/product/qt6)
* [OpenGL](https://opengl.org/)
* [OSMesa](https://docs.mesa3d.org/osmesa.html)
* [FreeType](https://freetype.org/)


If you are in a Debian-based system, you can install all these dependencies by pasting the following command in the terminal.

```bash
$ sudo apt install gcc git bison libpng++-dev python3.11 swig4.0 python3-numpy qt6-base-dev qt6-base-dev-tools libqt6opengl6-dev libosmesa6-dev libfreetype-dev
```

## Downloading the code
You can download the latest changes in RayZaler with:

```bash
$ git clone --recurse-submodules https://github.com/HARMONI-CAB/RayZaler
```

Note that `--recurse-submodules` is needed, as the repository depends on [Catch2](https://github.com/catchorg/Catch2) for unit testing.

## Building and installing
RayZaler is the typical CMake project, and the steps are fairly standard. Just run, in order:

```bash
$ cd RayZaler
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

If the installation process was successful, the following elements should have been installed in your system:

* The graphical application `RZGUI`, the RayZaler's graphical interface for interactive model navegation, simulation and debugging
* The command line tool `RZViewer`, a smaller command-line tool that allows you to visualize RayZaler model files.
* `libRZ`, the library containing RayZaler's modelization and simulation code.
* The Python package `RayZaler`, to use RayZaler from Python scripts.
