#ifndef APP_H_
#define APP_H_

#include "GL/glew.h"
#include <SFML/Graphics.hpp>
#include "particlemanager.hpp"
#include "guiconsole/console.hpp"

class App{

    public:
        // Constructor simply creates main window, specify context settings
        //	 and set position of main window to center of screen	
        App(sf::VideoMode mode);
        ~App();

        // Initialize all OpenGL aspects of program, ensures GLEW is working
        //	 and allocates the particle manager. Once particle manager is
        //	 allocated the console window is able to be binded and does such,
        //	 then the openGL buffers are generated, shaders loaded and shader
        //	 uniform locations created.
        bool initGL(const int& particle_count);

        // Load texture and internally initialize all particles once the texture
        //	 is loaded by calling the particle manager init particles. See
        //	 Particle Manager initParticles for more detailed information.
        bool initParticles(const std::string& texturepath);

        // The main loop of the program, while active, run calculates the delta time
        //	 for game movement, handles window events for both the console and window,
        //	 grabs matrices based off user input and makes calls to particle manager
        //	 rendering the particles to the screen.
        void run();

        // Exit method created for use by the console window with an exit event is 
        //	 parsed by the console. If the console closes so does the program
        static void procClose();

    private:
        sf::Window d_main_window;               // main window for program.

#ifdef USE_TGUI
        ConsoleManager *d_console_window;       // console window for parsing commands
#endif
        ParticleManager *d_particles_manager;   // manages all aspects of particles

        GLuint d_program_id;                    // we will only be using one program
        GLuint d_cameraright_worldspace_id;     // vector used by shader
        GLuint d_cameraup_worldspace_id;        // another vector used by shader
        GLuint d_viewprojmatrix_id;             // view and projection matrix for shader

        GLuint d_texture_id;                    // holds id of our texture

        sf::Clock clock;                        // calculates delta time
        sf::Clock fps_clock;                    // calculates fps

        static bool running;                    // application runs while this is true
        bool pressed;                           // is the mouse pressed?
        float fps_last_time;                    // var for calculating fps
};

#endif