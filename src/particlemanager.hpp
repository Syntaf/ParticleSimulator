#ifndef PARTICLE_MANAGER_H_
#define PARTICLE_MANAGER_H_

#include "Particle.hpp"
#include <glm/glm.hpp>
#include <GL/glew.h>
#ifdef USE_OPENCL
#include "opencl\cl_particle_updater.hpp"
#endif

class ParticleManager{
    public:
        ParticleManager(sf::Window* parent_window, int maxparticles=500000, 
            float drag=20.0f, float mass=50.0f,
            float size=0.15f, float mouseforce=5000);
        ~ParticleManager();

        void initParticles();
        void loadTexture(const std::string& filename);

        void genGlBuffers();
        void fillParticleGlBuffers(const int& index, const int& particle_count);
        void updateGlBuffers();

        void activateTexture();
        void updateParticles(const float& delta, glm::mat4& ProjectionMatrix,
                             glm::mat4& ViewMatrix);
        void drawParticles();

        float getDRAG()const{return d_DRAG;}

        size_t size(){return d_particles_container.size();}

        Particle& operator[](const size_t& i){return d_particles_container[i];}

    private:
        sf::Window* d_parent_window;
        float Distance(glm::vec3 const& v1, glm::vec3 const& v2);
        unsigned char clamp(float value, float min, float max);

        std::vector<Particle> d_particles_container;

        GLuint d_vertex_array_id;
        GLuint d_texture_id;
        GLuint d_billboard_vertex_buffer;
        GLuint d_particles_position_buffer;
        GLuint d_particles_color_buffer;

        GLuint d_texture;

        GLfloat* g_particle_position_size_data;
        GLubyte* g_particle_color_data;

        static const GLfloat g_vertex_buffer_data[];
        
#ifdef USE_OPENCL
        cl_particle_updater *cl_updater;
#endif

        //define program 'constants'
        float d_MOUSEFORCE;
        float d_MASS;
        float d_SIZE;
        float d_DRAG;
        int d_MAXPARTICLES;

};

#endif