#include <iostream>
#include <GL/glew.h>
#include <SFML/graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.hpp"
#include "controls.hpp"
#pragma comment(lib, "glew32.lib")

using namespace std;

int FindUnusedParticle();
void SortParticles();


// CPU represenatino of a particle
struct Particle{
	glm::vec3 pos, speed;
	unsigned char r,g,b,a;		//color
	float size, angle, weight;
	float life;					//remaining life of a particle, if <0 it's super dead
	float cameradistance;		//squared distance to camera : -1.0f if dead

	//used for std::sort, needs an overloaded comparison operator
	bool operator<(const Particle& that) const {
		return this->cameradistance > that.cameradistance;
	}
};

const int MaxParticles = 100000;
Particle ParticlesContainer[MaxParticles];		//one million is a humble start
int LastUsedParticle=0;							//used to help with efficiency since i'm using a linear search

int main(int argc, char* argv[]) {

	sf::ContextSettings settings;
	settings.depthBits			=24;
	settings.stencilBits		=8;
	settings.antialiasingLevel	=4;
	settings.majorVersion		=3;
	settings.minorVersion		=3;
	
	srand(time(NULL));
	
	sf::Window window(sf::VideoMode(800,800),
		"Particle Simulation",
		sf::Style::Default,
		settings
		);

	window.setVerticalSyncEnabled(true);

	GLenum err;
	glewExperimental = true;
	if (err = glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW\n";
		return -1;
	}

	//dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	//accept fragment if it is closer to the camera than the former one
	glDepthFunc(GL_LESS);
	//cull stuff not on screen
	glEnable(GL_CULL_FACE);

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	//create and compile the GLSL program
	GLuint programID = LoadShaders( "vertexShader.vert", "fragmentShader.frag");

	//vertex shader
	GLuint cameraRight_worldspace_ID	= glGetUniformLocation(programID, "cameraRight_worldspace");
	GLuint cameraUp_worldspace_ID		= glGetUniformLocation(programID, "cameraUp_worldspace");
	GLuint viewProjMatrixID				= glGetUniformLocation(programID, "VP");

	//data size, 4 because sizeof(float) = 4 bits
	static GLfloat * g_particle_position_size_data = new GLfloat[MaxParticles * 4];

	for(int i =0; i < MaxParticles; i++){
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	// the VBO containing 4 vertices will be shared by
	// all particles. Yay instancing
	static const GLfloat g_vertex_buffer_data[] = {
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};
	//billboard
	GLuint billboard_vertex_buffer;
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	//VBO containing positions and sizes of particles
	GLuint particles_position_buffer;
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	//Init with NULL buffer(empty) : updated later each frame
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	
	bool running=true;
	sf::Clock clock;
	float lastTime = clock.getElapsedTime().asSeconds();
	while( running )
	{
		double time= clock.getElapsedTime().asSeconds();
		double delta = time - lastTime;
		lastTime = time;
		clock.restart();
		sf::Event event;
		while(window.pollEvent(event))
		{
			if(event.type == sf::Event::Closed || event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				running = false;
			else if(event.type == sf::Event::Resized)
				glViewport(0,0,event.size.width,event.size.height);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		computeMatricesFromInputs(window, time);
		glm::mat4 projectionMatrix = getProjectionMatrix();
		glm::mat4 viewMatrix = getViewMatrix();

		//we need the cameras position in order to sort the particles
		//w.r.t the cameras distance
		glm::vec3 cameraPosition(glm::inverse(viewMatrix)[3]);

		glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

		//generate 10 new particles each milisecond
		//make sure to limit to 60fps(16ms) new particles will be
		//huge and the next frame, longer
		int newparticles = (int)(delta*10000.0);
		if(newparticles > (int)(0.016f*10000.0))
			newparticles = (int)(0.016f*10000.0);

		for(int i =0; i < newparticles; i++) {
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].life = 5.0f;
			ParticlesContainer[particleIndex].pos = glm::vec3(0,0,-20.0f);

			float spread = 1.5f;
			glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
			//kinda bad way to generate a random diraction
			glm::vec3 randomdir = glm::vec3(
				(rand()%2000 - 1000.0f)/1000.0f,
				(rand()%2000 - 1000.0f)/1000.0f,
				(rand()%2000 - 1000.0f)/1000.0f
			);

			ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;

			//another bad way to generate random color
			ParticlesContainer[particleIndex].r = rand() % 256;
			ParticlesContainer[particleIndex].g = rand() % 256;
			ParticlesContainer[particleIndex].b = rand() % 256;
			ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

			ParticlesContainer[particleIndex].size = (rand()%1000) / 2000.0f + 0.1f;
		}

		//simulate all particles
		int particlesCount =0;
		for(int i =0; i<particlesCount; i++){

			Particle& p = ParticlesContainer[i];

			if(p.life > 0.0f){

				//dec life
				p.life -= delta;
				if(p.life > 0.0f){

					//simulate simple physics: gravity
					p.speed += glm::vec3(0.0, -9.81f, 0.0f) * (float)delta * 0.5f;
					p.pos += p.speed * (float)delta;
					p.cameradistance = glm::length( p.pos - cameraPosition);

					//fill GPU buffer
					g_particle_position_size_data[4*particlesCount+0] = p.pos.x;
					g_particle_position_size_data[4*particlesCount+1] = p.pos.y;
					g_particle_position_size_data[4*particlesCount+2] = p.pos.z;

					g_particle_position_size_data[4*particlesCount+3] = p.size;

				}else{
					//particles that just died will be put at the end of the buffer in sort
					p.cameradistance = -1.0f;
				}

				particlesCount++;
			}
		}

		SortParticles();

		//now update the buffers that openGL uses for rendering

		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); //buffer orphaning
		glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLfloat) * 4, g_particle_position_size_data);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//use our shader
		glUseProgram(programID);

		glUniform3f(cameraRight_worldspace_ID, viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]);
		glUniform3f(cameraUp_worldspace_ID, viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]);

		glUniformMatrix4fv(viewProjMatrixID, 1, GL_FALSE, &viewProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
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
		
		// 2nd attribute buffer : positions of particles' centers
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

		// These functions are specific to glDrawArrays*Instanced*.
		// The first parameter is the attribute buffer we're talking about.
		// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

		// Draw the particles !
		// This draws a lot of triangle stips
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particlesCount);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		window.display();

	}
	
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &billboard_vertex_buffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &vertexArrayID);

	return 0;

}

int FindUnusedParticle(){

	for(int i=LastUsedParticle; i<MaxParticles; i++){
		if(ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	for(int i=0; i<LastUsedParticle; i++){
		if (ParticlesContainer[i].life<0){
			LastUsedParticle = i;
			return i;
		}
	}

	return 0;		//All particles taken, override first one
}

void SortParticles(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}