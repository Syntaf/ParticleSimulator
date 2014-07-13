#include <iostream>
#include <string>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include "Particle.hpp"
#include "particlemanager.hpp"

const GLfloat ParticleManager::g_vertex_buffer_data[] = {
         -0.5f, -0.5f, 0.0f,
          0.5f, -0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
};

ParticleManager::ParticleManager(int maxparticles, float drag,
                                 float mass, float size, float mouseforce):
d_MAXPARTICLES(maxparticles), d_DRAG(drag), d_MASS(mass),
d_SIZE(size), d_MOUSEFORCE(mouseforce)
{
    g_particle_position_size_data = new GLfloat[d_MAXPARTICLES * 4];
    g_particle_color_data = new GLubyte[d_MAXPARTICLES * 4];
    
    glGenVertexArrays(1, &d_vertex_array_id);
    glBindVertexArray(d_vertex_array_id);
}

ParticleManager::~ParticleManager()
{
    glDeleteBuffers(1, &d_particles_color_buffer);
    glDeleteBuffers(1, &d_particles_position_buffer);
    glDeleteBuffers(1, &d_billboard_vertex_buffer);
    glDeleteVertexArrays(1, &d_vertex_array_id);
    delete g_particle_color_data;
    delete g_particle_position_size_data;
}

void ParticleManager::initParticles()
{
    for(int i=0; i<(int)sqrt(d_MAXPARTICLES); i++) {
        for(int j=0; j<(int)sqrt(d_MAXPARTICLES); j++) {
            Particle particle;
            glm::vec2 d2Pos = glm::vec2(j*0.06, i*0.06) + glm::vec2(-20.0f,-20.0f);
            particle.pos = glm::vec3(d2Pos.x,d2Pos.y,-70);

            particle.mass= d_MASS;
            particle.life = 1000.0f;
            particle.cameradistance = -1.0f;

            particle.r = 255;
            particle.g = 0;
            particle.b = 0;
            particle.a = 255;

            particle.size = d_SIZE;
            d_particles_container.push_back(particle);
        }
    }
}

void ParticleManager::loadTexture(const std::string filename)
{

}

void ParticleManager::genGlBuffers()
{

    //billboard vertex_buffer
    glGenBuffers(1, &d_billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, d_billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    glGenBuffers(1, &d_particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, d_particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, d_MAXPARTICLES* 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    glGenBuffers(1, &d_particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, d_particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, d_MAXPARTICLES* 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

}

void ParticleManager::fillParticleGlBuffers(int index, int particle_count)
{
    Particle& p = d_particles_container[index];
    // Fill the GPU buffer
    g_particle_position_size_data[4*particle_count+0] = p.pos.x;
    g_particle_position_size_data[4*particle_count+1] = p.pos.y;
    g_particle_position_size_data[4*particle_count+2] = p.pos.z;
    g_particle_position_size_data[4*particle_count+3] = p.size;

    g_particle_color_data[4*particle_count+0] = p.r;
    g_particle_color_data[4*particle_count+1] = p.g;
    g_particle_color_data[4*particle_count+2] = p.b;
    g_particle_color_data[4*particle_count+3] = p.a;
}

void ParticleManager::updateGlBuffers()
{
    //update buffers openGL uses for rendering
    glBindBuffer(GL_ARRAY_BUFFER, d_particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, d_MAXPARTICLES* 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf
    glBufferSubData(GL_ARRAY_BUFFER, 0, d_MAXPARTICLES * sizeof(GLfloat) * 4, g_particle_position_size_data);

    glBindBuffer(GL_ARRAY_BUFFER, d_particles_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, d_MAXPARTICLES* 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf
    glBufferSubData(GL_ARRAY_BUFFER, 0, d_MAXPARTICLES * sizeof(GLubyte) * 4, g_particle_color_data);

}

void ParticleManager::drawParticles()
{
            // 1st attrib buffer: vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, d_billboard_vertex_buffer);
        glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // 2nd attrib buffer : positions of particle centers
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, d_particles_position_buffer);
        glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            4,                                // size : x + y + z + size => 4
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // 3rd attrib buffer : particles' colors
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, d_particles_color_buffer);
        glVertexAttribPointer(
            2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            4,                                // size : r + g + b + a => 4
            GL_UNSIGNED_BYTE,                 // type
            GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // The first parameter is the attribute buffer.
        // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
        glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
        glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
        glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

        //draw particles
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, d_MAXPARTICLES);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
}
