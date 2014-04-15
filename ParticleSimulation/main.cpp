#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>
#include "Particle.h"
#include "Utility.h"
#include <math.h>

const int GRAVITY = 2000;
const int DRAG = 10;

const int RESTITUTION_COEFF = 1;
const float INITIAL_DISTANCE = 0.5;

const int MOURSE_FORCE = -2000000;

const int CHUNK_NB = 10;
const int VERTEX_CHUNK = 100000;

using namespace std;
using namespace sf;

int main(int argc, char* argv[])
{
	srand(time(NULL));

	int width=1000;
	int height=1000;
	Window window(sf::VideoMode(width, height, 32), "Particle Simulation");

	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,width,height,0,-1,1);

	vector<Particle> particles;
	int particleSize = CHUNK_NB*VERTEX_CHUNK;										//avoid repeating particles.size() during for loop

	for(auto i(0); i<1000; i++) {													//store particle instances in particles array
		for(auto j(0); j<1000; j++) {
			Particle particle;														//set values and push back
			particle.setPosition( Vector2f(j*INITIAL_DISTANCE, i*INITIAL_DISTANCE) + Vector2f(20,20) );
			particle.setMass(10);
			particles.push_back(particle);
		}
	}

	Clock deltaTime;
	float dt=0.00000001;					//set so low to avoid undefined behaviors in first frame
	Vector2f previousPosition;				//sfml vector to check whether a particle passed a wall during frame

	vector<Vector2f> wallPoints;			//array of points that represent 4 walls that will block particles if specified.
	wallPoints.push_back(Vector2f(10,10));
	wallPoints.push_back(Vector2f(11,height-10));

	wallPoints.push_back(Vector2f(11,height-10));
	wallPoints.push_back(Vector2f(width-10,height-10));

	wallPoints.push_back(Vector2f(width-10,height-10));
	wallPoints.push_back(Vector2f(width-11,10));

	wallPoints.push_back(Vector2f(width-11,10));
	wallPoints.push_back(Vector2f(10,10));

	float vertexCoords[CHUNK_NB][2*VERTEX_CHUNK];

	unsigned char colors[CHUNK_NB][3*VERTEX_CHUNK];

	glEnable( GL_POINT_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glPointSize( 1 );

	bool LMB = false;
	float zoom = 1;
	Vector2f camPos(0,0);

	while(window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if(event.type == sf::Event::Closed)
				window.close();
		}

		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT);

		//Controls //////////////

		if(Mouse::isButtonPressed(Mouse::Left))
			LMB = true;
		else 
			LMB = false;

		if(Keyboard::isKeyPressed(Keyboard::Z))
			zoom += 1*dt*zoom;
		else if(Keyboard::isKeyPressed(Keyboard::S))
			zoom -= 1*dt*zoom;
		if(Keyboard::isKeyPressed(Keyboard::Left))
			camPos.x+=500*dt/zoom;
		else if(Keyboard::isKeyPressed(Keyboard::Right))
			camPos.x-=500*dt/zoom;
		if(Keyboard::isKeyPressed(Keyboard::Up))
			camPos.y+=500*dt/zoom;
		else if(Keyboard::isKeyPressed(Keyboard::Down))
			camPos.y-=500*dt/zoom;

		//current mouse position
		Vector2f mousePos = (Vector2f(Mouse::getPosition(window).x, Mouse::getPosition(window).y)/zoom-Vector2f(width/2,height/2)/zoom - camPos);

		for(int i=0; i< particleSize; i++){ //update newtons law equation each frame

			//if user clicks, add force proportional to the inverse of the distance squared
			particles[i].addForce( (Vector2f(mousePos - particles[i].getPosition()) * (float)(LMB*500000/pow(Distance(mousePos, particles[i].getPosition()) +10,2))));
			//add drag proportional to the speed
			particles[i].addForce( -particles[i].getSpeed()*(float)DRAG); 

			//update position of particles
			particles[i].updatePosition(dt);

			//clear to prevent adding force over time
			particles[i].clearForce();
	
		}

		for(int j=0; j < CHUNK_NB; j++) {
			for(int i=0; i < VERTEX_CHUNK; i++){
				colors[j][3*i] = 255;
				colors[j][3*i+1] = clamp(255-Norm(particles[i+j*VERTEX_CHUNK].getSpeed()),0,255);
				colors[j][3*i+2]= 0;

				vertexCoords[j][2*i] = particles[i + j*VERTEX_CHUNK].getPosition().x;
				vertexCoords[j][2*i+1] = particles[i + j*VERTEX_CHUNK].getPosition().y;
			}
		}

		glPushMatrix();

		glTranslatef(width/2.f,height/2.f,0);
		glScaled(zoom, zoom, zoom);
		glTranslated(camPos.x, camPos.y,0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		for(int j=0; j<CHUNK_NB; j++){
			glVertexPointer(2,GL_FLOAT,0,vertexCoords[j]);
			glColorPointer(3,GL_UNSIGNED_BYTE,0,colors[j]);
			glDrawArrays(GL_POINTS,0,VERTEX_CHUNK);
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		
		glPopMatrix();

		glFlush();
		window.display();

		dt=deltaTime.restart().asSeconds();
	}
		
	return 0;
}