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

    ConsoleManager console_window(&window);

    glViewport(0,0,window.getSize().x,window.getSize().y);
    window.setVerticalSyncEnabled(true);
    sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(window).x + window.getSize().x/2, sf::Mouse::getPosition(window).y));

    //create GUI console
    console_window.init();

    // Initialize GLEW, needed for core profile
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    ParticleManager particles_manager(&window);
    particles_manager.genGlBuffers();

    particles_manager.initParticles();

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
        particles_manager.updateParticles(delta, ProjectionMatrix, ViewMatrix);
        /*
        //handle any input and grab matrices
        computeMatricesFromInputs(window, delta);
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        //Need the cameras position in order to sort the particles
        glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
        //get the VP
        glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

        //grab mouse coordinates so the particles can accelerate toward the given
        //  position. The following code converts SFML mouse coordinates into model
        //  space coordinates that OpenGL can use with the current particle coords
        //  . The equation is a brain twister, I barley understand what I did myself
        glm::vec4 mousePos(
            sf::Mouse::getPosition(window).x,
            sf::Mouse::getPosition(window).y,
            0.0f,
            1.0f
            );

        //manipulation from mouse cords to model-view mouse cords
        //find inverse of Proj * View
        glm::mat4 matProj = ViewMatrix * ProjectionMatrix;
        glm::mat4 inverse = glm::inverse(matProj);
        float winZ = 1.0;

        //determine space cords
        glm::vec4 vIn(    (2.0f*((float)(mousePos.x) / (window.getSize().x))) - 1.0f,    //2 * x / window.x - 1.0f
            1.0f - (2.0f * ((float)(mousePos.y)) / (window.getSize().y)),            //1 - 2 * y / window.y
            2.0 * winZ - 1.0f,                                                        //equates to 1, we are only manipulating y,z
            1.0f                                                                    //dont question it
            );


        //find inverse
        glm::vec4 mousePosmdl = vIn * inverse;

        mousePosmdl.w = 1.0f / mousePosmdl.w;
        mousePosmdl.x *= mousePosmdl.w;
        mousePosmdl.y *= mousePosmdl.w;
        mousePosmdl.z *= mousePosmdl.w;

        //if left mouse button is pressed
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            pressed = true;
        }else
            pressed = false;

        // calculate next position of particles, determine color as well
        GLsizei ParticlesCount = 0;
#ifndef USE_OPENCL
        for(size_t i=0; i<particles_manager.size(); i++){
            Particle& p = particles_manager[i]; // shortcut

            //cycle through all particles currently alive
            if(p.life > 0.0f){

                // Decrease life, cycle through now if it's *still alive
                p.life -= delta;
                if (p.life > 0.0f){

                    p.addForce(
                        (glm::vec3(glm::vec3(-mousePosmdl.x*500,-mousePosmdl.y*500, -70.0) - p.pos) * (float)(pressed*50000/pow(Distance(glm::vec3(mousePosmdl.x,mousePosmdl.y, -70.0f),p.pos)+10,2))));
                    p.addForce( -p.speed*particles_manager.getDRAG());

                    glm::vec3 prevPosition = p.pos;
                    p.pos = p.pos + p.speed*(float)delta + 0.5f*p.getTotalForce()/p.mass*(float)pow(delta,2);
                    p.speed = (p.pos - prevPosition)/(float)delta;

                    p.clearForce();

                    float normSpeed = sqrt( pow(p.speed.x,2) + pow(p.speed.y,2));
                    p.r = 120;
                    p.g = clamp(200 - (normSpeed)*20,5,255);
                    p.b = 10;

                    p.cameradistance = glm::length2( p.pos - CameraPosition );

                    particles_manager.fillParticleGlBuffers(i, ParticlesCount);
                }else{
                    // Particles that just died will be put at the end of the buffer in SortParticles();
                    p.cameradistance = -1.0f;
                }

                ParticlesCount++;

            }
        }

        particles_manager.updateGlBuffers();

#else // USE_OPENCL

        ParticlesCount = (GLsizei)particles_manager.size();

        // move gl buffers to cl
        cl_updater.lock_gl_buffers();

        cl_updater.update(mousePosmdl, pressed, delta, ParticlesCount);

        // move buffers back to gl
        cl_updater.unlock_gl_buffers();

#endif // USE_OPENCL
        */

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


