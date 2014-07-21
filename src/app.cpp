#include "app.hpp"
#include <iostream>
#include <cstdlib>
#include <vector>
#include "GL/glew.h"
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/controls.hpp"
#include "guiconsole/console.hpp"
#include "particle.hpp"
#include "particlemanager.hpp"

#ifdef USE_OPENCL
#include "opencl/cl_particle_updater.hpp"
#endif

App::App(sf::VideoMode mode):
    running(true), pressed(false), fps_last_time(0.0f)
{
    d_main_window.create(mode,
        "Particle Simulator",
        sf::Style::Default,
        sf::ContextSettings(32, 8, 0, 3, 3)
    );

#ifdef USE_TGUI
    d_console_window = new ConsoleManager(&d_main_window);
#endif

    glViewport(0,0,d_main_window.getSize().x, d_main_window.getSize().y);
    //disable verical sync, set mouse position to middle of screen
    d_main_window.setVerticalSyncEnabled(false);
    sf::Mouse::setPosition(
        sf::Vector2i(
            sf::Mouse::getPosition(d_main_window).x + d_main_window.getSize().x/2, 
            sf::Mouse::getPosition(d_main_window).y
        )
     );

#ifdef USE_TGUI
    //create GUI console
    d_console_window->init();
#endif

}


App::~App()
{
    //free dynamic memory
#ifdef USE_TGUI
    delete d_console_window;
#endif
    delete d_particles_manager;
    //delete program
    glDeleteProgram(d_program_id);
}

bool App::initGL()
{
    glewExperimental = true;
    if(glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return false;
    }
    //create our particle manager class
    d_particles_manager = new ParticleManager(&d_main_window);
    //generate particle manager GL buffers
    d_particles_manager->genGlBuffers();

    //dark blue background
    glClearColor(0.0f, 0.0f, 0.15f, 0.0f);

    //create and compile our GLSL program from the shaders
    d_program_id = LoadShaders("shaders/vertexshader.vert",
        "shaders/fragmentshader.frag");

    //vertex shader locations
    d_cameraright_worldspace_id = glGetUniformLocation(d_program_id, "CameraRight_worldspace");
    d_cameraup_worldspace_id = glGetUniformLocation(d_program_id, "CameraUp_worldspace");
    d_viewprojmatrix_id = glGetUniformLocation(d_program_id, "VP");
    
    //fragment shader locations
    d_texture_id = glGetUniformLocation(d_program_id, "myTextureSampler");
    return true;
}

bool App::initParticles(const std::string& texturepath)
{
    //load texture and initialize particle position
    if(!(d_particles_manager->loadTexture(texturepath.c_str())))
        return false;
    d_particles_manager->initParticles();
    return true;
}

void App::run()
{
    while( running )
    {
        float delta = clock.restart().asSeconds();
        //handle window events
        sf::Event event;
        while(d_main_window.pollEvent(event))
                  {
            if(event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                running = false;
            else if(event.type == sf::Event::Resized)
                glViewport(0,0,event.size.width,event.size.height);
        }
#ifdef USE_TGUI
        d_console_window->handleEvent(event, running);
#endif

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //handle any input and grab matrices
        computeMatricesFromInputs(d_main_window, delta);
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

        //update position of every particle
        d_particles_manager->updateParticles(delta, ProjectionMatrix, ViewMatrix);
        
        //use gl blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Use our shader
        glUseProgram(d_program_id);

        // Bind texture to Texture Unit 0
        d_particles_manager->activateTexture();
        // Set "myTextureSampler" sampler to user Texture Unit 0
        glUniform1i(d_texture_id, 0);

        // Uniforms for shaders
        glUniform3f(d_cameraright_worldspace_id, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
        glUniform3f(d_cameraup_worldspace_id, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

        glUniformMatrix4fv(d_viewprojmatrix_id, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

        d_particles_manager->drawParticles();
#ifdef USE_TGUI
        d_console_window->render();
        d_main_window.display();
#endif

        //used for determining application performance
        float fps_current_time = fps_clock.restart().asSeconds();
        float fps = 1.f / (fps_current_time - (fps_current_time - fps_last_time));
        fps_last_time = fps_current_time;
        std::cout << "\r" << fps;
    }
}