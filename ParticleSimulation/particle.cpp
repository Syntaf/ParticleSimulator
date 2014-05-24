#include "particle.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

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

Particle::Particle()
{
	d_size = 0.2f;							//small size
	d_mass = 50.0f;							//not tooooo heavy
	d_life = -1.0f;							//dead
	d_cameradistance = -1.0f;				//start dead
	d_r = d_g = d_b = d_a = 125;			//rgba=125
	d_speed = glm::vec3(0.0f,0.0f,0.0f);	//no speed
	d_pos = glm::vec3(50.0f,50.0f,-50.0f);	//near middle of screen
}

Particle::Particle(float mass, float size, float life, float cameradistance,
				   unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	d_mass = mass;
	d_size = size;
	d_life = life;
	d_cameradistance = cameradistance;
	d_r = r; d_g = g; d_b = b; d_a = a;

}

void Particle::addForce(glm::vec3 const& force)
{
	d_force.push_back(force);
}

void Particle::setSpeed(glm::vec3 const& speed)
{
	d_speed = speed;
}

void Particle::setPosition(glm::vec3 const& pos)
{
	d_pos = pos;
}

void Particle::setMass(float mass)
{
	d_mass = mass;
}

void Particle::setSize(float size)
{
	d_size = size;
}

void Particle::setLife(float life)
{
	d_life = life;
}

void Particle::setCameradistance(float cameradistance)
{
	d_cameradistance = cameradistance;
}

void Particle::setRGBA(unsigned char r, 
			 unsigned char g, 
			 unsigned char b,
			 unsigned char a)
{
	d_r = r;
	d_g = g;
	d_b = b;
	d_a = a;
}

glm::vec3 Particle::getTotalForce() const
{
	glm::vec3 total(0.0f,0.0f,0.0f);
	for(auto i=0; i < d_force.size(); i++) {
		total += d_force[i];
	}
	return total;
}

std::vector<glm::vec3> Particle::getForce() const
{
	return d_force;
}

glm::vec3 Particle::getPosition() const
{
	return d_pos;
}

glm::vec3 Particle::getSpeed() const
{
	return d_speed;
}

void Particle::getRGBA(char* RGBA)
{
	RGBA[0] = d_r;
	RGBA[1] = d_g;
	RGBA[2] = d_b;
	RGBA[3] = d_a;
}

float Particle::getMass() const
{
	return d_mass;
}

float Particle::getLife() const
{
	return d_life;
}

float Particle::getCameradistance() const
{
	return d_cameradistance;
}

float Particle::getSize() const
{
	return d_size;
}

void Particle::updateParticle(float const& dt, glm::vec3 const& cameraPosition)
{
	glm::vec3 prevPosition = d_pos;
	d_pos = d_pos + (d_pos*(float)dt) + (0.5f*getTotalForce())/(d_mass*(float)pow(dt,2));
	d_speed = (d_pos - prevPosition)/(float)dt;

	float normSpeed = sqrt( pow(d_speed.x,2) + pow(d_speed.y,2));
	d_r = 255;
	d_g = clamp(255 - (normSpeed)*10,0,255);
	d_b = 0;
					
	d_cameradistance = glm::length2( d_pos - cameraPosition );
}
