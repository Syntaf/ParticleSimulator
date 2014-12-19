#include <iostream>
#include <string>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "common/controls.hpp"
#include "common/texture.hpp"
#include "particle.hpp"
#include "particlemanager.hpp"
#ifdef USE_OPENCL
#include "opencl/cl_particle_updater.hpp"
#endif

const GLfloat ParticleManager::g_vertex_buffer_data[] = {
         -0.5f, -0.5f, 0.0f,
          0.5f, -0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
};

ParticleManager::ParticleManager(sf::Window* parent_window, int maxparticles, float drag,
                                 float mass, float size, float mouseforce):
d_MAXPARTICLES(maxparticles), d_DRAG(drag), d_MASS(mass),
d_SIZE(size), d_MOUSEFORCE(mouseforce), d_R(15), d_G(200),
d_B(75), d_A(255)
{
    d_parent_window = parent_window;

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
#ifdef USE_OPENCL
    delete cl_updater;
#endif
}

void ParticleManager::initParticles()
{
    int max_particles = (int)sqrt(d_MAXPARTICLES);
    for(int i=0; i<max_particles; i++) {
        for(int j=0; j<max_particles; j++) {
            //create single particle and place it in allignment with other particles to form
            //a square
            Particle particle;
            auto factor_x = (i - (float)max_particles / 2.0f) / (float)max_particles;
            auto factor_y = (j - (float)max_particles / 2.0f) / (float)max_particles;

            //multiply by arbitrary constant
            glm::vec2 particle_locale_delta = glm::vec2(30  * factor_x, 30 * factor_y);
            glm::vec2 d2Pos = glm::vec2(0, 0);
            d2Pos += particle_locale_delta;
            particle.pos = glm::vec3(d2Pos.x,d2Pos.y,-70);

            //set particle constants
            particle.life = 1000.0f;
            particle.cameradistance = -1.0f;

            particle.r = d_R;
            particle.g = d_G;
            particle.b = d_B;
            particle.a = d_A;

            particle.size = d_SIZE;
            d_particles_container.push_back(particle);
        }
    }

#ifdef USE_OPENCL
    // initialize opencl using opengl buffers
    cl_updater = new cl_particle_updater(d_MAXPARTICLES,
                                   d_particles_position_buffer, d_particles_color_buffer);

    // move gl buffers to cl
    cl_updater->lock_gl_buffers();

    // set the initial values of the particles
    cl_updater->set_particle_values(d_particles_container);

    // move buffers back to gl
    cl_updater->unlock_gl_buffers();
#endif
}

bool ParticleManager::loadTexture(const std::string& filename)
{
    if(!(d_texture = Syn::loadDDS("textures/Particle.DDS")))
        return false;
    return true;
}

void ParticleManager::genGlBuffers()
{
    glGenBuffers(1, &d_billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, d_billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &d_particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, d_particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, d_MAXPARTICLES* 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    glGenBuffers(1, &d_particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, d_particles_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, d_MAXPARTICLES* 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

}

void ParticleManager::fillParticleGlBuffers(const int& index,const int& particle_count)
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

void ParticleManager::activateTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, d_texture);
}

void ParticleManager::updateParticles(const float& delta, glm::mat4& ProjectionMatrix,
                                      glm::mat4& ViewMatrix)
{
    bool pressed = false;
    //Need the cameras position in order to sort the particles
    glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
    //get the VP

    //grab mouse coordinates so the particles can accelerate toward the given
    //  position. The following code converts SFML mouse coordinates into model
    //  space coordinates that OpenGL can use with the current particle coords
    //  . The equation is a brain twister, I barley understand what I did myself
    glm::vec4 mousePos(
        sf::Mouse::getPosition(*d_parent_window).x,
        sf::Mouse::getPosition(*d_parent_window).y,
        0.0f,
        1.0f
        );

    //manipulation from mouse cords to model-view mouse cords
    //find inverse of Proj * View
    glm::mat4 matProj = ViewMatrix * ProjectionMatrix;
    glm::mat4 inverse = glm::inverse(matProj);
    float winZ = 1.0;

    //determine space cords
    glm::vec4 vIn(    (2.0f*((float)(mousePos.x) / (d_parent_window->getSize().x))) - 1.0f,    //2 * x / window.x - 1.0f
        1.0f - (2.0f * ((float)(mousePos.y)) / (d_parent_window->getSize().y)),            //1 - 2 * y / window.y
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
    for(size_t i=0; i<d_particles_container.size(); i++){
        Particle& p = d_particles_container[i]; // shortcut

        //cycle through all particles currently alive
        if(p.life > 0.0f){

            // Decrease life, cycle through now if it's *still alive
            p.life -= delta;
            if (p.life > 0.0f){

                p.addForce(
                    (glm::vec3(glm::vec3(-mousePosmdl.x*1500,-mousePosmdl.y*1500, -70.0) - p.pos) * (float)(pressed*d_MOUSEFORCE/pow(Distance(glm::vec3(mousePosmdl.x,mousePosmdl.y, -70.0f),p.pos)+10,2))));
                p.addForce( -p.speed*d_DRAG);

                glm::vec3 prevPosition = p.pos;
                p.pos = p.pos + p.speed*(float)delta + 0.5f*p.getTotalForce()/d_MASS*(float)pow(delta,2);
                p.speed = (p.pos - prevPosition)/(float)delta;

                p.clearForce();

                float normSpeed = sqrt( pow(p.speed.x,2) + pow(p.speed.y,2));
                p.r = d_R;
                p.g = clamp(d_G - (normSpeed)*20,5,255);
                p.b = d_B;

                p.cameradistance = glm::length2( p.pos - CameraPosition );

                fillParticleGlBuffers(int(i), ParticlesCount);
            }else{
                // Particles that just died will be put at the end of the buffer in SortParticles();
                p.cameradistance = -1.0f;
            }
            ParticlesCount++;
        }
    }

    updateGlBuffers();

#else // USE_OPENCL

    ParticlesCount = (GLsizei)d_particles_container.size();

    // move gl buffers to cl
    cl_updater->lock_gl_buffers();

    cl_updater->update(mousePosmdl, pressed, delta, ParticlesCount);

    // move buffers back to gl
    cl_updater->unlock_gl_buffers();

#endif // USE_OPENCL
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

        // draw particles
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, d_MAXPARTICLES);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
}

unsigned char ParticleManager::clamp(float value, float min, float max)
{
    float result;
    if(value > max)
        result = max;
    else if(value < min)
        result = min;
    else
        result = value;
    return unsigned char(result);
}

float ParticleManager::Distance(glm::vec3 const& v1, glm::vec3 const& v2)
{
    return sqrt(pow((v2.x-v1.x),2) + pow((v2.y-v1.y),2));
}

void ParticleManager::setDrag(const float& val)
{
    d_DRAG = val;
}

void ParticleManager::setMass(const float& val)
{
    d_MASS = val;
}

void ParticleManager::setMouseForce(const float& val)
{
    d_MOUSEFORCE = val;
}

void ParticleManager::setColorR(const unsigned char& val)
{
    d_R = val;
}

void ParticleManager::setColorG(const unsigned char& val)
{
    d_G = val;
}

void ParticleManager::setColorB(const unsigned char& val)
{
    d_B = val;
}

void ParticleManager::setColorA(const unsigned char& val)
{
    d_A = val;
}

void ParticleManager::setColorRand(const int& val)
{
    if(val > 255 || val < 0) return;
    d_R = std::rand() % 255 - val;
    d_G = std::rand() % 255 - val;
    d_B = std::rand() % 255 - val;
}

void ParticleManager::resetParticles()
{
    d_particles_container.clear();
    int max_particles = (int)sqrt(d_MAXPARTICLES);
    for(int i=0; i<max_particles; i++) {
        for(int j=0; j<max_particles; j++) {
            //create single particle and place it in allignment with other particles to form
            //a square
            Particle particle;
            auto factor_x = (i - (float)max_particles / 2.0f) / (float)max_particles;
            auto factor_y = (j - (float)max_particles / 2.0f) / (float)max_particles;

            //multiply by arbitrary constant
            glm::vec2 particle_locale_delta = glm::vec2(30 * factor_x, 30 * factor_y);
            glm::vec2 d2Pos = glm::vec2(0, 0);
            d2Pos += particle_locale_delta;
            particle.pos = glm::vec3(d2Pos.x, d2Pos.y, -70);

            particle.life = 1000.0f;
            particle.cameradistance = -1.0f;

            particle.r = d_R;
            particle.g = d_G;
            particle.b = d_B;
            particle.a = d_A;

            particle.size = d_SIZE;
            d_particles_container.push_back(particle);
        }
    }
}

float ParticleManager::getDrag()
{
    return d_DRAG;
}

float ParticleManager::getMass()
{
    return d_MASS;
}

float ParticleManager::getMouseForce()
{
    return d_MOUSEFORCE;
}

int ParticleManager::getParticleCount()
{
    return d_MAXPARTICLES;
}

int ParticleManager::getColorR()
{
    return d_R;
}

int ParticleManager::getColorG()
{
    return d_G;
}

int ParticleManager::getColorB()
{
    return d_B;
}

int ParticleManager::getColorA()
{
    return d_A;
}

