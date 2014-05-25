#define _VARIADIC_MAX 9
#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <GL/glew.h>
#include <mutex>
#include <thread>
#include <future>
#include <SFML/graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "Common/shader.hpp"
#include "Common/controls.hpp"
#include "Common/texture.hpp"
#include "Particle.hpp"

#pragma comment(lib, "glew32.lib")

int FindUnusedParticle();
void SortParticles();
float clamp(float value, float min, float max);
float Distance(glm::vec3 const&, glm::vec3 const&);
double findAverage(std::vector<int> const& vec);
//oh my lord those parameters
void updateParticle(Particle& p,sf::Window const& window, glm::mat4& ViewMatrix, glm::mat4& ProjectionMatrix,
					double& delta, glm::vec3& CameraPosition, GLfloat* g_particule_position_size_data, GLubyte* g_particule_color_data,
					int& ParticlesCount);

static const int NUMTHREADS = 3000;
const float DRAG = 10;
const int MAXPARTICLES= 3000;					//3k particles					
Particle ParticlesContainer[MAXPARTICLES];		//declare array for particles
int LastUsedParticle=0;							//used to help with efficiency since i'm using a linear search
std::mutex forceVectorMutex;
std::mutex particleContainerMutex;
std::mutex tryLock;

int main(int argc, char* argv[]) {
	
	srand(time(NULL));							//seed the random generator
	
	
	sf::Window window;
	window.create(sf::VideoMode(800,600),	//declare window
		"Particle Simulation",					//window title
		sf::Style::Default,
		sf::ContextSettings(32, 8, 0, 3, 3)
		);										//default context settings, my custom ones were screwing with the program so I let SFML decide

	glViewport(0,0,window.getSize().x,window.getSize().y);
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
	static GLfloat* g_particule_position_size_data = new GLfloat[MAXPARTICLES* 4];
	static GLubyte* g_particule_color_data         = new GLubyte[MAXPARTICLES* 4];

	//initialize particle information
	for(auto i=0; i < MAXPARTICLES; i++){
		ParticlesContainer[i].mass = 50.0;
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
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
	glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES* 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	GLuint particles_color_buffer;
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES* 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	//generate all particles only ONCE, this way you don't have particles spawning while testing
	//not sure if this is a permanant implementation
	for(auto i=0; i<MAXPARTICLES; i++){
		int particleIndex = FindUnusedParticle();			//grab the index to give a particle life
		ParticlesContainer[particleIndex].life = 100.0f;	//This particle will live 50 seconds, more than enough to simulate a particle for the program

		//generate random positions for particles in the shape of a box with random patterns
		ParticlesContainer[particleIndex].pos = glm::vec3((rand()%50)/5.0,(rand()%50)/5.0,-50.0);
		
		// Very bad way to generate a random color
		ParticlesContainer[particleIndex].r = 255;
		ParticlesContainer[particleIndex].g = 0;
		ParticlesContainer[particleIndex].b = 0;
		ParticlesContainer[particleIndex].a = 255;

		ParticlesContainer[particleIndex].size = .2f;
			
	}

	bool running=true;										//set up bool to run SFML loop
	sf::Clock clock;										//clock for delta and controls
	sf::Clock mathRuntime;									//measures time taken to execute physcis equation
	std::vector<int> execution;
	std::thread t[NUMTHREADS];
	while( running )
	{
		double delta = clock.restart().asSeconds();

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
		computeMatricesFromInputs(window, delta);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		//Need the cameras position in order to sort the particles
		glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
		//get the VP
		glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		// NOT IN USE, pixels are currently not given a lifetime
		// Generate 5 new particule each millisecond,
		// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec)
		// int newparticles = (int)(delta*5000.0);
		// if (newparticles > (int)(0.016f*5000.0))
		//	 newparticles = (int)(0.016f*5000.0);

		// Simulate all particles
		int ParticlesCount = 0;
		for(int i=0; i<MAXPARTICLES; i++){

			Particle& p = ParticlesContainer[i]; // shortcut

			if(p.life > 0.0f){

				// Decrease life
				p.life -= delta;
				if (p.life > 0.0f){
					
					//mathRuntime.restart();
					updateParticle(p,window,ViewMatrix,ProjectionMatrix, delta,CameraPosition,g_particule_position_size_data,g_particule_color_data,ParticlesCount);
					//CURRENTLY NOT WORKING, commented out
					//t[i] = std::thread(updateParticle,p,ViewMatrix,ProjectionMatrix, delta,CameraPosition,g_particule_position_size_data,g_particule_color_data,ParticlesCount);
					//execution.push_back(mathRuntime.getElapsedTime().asMicroseconds());

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
		glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES* 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MAXPARTICLES* 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf
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
	//std::ofstream dataOut("recordings.txt", std::ios_base::app);
	//dataOut << findAverage(execution) << std::endl;

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

float clamp(float value, float min, float max)
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
	float distance = sqrt(pow((v2.x-v1.x),2) + pow((v2.y-v1.y),2));
	return distance;
}

double findAverage(std::vector<int> const& vec)
{
	long double average;
	for(auto i:vec){
		average += i;
	}
	average = average / vec.size();
	return average;
}

void updateParticle(Particle& p,sf::Window const& window, glm::mat4& ViewMatrix, glm::mat4& ProjectionMatrix,
					double& delta, glm::vec3& CameraPosition, GLfloat* g_particule_position_size_data, GLubyte* g_particule_color_data,
					int& ParticlesCount)
{
	bool pressed;
	
	// Simulate simple physics : gravity only, no collisions
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
	glm::vec4 vIn(	(2.0f*((float)(mousePos.x) / (window.getSize().x))) - 1.0f,		//2 * x / window.x - 1.0f
		1.0f - (2.0f * ((float)(mousePos.y)) / (window.getSize().y)),	//1 - 2 * y / window.y
		2.0 * winZ - 1.0f,												//equates to 1, we are only manipulating y,z
		1.0f															//dont question it
		);
	//find inverse
	glm::vec4 pos = vIn * inverse;

	pos.w = 1.0 / pos.w;
	pos.x *= pos.w;
	pos.y *= pos.w;
	pos.z *= pos.w;
					
	//if left mouse button is pressed
	if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
		pressed = true;
	}else
		pressed = false;


	p.addForce( (glm::vec3(glm::vec3(pos.x,pos.y,-50.0) - p.pos) * (float)(pressed*5000/pow(Distance(glm::vec3(pos.x,pos.y,pos.z),p.pos)+10,2))));
	p.addForce( -p.speed*DRAG);
				
	glm::vec3 prevPosition = p.pos;
	p.pos = p.pos + p.speed*(float)delta + 0.5f*p.getTotalForce()/p.mass*(float)pow(delta,2);
	p.speed = (p.pos - prevPosition)/(float)delta;

	float normSpeed = sqrt( pow(p.speed.x,2) + pow(p.speed.y,2));
	p.r = 255;
	p.g = clamp(255 - (normSpeed)*10,0,255);
	p.b = 0;
					
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

}