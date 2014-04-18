#include <iostream>
#include <GL/glew.h>
#include <cstdlib>
#include <SFML/graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "Common/shader.hpp"
#include "Common/controls.hpp"
#include "Common/texture.hpp"
#include <sstream>
#pragma comment(lib, "glew32.lib")

int FindUnusedParticle();
void SortParticles();

// CPU representation of a particle
struct Particle{
	glm::vec3 pos, speed;		//used for physics manipulation
	unsigned char r,g,b,a;		//color
	float size, angle, weight;
	float life;					//remaining life of a particle, if <0 it's super dead
	float cameradistance;		//squared distance to camera : -1.0f if dead

	//used for std::sort, needs an overloaded comparison operator
	bool operator<(const Particle& that) const {
		return this->cameradistance > that.cameradistance;
	}
};

const int MaxParticles = 100000;				//100k max particles to start humble						
Particle ParticlesContainer[MaxParticles];		//declare array for particles
int LastUsedParticle=0;							//used to help with efficiency since i'm using a linear search

int main(int argc, char* argv[]) {
	
	srand(time(NULL));							//seed the random generator
	
	sf::Window window(sf::VideoMode(800,600),	//declare window
		"Particle Simulation"					//window title
		);										//default context settings, my custom ones were screwing with the program so I let SFML decide

	window.setMouseCursorVisible(true);			//Make sure cursor is visible
	window.setVerticalSyncEnabled(true);		//smooth

	// Initialize GLEW
	glewExperimental = true;					// Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);		// Dark blue background

	
	glEnable(GL_DEPTH_TEST);					// Enable depth test
	
	glDepthFunc(GL_LESS);						// Accept fragment if it is closer to the camera than the former one

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "Shaders/vertexShader.vert", "Shaders/fragmentShader.frag" );

	// Vertex shader
	GLuint CameraRight_worldspace_ID  = glGetUniformLocation(programID, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID  = glGetUniformLocation(programID, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

	// fragment shader
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	//data!
	static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	static GLubyte* g_particule_color_data         = new GLubyte[MaxParticles * 4];

	//initialize particle information
	for(int i=0; i<MaxParticles; i++){
		ParticlesContainer[i].life = -1.0f;							//starts dead
		ParticlesContainer[i].cameradistance = -1.0f;				//not on screen
	}

	//load texture
	GLuint Texture = loadDDS("Textures/Particle.DDS");	

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
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	GLuint particles_color_buffer;
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	//Important to set the mouse to the middle of the screen before loop creation and calculating direction
	sf::Mouse::setPosition(sf::Vector2i(400,300), window);	
	bool running=true;										//set up bool to run SFML loop
	sf::Clock clock;										//clock for delta and controls
	float lastTime = clock.getElapsedTime().asSeconds();	//UNDER REVISION NOT IN USE
	while( running )
	{
		double time= clock.getElapsedTime().asSeconds();
		double delta = .03;									//PLACE HOLDER, FAKE DELTA
		lastTime = time;
		clock.restart();
		sf::Event event;
		while(window.pollEvent(event))						//handle any closing events
		{
			if(event.type == sf::Event::Closed || event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				running = false;
			else if(event.type == sf::Event::Resized)
				glViewport(0,0,event.size.width,event.size.height);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//clear the screen in anticipation for drawing

		//handle any input and grab matrices
		computeMatricesFromInputs(window, time);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		//Need the cameras position in order to sort the particles
		glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
		//get the VP
		glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;


		// Generate 30 new particule each millisecond,
		// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec)
		int newparticles = (int)(delta*30000.0)
			
			;
		if (newparticles > (int)(0.016f*30000.0))
			newparticles = (int)(0.016f*30000.0);
		
		for(int i=0; i<newparticles; i++){
			int particleIndex = FindUnusedParticle();		//grab the index to give a particle life
			ParticlesContainer[particleIndex].life = 5.0f;	//This particle will live 5 seconds.
			ParticlesContainer[particleIndex].pos = glm::vec3(0,0,-20.0f);

			float spread = 1.5f;
			glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
			
			//bad way to randomize direction
			glm::vec3 randomdir = glm::vec3(
				(rand()%2000 - 1000.0f)/1000.0f,
				(rand()%2000 - 1000.0f)/1000.0f,
				(rand()%2000 - 1000.0f)/1000.0f
			);
			
			ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;


			// Very bad way to generate a random color
			ParticlesContainer[particleIndex].r = rand() % 256;
			ParticlesContainer[particleIndex].g = rand() % 256;
			ParticlesContainer[particleIndex].b = rand() % 256;
			ParticlesContainer[particleIndex].a = 255;//(rand() % 256) / 3;

			ParticlesContainer[particleIndex].size = .07f;//(rand()%1000)/2000.0f + 0.1f;
			
		}



		// Simulate all particles
		int ParticlesCount = 0;
		for(int i=0; i<MaxParticles; i++){

			Particle& p = ParticlesContainer[i]; // shortcut

			if(p.life > 0.0f){

				// Decrease life
				p.life -= delta;
				if (p.life > 0.0f){

					// Simulate simple physics : gravity only, no collisions
					glm::vec2 mousePos(
						sf::Mouse::getPosition(window).x, 
						sf::Mouse::getPosition(window).y
					); 
					mousePos = mousePos / glm::vec2(window.getSize().x/2, window.getSize().y/2) - glm::vec2(CameraPosition.x, CameraPosition.y);
					//std::cout << "mousePos(x,y): (" << mousePos.x << "," << mousePos.y << ")\r";
					p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;
					p.pos += p.speed * (float)delta;
					p.cameradistance = glm::length2( p.pos - CameraPosition );
					//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

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

		SortParticles();

		//update buffers openGL uses for rendering
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

		//std::cout << "Particle Count: " << ParticlesCount << "\r";
		
		//sfml display to window
		window.display();

		

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

//sort particles according to dist.
void SortParticles(){
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}