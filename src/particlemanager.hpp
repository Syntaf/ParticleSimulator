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
        ParticleManager(sf::Window* parent_window, int maxparticles=200000, 
            float drag=20.0f, float mass=50.0f,
            float size=0.15f, float mouseforce=50000);
        ~ParticleManager();

        //particle initialization functions
        void initParticles();
        bool loadTexture(const std::string& filename);

        //OpenGL buffer functions
        void genGlBuffers();
        void fillParticleGlBuffers(const int& index, const int& particle_count);
        void updateGlBuffers();

        //active OpenGL texture for drawing to screen
        void activateTexture();

        //update the positions of the particles and also update the openGL 
        //  buffers
        void updateParticles(const float& delta, glm::mat4& ProjectionMatrix,
                             glm::mat4& ViewMatrix);

        //draw all particles to the screen
        void drawParticles();

        //reset all particles back onto screen
        void resetParticles();

        //set functions
        void setDrag(const float& val);
        void setMass(const float& val);
        void setMouseForce(const float& val);
        void setColorR(const unsigned char& val);
        void setColorG(const unsigned char& val);
        void setColorB(const unsigned char& val);
        void setColorA(const unsigned char& val);


        //get functions
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

        //keep a pointer to parent window for pulling data
        sf::Window* d_parent_window;

        //the meat of the class, all our particles
        std::vector<Particle> d_particles_container;

        //openGL buffer data, texture id and vertex array id
        GLuint d_vertex_array_id;
        GLuint d_texture_id;
        GLuint d_billboard_vertex_buffer;
        GLuint d_particles_position_buffer;
        GLuint d_particles_color_buffer;

        GLuint d_texture;

        //data
        GLfloat* g_particle_position_size_data;
        GLubyte* g_particle_color_data;

        static const GLfloat g_vertex_buffer_data[];
        
#ifdef USE_OPENCL
        cl_particle_updater *cl_updater;
#endif

        //define program 'constants'
        unsigned char d_R, d_G, d_B, d_A;
        float d_MOUSEFORCE;
        float d_MASS;
        float d_SIZE;
        float d_DRAG;
        int d_MAXPARTICLES;

};

#endif