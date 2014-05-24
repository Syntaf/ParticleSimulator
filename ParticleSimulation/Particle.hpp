#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include <glm/glm.hpp>
#include <vector>

// CPU representation of a particle
struct Particle{
	glm::vec3 pos, speed;			//position, speed
	std::vector<glm::vec3> force;	//total force
	unsigned char r,g,b,a;			//color
	float mass,size;
	float life;						//remaining life of a particle, if <0 it's super dead
	float cameradistance;			//squared distance to camera : -1.0f if dead

	void addForce(glm::vec3 const& m_force)
	{
		force.push_back(m_force);
	}

	glm::vec3 getTotalForce() const
	{
		glm::vec3 total(0.0,0.0,0.0);
		for(auto i=0; i < force.size(); i++) {
			total += force[i];
		}
		return total;
	}

	//used for std::sort, needs an overloaded comparison operator
	bool operator<(const Particle& that) const {
		return this->cameradistance > that.cameradistance;
	}
};

#endif