ParticleSimulator
=================

* [Installation](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#installation)
    * [Required Libraries](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#required-libraries)
    * [Compiler Support](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#compiler-support)
    * [Build](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#build)
      * [Windows](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#windows)
      * [Linux](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#linux)
      * [OS X](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#os-x-mac)
* [Console Commands](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#console-commands)
* [To-Do](https://github.com/Syntaf/ParticleSimulator/tree/mGL_sync/README.md#to-do)

A particle simulator written with C++ SFML OpenGL. 

Random color | Default color
:-----------:|:--------------:
![gif](http://i.imgur.com/WNB2BEY.gif) | ![gif](http://i.imgur.com/DaPWowN.gif)

#Installation#

**Required Libraries**
--------------------------------------------------------

| Library        | Minimum Version | Architecture |Header Only  |
| -------------  |:-------------:  | :---------:  |-----:       |
| GLM            | 0.9.5.3         | N/A          |  Yes        |
| GLEW           | 1.5.3           | x64          |  No         |
| SFML           | 2.1             | x64          |  No         |
| TGUI           | 0.6             | x64          |  No         |
[**GLM**](http://glm.g-truc.net/0.9.5/index.html):  A header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specification and released under the MIT license.

[**GLEW**](http://glew.sourceforge.net/):   A cross-platform open-source C/C++ extension loading library. GLEW provides efficient run-time mechanisms for determining which OpenGL extensions are supported on the target platform. OpenGL core and extension functionality is exposed in a single header file. GLEW has been tested on a variety of operating systems, including Windows, Linux, Mac OS X, FreeBSD, Irix, and Solaris.
  
[**SFML**](http://www.sfml-dev.org/):   SFML provides a simple interface to the various components of your PC, to ease the development of games and multimedia applications.

[**TGUI**](http://tgui.eu/):  TGUI is an easy to use cross-platform GUI based on SFML2 and written in C++, **Note** that TGUI currently does not offer x64 downloads. If you plan on building the Particle Simulator with x64 you will need to build TGUI from source, you can see how [here](http://tgui.eu/tutorials/v06/visual-studio/)
**Compiler Support**
--------------------------------------------------------

|  Compilers  |  Recommended Version  |  Minimum Version  |
| ----------- | --------------------- | ----------------- |
| MSVC | `v110` |  `v100` |
| GNU Compiler Collection(G++) | `4.9.0` | `?` |
| Clang | `3.4.0` | `?` |

| Build System | Recommended Version | Minimum Version |
|--------------|---------------------|-----------------|
| CMake | `2.8.4` | `2.8.4` |


Build
-----------------
Before beginning to build this application, make sure you have downloaded the prerequisites above.

Windows
--------
a quick note before moving on: this build tutorial takes into account the user is using the x64 architecture. If you are planning on building the particle simulator with x86 you may have to change around a couple CMake lines, but the program should have no issue otherwise running with x86.

1. Clone the master Particle Simulator repository in a directory of your choice.

2. Create a build folder. This project requires and out-of-tree-build. This means you ~~will be unable to~~ **should not** run CMake in the Particle Simulator folder

3. Open up the CMake GUI. In the input box labelled "Where is the source code:", enter the full path to the source folder. In the input box labelled "Where to build the binaries:", enter the full path to the build folder you created in step 2

4. Add CMake variable definitions, you will need three to configure and generate the project. They are:
    * `SFML_ROOT` - The root folder of SFML (e.g. *c:/local/sfml-2.1*)
    * `GLM_ROOT` - The root folder of GLM (e.g. *c:/local/glm*)
    * `GLEW_ROOT` - The root folder of GLEW (e.g. *c:/loacl/glew*)
    * `TGUI_ROOT` - The root folder of TGUI (e.g. *c:/local/tgui*), CMake will currently look for libraries in ${TGUI_ROOT}/build/lib. If you did not build from source or did not build inside of the source make sure to modify FindTGUI.cmake
    * CMake will search for glew libraries in {GLEW_ROOT}/lib/release/x64 `{GLEW_ROOT}/lib/Release/x64/glew32.lib`, to change this path edit line `53` in `CMakeLists.txt`
    * `USE_OPENCL` - Currently an unfinished experimental implementation with OpenCL, default `false` is it is unfinished
    * `USE_TGUI` - TGUI will soon become mandatory for the repository, but for now you can opt out of using TGUI by setting this to false
5. Press the "Configure" button. A window will pop up asking you which compiler to use. Select your x64 version of your preferred compiler. Note that it is possible to build x32 without running into any errors, feel to modify your build to build x86 if you wish.

6. If the "Generate" button is not clickable, press "Configure" again. Repeat this step until the "Generate" button becomes clickable.

7. Press "Generate".

8. Open up the build folder, and double-click ParticleSimulator.sln

9. Build the particle simulator target, **only** build in debug if you *actually* want to debug something; without optimizations the simulator will run extremely slow. CMake copies the source files shaders to the executable directory, in this case you will find the shaders and textures in `{BINARY_DIR}/Release/`. Running in debug will require you to copy these files to the Debug directory, I am working on a fix that will place all executables into the same Release directory.

10. copy the required DLL's into your Debug/Release folder, these would be
    * `sfml-graphics-2.dll`
    * `sfml-system-2.dll`
    * `sfml-window-2.dll`
    * `tgui.dll`
    * `glew32.dll`
    * for Debug mode make sure to use the debug SFML DLLS, the one's with a trailing `-d`. Previous versions of SFML do not have the `2` either as listed here.

Linux
------
*support coming soon*

OS X (Mac)
----------
*support coming soon*

#Console Commands#
The console in this application supports a number of commands aimed at allowing a ton of user interaction and fun. The available commands are:
   * `get <>`
   * `set <> <>`
   * `help <>`
   * `reset`
   * `exit`

Get and set both require a name of a variable to either get/set, and set additionally requires a value to pass. The currently support variables are:

   * `drag`
   * `mass`
   * `mouseforce`
   * `particlecount` (*get* **only** for now)
   * `color_r/g/b/a`
   * `color_rand` (*set* **only**, the value after this command is the range to pick random values from).

An example run of the console might look like:
````
> get drag
20.f
> set drag 50
> get drag
50.0f
````
#To-Do#
- [x] Begin GUI development
- [ ] Optimize particle container(chunking)
- [x] Provide a CMake build system
- [x] Implement highly customizable enviroment
- [ ] Move particle calculations to GPU

Additional Notes
-------------------------
- Intel HD 4000 graphics cards currently have issues running ModernGL, however this should not inhibit the simulator from running
- A thanks to opengl-tutorial.org for code structure and teachings
