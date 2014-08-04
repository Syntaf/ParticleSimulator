#ifndef APP_H_
#define APP_H_

#include "GL/glew.h"
#include <SFML/Graphics.hpp>
#include "particlemanager.hpp"
#include "guiconsole/console.hpp"

class App{

    public:
        App(sf::VideoMode mode);
        ~App();

        //initialize uniform locations, compile shaders and create
        //  program ID
        bool initGL();

        //load a texture and initialize particle starting positions
        //  if openCL is enabled it will also put all of the new data
        //  into a buffer for the GPU to use.
        bool initParticles(const std::string& texturepath);

        //the main program loop, run will handle event, receive user input
        //  and handle user input. the most important part of the class
        void run();

        //this is an exit method to be used for the console window, specifically to set running
        //to false
        static void procClose();

    private:
        //main window to render to
        sf::Window d_main_window;

        //we need to control when these are created due to the nature of their
        //  constructors, so we will use dynamic allocations.
#ifdef USE_TGUI
        ConsoleManager *d_console_window;
#endif
        ParticleManager *d_particles_manager;

        //OpenGL variables
        GLuint d_program_id;
        GLuint d_cameraright_worldspace_id;
        GLuint d_cameraup_worldspace_id;
        GLuint d_viewprojmatrix_id;

        //texture ID
        GLuint d_texture_id;

        //timers for delta and FPS
        sf::Clock clock;
        sf::Clock fps_clock;

        //loop variables
        static bool running;
        bool pressed;
        float fps_last_time;
};

#endif