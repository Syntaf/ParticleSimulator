/*
    This file needs to be broken up BADLY! Work in progress, be careful
    editing this file as it will be changed very soon.
*/
#include <iostream>
#include <cstdlib>
#include <vector>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "common/shader.hpp"
#include "common/controls.hpp"
#include "common/texture.hpp"
#include "guiconsole/console.hpp"
#include "particle.hpp"
#include "particlemanager.hpp"
#include "app.hpp"
#include "config.h"

//this is for experimental openCL support, if you do not have this enabled
//the program still run nicely
#ifdef USE_OPENCL
#include "opencl/cl_particle_updater.hpp"
#endif

int main(int argc, char* argv[]) {

//debug is slow as fuck, make sure people are running on release
#ifdef _DEBUG
    std::cout << "\nWARNING: Debug is currently active, optimizations are not enabled this is configuration" <<
        ". Unless you are actually debugging, switch to Release mode\n" << std::endl;
#endif

    std::cout << "Particle Simulator Version: " << VERSION_MAJOR << "." << VERSION_MINOR << "\n";

    //keep 1.33 ratio
    App programApp(sf::VideoMode(1000,750));

    //if either initialization fails, exit
    if( !(programApp.initGL()) || !(programApp.initParticles("textures/Particle.dds")) )
        return -1;

    //run main loop
    programApp.run();

    return 0;
}


