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

        bool initGL();
        bool initParticles();
        void run();

    private:
        sf::Window d_main_window;

        //we need don't want these to be created until the constructor, so make them
        //pointers
        ConsoleManager *d_console_window;
        ParticleManager *d_particles_manager;

        //OpenGL variables
        GLuint d_program_id;
        GLuint d_cameraright_worldspace_id;
        GLuint d_cameraup_worldspace_id;
        GLuint d_viewprojmatrix_id;

        //texture ID
        GLuint d_texture_id;

        //timers
        sf::Clock clock;
        sf::Clock fps_clock;

        //loop variables
        bool running;
        bool pressed;
        float fps_last_time;
};

#endif