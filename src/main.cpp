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
#include "Particle.hpp"
#include "particlemanager.hpp"
#include "config.h"

//this is for experimental openCL support, if you do not have this enabled
//the program still run nicely
#ifdef USE_OPENCL
#include "opencl/cl_particle_updater.hpp"
#endif

//this used to be needed... I guess not anymore though
//#pragma comment(lib, "glew32.lib")

int main(int argc, char* argv[]) {

//debug is slow as fuck, make sure people are running on release
#ifdef _DEBUG
    std::cout << "\nWARNING: Debug is currently active, optimizations are not enabled this is configuration" <<
        ". Unless you are actually debugging, switch to Release mode\n" << std::endl;
#endif

    std::cout << "Particle Simulator Version: " << VERSION_MAJOR << "." << VERSION_MINOR << "\n";

    srand(time(NULL));                            //seed the random generator

    sf::Window window;
    window.create(sf::VideoMode(1000,750),        //maintain 1.33 ratio
        "Particle Simulation",                    //window title
        sf::Style::Default,
          sf::ContextSettings(32, 8, 0, 3, 3)       //we're using openGL 3.3
        );

    // Initialize GLEW, needed for core profile
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glViewport(0,0,window.getSize().x,window.getSize().y);
    window.setVerticalSyncEnabled(true);
    sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(window).x + window.getSize().x/2, sf::Mouse::getPosition(window).y));

    ConsoleManager console_window(&window);
    //create GUI console
    console_window.init();

    ParticleManager particles_manager(&window);
    particles_manager.genGlBuffers();

    //dark blue background
    glClearColor(0.0f, 0.0f, 0.15f, 0.0f);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "shaders/vertexshader.vert", "shaders/fragmentshader.frag" );

    // Vertex shader
    GLuint CameraRight_worldspace_ID  = glGetUniformLocation(programID, "CameraRight_worldspace");
    GLuint CameraUp_worldspace_ID  = glGetUniformLocation(programID, "CameraUp_worldspace");
    GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

    // fragment shader
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    //load texture
    GLuint Texture = loadDDS("textures/Particle.DDS");

    //initialize particle position
    particles_manager.initParticles();

    bool running=true;    //set up bool to run SFML loop
    bool pressed=false;   //bool for managing when LMB pressed
    sf::Clock clock;      //clock for delta timing
    sf::Clock fps_clock;  //fps clock
    float fps_last_time = 0;
    while( running )
    {
        float delta = clock.restart().asSeconds();
        //handle window events
        sf::Event event;
        while(window.pollEvent(event))
                  {
            if(event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                running = false;
            else if(event.type == sf::Event::Resized)
                glViewport(0,0,event.size.width,event.size.height);
        }
        console_window.handleEvent(event, running);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //handle any input and grab matrices
        computeMatricesFromInputs(window, delta);
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

        //update position of every particle
        particles_manager.updateParticles(delta, ProjectionMatrix, ViewMatrix);
        
        //use gl blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Use our shader
        glUseProgram(programID);

        // Bind texture to Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set "myTextureSampler" sampler to user Texture Unit 0
        glUniform1i(TextureID, 0);

        // Uniforms for shaders
        glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
        glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

        glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

        particles_manager.drawParticles();
        console_window.render();
        window.display();

        //used for determining application performance
        float fps_current_time = fps_clock.restart().asSeconds();
        float fps = 1.f / (fps_current_time - (fps_current_time - fps_last_time));
        fps_last_time = fps_current_time;
        std::cout << "\r" << fps;
    }

    glDeleteProgram(programID);
    return 0;

}


