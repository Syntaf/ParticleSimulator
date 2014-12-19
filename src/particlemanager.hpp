#ifndef PARTICLE_MANAGER_H_
#define PARTICLE_MANAGER_H_

#include "particle.hpp"
#include <glm/glm.hpp>
#include <GL/glew.h>
#ifdef USE_OPENCL
#include "opencl/cl_particle_updater.hpp"
#endif

class ParticleManager{
    public:
        // Constructor performs simple assignments and generates/bind vertex
        //	 arrays. The parent window pointer is set in order to get mouse
        //	 position during the update function, and the vertex array is
        //	 set accordingly
        ParticleManager(sf::Window* parent_window, int maxparticles=200000, 
            float drag=20.0f, float mass=50.0f,
            float size=0.15f, float mouseforce=50000);
        ~ParticleManager();

        // Initialize all particles before the first render is proc'd. The
        //	 function will arrange all particles in a square shape at the
        //	 center of the screen(regardless of the amount of particles) and
        //	 ensure all variables of the particle are initialized to their
        //	 starting values.
        void initParticles();

        // Load texture using common set of functions written for OpenGL
        //	 related programs. returns false if texture cannot be found
        bool loadTexture(const std::string& filename);

        // Generate and set initial state of buffers before the first render
        //	 is proc'd. All buffers are initially filled with NULL until the
        //	 particle position is updated. After the initial state is complete
        //	 the program will make calls to updateGLBuffers.
        void genGlBuffers();

        // Fill particle particle position and particle color arrays with new
        //	 data after updateParticles is called. Accepts index and particle_count
        //	 to properly store data in it's respective element
        void fillParticleGlBuffers(const int& index, const int& particle_count);

        // Update OpenGL buffers with new particle position and color data, called
        //	 inside updateParticles to ensure OpenGL also receives the new info
        void updateGlBuffers();

        // Active OpenGL texture for drawing to screen
        void activateTexture();

        // Accepts delta time and program matrices for context, and calculates new
        //	 position of each particle in the frame. Uses parent_window to grab
        //	 mouse input and newtons laws, also makes sure to pass the new data
        //	 along to OpenGL through fillParticleGlBuffers and updateGlBuffers
        void updateParticles(const float& delta, glm::mat4& ProjectionMatrix,
                             glm::mat4& ViewMatrix);

        // Draw all particles to the screen
        void drawParticles();

        // Reset all particles back onto screen
        void resetParticles();

        // Set functions
        void setDrag(const float& val);
        void setMass(const float& val);
        void setMouseForce(const float& val);
        void setColorR(const unsigned char& val);
        void setColorG(const unsigned char& val);
        void setColorB(const unsigned char& val);
        void setColorA(const unsigned char& val);
        void setColorRand(const int& val);


        // Get functions
        float getDrag();
        float getMass();
        float getMouseForce();
        int getParticleCount();
        int getColorR();
        int getColorG();
        int getColorB();
        int getColorA();

    private:
        float Distance(glm::vec3 const& v1, glm::vec3 const& v2);
        unsigned char clamp(float value, float min, float max);

        
        sf::Window* d_parent_window;                // keep a pointer to parent window for
                                                    // mouse data

        std::vector<Particle> d_particles_container;// meat of the class, all particles

        GLuint d_vertex_array_id;                   // our vertex array id for drawing
        GLuint d_texture_id;                        // holds the id of our particle texture
        GLuint d_billboard_vertex_buffer;           // ensure particles always face screen
        GLuint d_particles_position_buffer;         // position buffer for OpenGL
        GLuint d_particles_color_buffer;            // color buffer for OpenGL

        GLuint d_texture;                           // location of our texture

        GLfloat* g_particle_position_size_data;     // particle position size data
        GLubyte* g_particle_color_data;             // particle color data

        static const GLfloat g_vertex_buffer_data[];// simple coordinates
        
#ifdef USE_OPENCL
        cl_particle_updater *cl_updater;
#endif

        unsigned char d_R, d_G, d_B, d_A;           // the color of each particle
        float d_MOUSEFORCE;                         // force of the mouse click
        float d_MASS;                               // mass of each particle
        float d_SIZE;                               // how big is each particle?
        float d_DRAG;                               // drag after mouse released
        int d_MAXPARTICLES;                         // number of particles

};

#endif
