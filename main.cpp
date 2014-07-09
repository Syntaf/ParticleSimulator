#include <iostream>
#include <cstdlib>
#include <vector>
#include <GL/glew.h>
#include <SFML/graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "common/shader.hpp"
#include "common/controls.hpp"
#include "common/texture.hpp"
#include "console.hpp"
#include "Particle.hpp"
#include "config.h"

#ifdef USE_OPENCL
#include "opencl/cl_particle_updater.hpp"
#endif

//#pragma comment(lib, "glew32.lib")

int FindUnusedParticle();
void SortParticles();
unsigned char clamp(float value, float min, float max);
float Distance(glm::vec3 const&, glm::vec3 const&);

const float DRAG = 20;							//drag force
const int MAXPARTICLES= 1000000;					//2.5k particles				
std::vector<Particle> ParticlesContainer;		//holds all particles
int LastUsedParticle=0;							//used to help with efficiency since i'm using a linear search

int main(int argc, char* argv[]) {

//debug is slow as fuck, make sure people are running on release 
#ifdef _DEBUG
    std::cout << "\nWARNING: Debug is currently active, optimizations are not enabled this is configuration" <<
        ". Unless you are actually debugging, switch to Release mode\n" << std::endl;
#endif
    
    std::cout << "Particle Simulator Version: " << VERSION_MAJOR << "." << VERSION_MINOR << "\n";

    srand(time(NULL));							//seed the random generator

    sf::Window window;
    window.create(sf::VideoMode(1000,750),		//declare window
        "Particle Simulation",					//window title
        sf::Style::Default,
        sf::ContextSettings(32, 8, 0, 3, 3)
        );										//default context settings, my custom ones were screwing with the program so I let SFML decide
    
    ConsoleManager console_window(&window);
    
    glViewport(0,0,window.getSize().x,window.getSize().y);
    window.setMouseCursorVisible(true);			//Make sure cursor is visible
    window.setVerticalSyncEnabled(true);		//smooth
    sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(window).x + window.getSize().x/2, sf::Mouse::getPosition(window).y));
    

    // Initialize GLEW
    glewExperimental = true;					// Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.15f, 0.0f);		// Dark blue background

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "shaders/vertexShader.vert", "shaders/fragmentShader.frag" );

    // Vertex shader
    GLuint CameraRight_worldspace_ID  = glGetUniformLocation(programID, "CameraRight_worldspace");
    GLuint CameraUp_worldspace_ID  = glGetUniformLocation(programID, "CameraUp_worldspace");
    GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

    // fragment shader
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    //buffer data
    static GLfloat* g_particule_position_size_data = new GLfloat[MAXPARTICLES* 4];
    static GLubyte* g_particule_color_data         = new GLubyte[MAXPARTICLES* 4];
        
    //load texture
    GLuint Texture = loadDDS("textures/Particle.DDS");	

    // The VBO containing the 4 vertices of the particles.
    // Thanks to instancing, they will be shared by all particles.
    static const GLfloat g_vertex_buffer_data[] = { 
         -0.5f, -0.5f, 0.0f,
          0.5f, -0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
    };
    //buffer data
    GLuint billboard_vertex_buffer;
    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    GLuint particles_position_buffer;
    glGenBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES* 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    GLuint particles_color_buffer;
    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES* 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);


    //init particles and shape them like a rectangle, i*j should always = MAXPARTICLES or we have a problem
    for(int i(0); i < (int)sqrt(MAXPARTICLES); i++) {													
        for(int j(0); j < (int)sqrt(MAXPARTICLES); j++) {
            Particle particle;		
            glm::vec2 d2Pos = glm::vec2(j*0.05, i*0.05) + glm::vec2(-15.0f,-15.0f);
            particle.pos = glm::vec3(d2Pos.x,d2Pos.y,-70);
            
            particle.mass=50.0;
            particle.life = 100.0f;
            particle.cameradistance = -1.0f;
            
            particle.r = 255;
            particle.g = 0;
            particle.b = 0;
            particle.a = 255;

            particle.size = .1f;
            ParticlesContainer.push_back(particle);
        }
    }

#ifdef USE_OPENCL

    // initialize opencl using opengl buffers
    cl_particle_updater cl_updater(wglGetCurrentContext(), wglGetCurrentDC(), MAXPARTICLES,
                                   particles_position_buffer, particles_color_buffer);

    // move gl buffers to cl
    cl_updater.lock_gl_buffers();

    // set the initial values of the particles
    cl_updater.set_particle_values(ParticlesContainer);

    // move buffers back to gl
    cl_updater.unlock_gl_buffers();

#endif


    bool running=true;										//set up bool to run SFML loop
    bool pressed=false;
    sf::Clock clock;										//clock for delta and controls
    sf::Clock fps_clock;
    float fps_last_time = 0;
    while( running )
    {
        float delta = clock.restart().asSeconds();
        sf::Event event;
        while(window.pollEvent(event))						//handle any closing events
        {
            if(event.type == sf::Event::Closed || event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                running = false;
            else if(event.type == sf::Event::Resized)
                glViewport(0,0,event.size.width,event.size.height);
        }
        console_window.handleEvent(event, running);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//clear the screen in anticipation for drawing
        
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
        glm::vec4 vIn(	(2.0f*((float)(mousePos.x) / (window.getSize().x))) - 1.0f,	//2 * x / window.x - 1.0f
            1.0f - (2.0f * ((float)(mousePos.y)) / (window.getSize().y)),			//1 - 2 * y / window.y
            2.0 * winZ - 1.0f,														//equates to 1, we are only manipulating y,z
            1.0f																	//dont question it
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
        for(size_t i=0; i<ParticlesContainer.size(); i++){
            Particle& p = ParticlesContainer[i]; // shortcut

            //cycle through all particles currently alive
            if(p.life > 0.0f){

                // Decrease life, cycle through now if it's *still alive
                p.life -= delta;
                if (p.life > 0.0f){
                    
                    p.addForce( 
                        (glm::vec3(glm::vec3(-mousePosmdl.x*500,-mousePosmdl.y*500, -70.0) - p.pos) * (float)(pressed*50000/pow(Distance(glm::vec3(mousePosmdl.x,mousePosmdl.y, -70.0f),p.pos)+10,2))));
                    p.addForce( -p.speed*DRAG);
                
                    glm::vec3 prevPosition = p.pos;
                    p.pos = p.pos + p.speed*(float)delta + 0.5f*p.getTotalForce()/p.mass*(float)pow(delta,2);
                    p.speed = (p.pos - prevPosition)/(float)delta;

                    p.clearForce();

                    float normSpeed = sqrt( pow(p.speed.x,2) + pow(p.speed.y,2));
                    p.r = 120;
                    p.g = clamp(200 - (normSpeed)*20,0,255);
                    p.b = 5;
                    
                    p.cameradistance = glm::length2( p.pos - CameraPosition );

                    // Fill the GPU buffer
                    g_particule_position_size_data[4*ParticlesCount+0] = p.pos.x;
                    g_particule_position_size_data[4*ParticlesCount+1] = p.pos.y;
                    g_particule_position_size_data[4*ParticlesCount+2] = p.pos.z;
                    g_particule_position_size_data[4*ParticlesCount+3] = p.size;	

                    g_particule_color_data[4*ParticlesCount+0] = p.r;				
                    g_particule_color_data[4*ParticlesCount+1] = p.g;
                    g_particule_color_data[4*ParticlesCount+2] = p.b;					
                    g_particule_color_data[4*ParticlesCount+3] = p.a;
                }else{
                    // Particles that just died will be put at the end of the buffer in SortParticles();
                    p.cameradistance = -1.0f;
                }

                ParticlesCount++;

            }
        }



        //SortParticles();

        //update buffers openGL uses for rendering
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES* 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf
        glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES* 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf
        glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);

#else // USE_OPENCL
        
        ParticlesCount = (GLsizei)ParticlesContainer.size();

        // move gl buffers to cl
        cl_updater.lock_gl_buffers();

        cl_updater.update(mousePosmdl, pressed, delta, ParticlesCount);

        // move buffers back to gl
        cl_updater.unlock_gl_buffers();

#endif // USE_OPENCL


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

        // 1st attrib buffer: vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
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
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
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
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
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
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        console_window.render();
        window.display();

        float fps_current_time = fps_clock.restart().asSeconds();
        float fps = 1.f / (fps_current_time - (fps_current_time - fps_last_time));
        fps_last_time = fps_current_time;
        std::cout << "\r" << fps;   
    }

    //free up memory openGL used
    glDeleteBuffers(1, &particles_color_buffer);
    glDeleteBuffers(1, &particles_position_buffer);
    glDeleteBuffers(1, &billboard_vertex_buffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    return 0;

}

//Function to find all unused particles in the array, uses linear search and leaves off
//on the last particle found. If out of particles then it overwrites first element
int FindUnusedParticle(){

    for(auto i=LastUsedParticle; i<MAXPARTICLES; i++){
        if(ParticlesContainer[i].life < 0){
            LastUsedParticle = i;
            return i;
        }
    }

    for(auto i=0; i<LastUsedParticle; i++){
        if (ParticlesContainer[i].life<0){
            LastUsedParticle = i;
            return i;
        }
    }

    return 0;		//All particles taken, override first one
}

//sort particles according to dist.
void SortParticles(){
    std::sort(&ParticlesContainer[0], &ParticlesContainer[MAXPARTICLES]);
}

unsigned char clamp(float value, float min, float max)
{
    float result;
    if(value > max)
        result = max;
    else if(value < min)
        result = min;
    else 
        result = value;
    return result;
}

float Distance(glm::vec3 const& v1, glm::vec3 const& v2)
{
    return sqrt(pow((v2.x-v1.x),2) + pow((v2.y-v1.y),2));
}
